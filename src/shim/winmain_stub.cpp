// SPDX-License-Identifier: MIT
// Replaces upstream winmain.cpp under libretro: keep symbols, drop SDL UI loop.
#include "pch.h"
#include "winmain.h"

#include "options.h"
#include "pb.h"

#include <cstdlib>

constexpr const char* winmain::Version;

SDL_Window* winmain::MainWindow = nullptr;
SDL_Renderer* winmain::Renderer = nullptr;
ImGuiIO* winmain::ImIO = nullptr;
int winmain::return_value = 0;
bool winmain::bQuit = false;
bool winmain::activated = true;
bool winmain::DispFrameRate = false;
bool winmain::DispGRhistory = false;
bool winmain::single_step = false;
bool winmain::has_focus = true;
int winmain::last_mouse_x = 0;
int winmain::last_mouse_y = 0;
int winmain::mouse_down = 0;
bool winmain::no_time_loss = false;
bool winmain::restart = false;
std::vector<float> winmain::gfrDisplay{};
unsigned winmain::gfrOffset = 0;
float winmain::gfrWindow = 5.0f;
bool winmain::ShowAboutDialog = false;
bool winmain::ShowImGuiDemo = false;
bool winmain::ShowSpriteViewer = false;
bool winmain::ShowExitPopup = false;
bool winmain::LaunchBallEnabled = true;
bool winmain::HighScoresEnabled = true;
bool winmain::DemoActive = false;
int winmain::MainMenuHeight = 0;
std::string winmain::FpsDetails, winmain::PrevSdlError;
unsigned winmain::PrevSdlErrorCount = 0;
double winmain::UpdateToFrameRatio = 1.0;
winmain::DurationMs winmain::TargetFrameTime{1000.0 / 60.0};
optionsStruct& winmain::Options = options::Options;
winmain::DurationMs winmain::SpinThreshold = DurationMs(0.005);
WelfordState winmain::SleepState{};
int winmain::CursorIdleCounter = 0;

int winmain::WinMain(LPCSTR) { return 0; }
void winmain::MainLoop() {}
void winmain::RenderUi() {}
int winmain::event_handler(const SDL_Event*) { return 1; }
int winmain::ProcessWindowMessages() { return 1; }

void winmain::memalloc_failure() {
  bQuit = true;
  std::abort();
}

void winmain::a_dialog() {}
void winmain::end_pause() { single_step = false; }

void winmain::new_game() {
  end_pause();
  pb::replay_level(false);
}

void winmain::pause(bool toggle) {
  if (toggle || !single_step)
    single_step ^= true;
  else
    single_step = true;
}

void winmain::Restart() { restart = true; }
void winmain::UpdateFrameRate() {}

void winmain::HandleGameBinding(GameBindings binding, bool) {
  switch (binding) {
    case GameBindings::TogglePause:
      pause();
      break;
    case GameBindings::NewGame:
      new_game();
      break;
    case GameBindings::ToggleSounds:
      options::toggle(Menu1::Sounds);
      break;
    case GameBindings::ToggleMusic:
      options::toggle(Menu1::Music);
      break;
    default:
      break;
  }
}

void winmain::RenderFrameTimeDialog() {}
void winmain::HybridSleep(DurationMs) {}
void winmain::ImGuiMenuItemWShortcut(GameBindings, bool) {}
