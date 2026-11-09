// SPDX-License-Identifier: MIT
#pragma once

namespace scp {

struct GameConfig {
  const char* dat_path = nullptr;   // full path to PINBALL.DAT / CADET.DAT
  const char* system_dir = nullptr; // optional
  const char* save_dir = nullptr;   // optional
};

bool game_init(const GameConfig& cfg);
void game_step(float dt_ms);
void game_shutdown();
bool game_ready();

// Expose vscreen for Phase 3 video conversion.
struct FramebufferView {
  const unsigned char* indexed = nullptr;  // may be null; prefer rgba
  const void* rgba = nullptr;              // ColorRgba*
  int width = 0;
  int height = 0;
  int stride = 0;  // pixels per row
  bool valid = false;
};

FramebufferView game_framebuffer();

}  // namespace scp
