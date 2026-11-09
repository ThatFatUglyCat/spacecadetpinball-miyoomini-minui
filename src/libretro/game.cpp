// SPDX-License-Identifier: MIT
#include "game.h"

#include "clock.h"
#include "log.h"

#include "pch.h"
#include "Sound.h"
#include "fullscrn.h"
#include "midi.h"
#include "options.h"
#include "pb.h"
#include "render.h"
#include "winmain.h"

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
  options::Options.Sounds = false;
  options::Options.Music = false;
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
  Sound::Init(false, options::Options.SoundChannels, false, options::Options.SoundVolume);
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
    pb::uninit();
    fullscrn::shutdown();
  }
  g_ready = false;

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
