// SPDX-License-Identifier: MIT
#include "game.h"

#include "audio.h"
#include "clock.h"
#include "input.h"
#include "log.h"

#include "pch.h"
#include "Sound.h"
#include "TPinballTable.h"
#include "fullscrn.h"
#include "midi.h"
#include "nudge.h"
#include "options.h"
#include "pb.h"
#include "render.h"
#include "winmain.h"

#include "SDL_mixer.h"

#include <cctype>
#include <string>

namespace scp {
namespace {

bool g_ready = false;

std::string dirname_of(const char* path) {
  if (!path) return {};
  std::string p(path);
  auto pos = p.find_last_of("/\\");
  if (pos == std::string::npos) return {};
  return p.substr(0, pos + 1);
}

std::string basename_of(const char* path) {
  if (!path) return {};
  std::string p(path);
  auto pos = p.find_last_of("/\\");
  if (pos == std::string::npos) return p;
  return p.substr(pos + 1);
}

}  // namespace

bool game_init(const GameConfig& cfg) {
  game_shutdown();
  input_reset();
  clock_reset(0);
  rng_seed(1u);

  if (!cfg.dat_path || !cfg.dat_path[0]) {
    log(RETRO_LOG_ERROR, "game_init: missing DAT path\n");
    return false;
  }

  ImGui::CreateContext();
  winmain::ImIO = &ImGui::GetIO();
  winmain::ImIO->IniFilename = nullptr;

  // Fake window/renderer so texture creation during render::init succeeds.
  winmain::MainWindow = SDL_CreateWindow("scp", 0, 0, 640, 480, 0);
  winmain::Renderer = SDL_CreateRenderer(winmain::MainWindow, -1, 0);

  options::Options.ShowMenu = false;
  options::Options.Sounds = true;
  options::Options.Music = false;
  // Force original 640x480 table geometry for MinUI (resolution index 0).
  options::Options.Resolution = 0;
  options::InitPrimary();

  pb::BasePath = dirname_of(cfg.dat_path);
  pb::DatFileName = basename_of(cfg.dat_path);
  pb::FullTiltMode = false;
  pb::FullTiltDemoMode = false;
  {
    auto lower = pb::DatFileName;
    for (auto& c : lower) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (lower == "cadet.dat") pb::FullTiltMode = true;
    if (lower == "demo.dat") pb::FullTiltDemoMode = pb::FullTiltMode = true;
  }

  options::InitSecondary();
  fullscrn::SetResolution(0);

  const bool mix_ok = Mix_OpenAudio(scp::kAudioSampleRate, MIX_DEFAULT_FORMAT, 2, 1024) == 0;
  Sound::Init(mix_ok, options::Options.SoundChannels, options::Options.Sounds,
              options::Options.SoundVolume);
  midi::music_init(false, options::Options.MusicVolume);

  if (pb::init() != 0) {
    log(RETRO_LOG_ERROR, "pb::init failed for %s\n", cfg.dat_path);
    game_shutdown();
    return false;
  }

  fullscrn::init();
  pb::reset_table();
  pb::firsttime_setup();
  pb::replay_level(false);

  g_ready = true;
  log(RETRO_LOG_INFO, "game_init OK (%s)\n", cfg.dat_path);
  return true;
}

void game_step(float dt_ms) {
  if (!g_ready) return;
  if (winmain::single_step) {
    clock_advance_ms(dt_ms);
    return;
  }
  pb::frame(dt_ms);
  clock_advance_ms(dt_ms);
}

void game_shutdown() {
  if (g_ready) {
    options::uninit();
    midi::music_shutdown();
    Sound::Close();
    Mix_CloseAudio();
    pb::uninit();
    fullscrn::shutdown();
  }
  g_ready = false;
  input_reset();

  if (winmain::Renderer) {
    SDL_DestroyRenderer(winmain::Renderer);
    winmain::Renderer = nullptr;
  }
  if (winmain::MainWindow) {
    SDL_DestroyWindow(winmain::MainWindow);
    winmain::MainWindow = nullptr;
  }
  winmain::ImIO = nullptr;
  ImGui::DestroyContext();
}

bool game_ready() { return g_ready; }

bool game_input_blocked() {
  return !g_ready || winmain::single_step;
}

void game_handle_input(const InputEvent* events, int count) {
  if (!g_ready || !events || count <= 0) return;
  if (!pb::MainTable) return;

  const bool in_game = pb::game_mode == GameModes::InGame;

  for (int i = 0; i < count; ++i) {
    switch (events[i].kind) {
      case InputEventKind::LeftFlipperDown:
        if (in_game)
          pb::MainTable->Message(MessageCode::LeftFlipperInputPressed, pb::time_now);
        break;
      case InputEventKind::LeftFlipperUp:
        pb::MainTable->Message(MessageCode::LeftFlipperInputReleased, pb::time_now);
        break;
      case InputEventKind::RightFlipperDown:
        if (in_game)
          pb::MainTable->Message(MessageCode::RightFlipperInputPressed, pb::time_now);
        break;
      case InputEventKind::RightFlipperUp:
        pb::MainTable->Message(MessageCode::RightFlipperInputReleased, pb::time_now);
        break;
      case InputEventKind::PlungerDown:
        if (in_game)
          pb::MainTable->Message(MessageCode::PlungerInputPressed, pb::time_now);
        break;
      case InputEventKind::PlungerUp:
        pb::MainTable->Message(MessageCode::PlungerInputReleased, pb::time_now);
        break;
      case InputEventKind::NudgeLeftDown:
        if (in_game && !pb::MainTable->TiltLockFlag) nudge::nudge_right();
        break;
      case InputEventKind::NudgeLeftUp:
        nudge::un_nudge_right(0, nullptr);
        break;
      case InputEventKind::NudgeRightDown:
        if (in_game && !pb::MainTable->TiltLockFlag) nudge::nudge_left();
        break;
      case InputEventKind::NudgeRightUp:
        nudge::un_nudge_left(0, nullptr);
        break;
      case InputEventKind::NudgeBottomDown:
        if (in_game && !pb::MainTable->TiltLockFlag) nudge::nudge_up();
        break;
      case InputEventKind::NudgeBottomUp:
        nudge::un_nudge_up(0, nullptr);
        break;
      case InputEventKind::Pause:
        winmain::pause();
        break;
      case InputEventKind::NewGame:
        winmain::new_game();
        break;
      case InputEventKind::HighScores:
        winmain::pause(false);  // force pause while overlay is conceptually open
        pb::high_scores();
        break;
      case InputEventKind::ToggleSound:
        options::toggle(Menu1::Sounds);
        break;
      case InputEventKind::ToggleMusic:
        options::toggle(Menu1::Music);
        break;
    }
  }
}

FramebufferView game_framebuffer() {
  FramebufferView view;
  if (!g_ready || !render::vscreen) return view;
  view.width = render::vscreen->Width;
  view.height = render::vscreen->Height;
  view.stride = render::vscreen->Stride;
  view.rgba = render::vscreen->BmpBufPtr1;
  view.indexed = reinterpret_cast<const unsigned char*>(render::vscreen->IndexedBmpPtr);
  view.valid = view.rgba != nullptr && view.width > 0 && view.height > 0;
  return view;
}

}  // namespace scp
