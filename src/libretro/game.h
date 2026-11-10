// SPDX-License-Identifier: MIT
#pragma once

namespace scp {

struct InputEvent;

struct GameConfig {
  const char* dat_path = nullptr;   // full path to PINBALL.DAT / CADET.DAT
  const char* system_dir = nullptr; // optional (SF2 under PINBALL/)
  const char* save_dir = nullptr;   // optional
  bool music_enabled = false;       // core option scp_music
};

bool game_init(const GameConfig& cfg);
void game_step(float dt_ms);
void game_shutdown();
bool game_ready();

// True when gameplay holds (flippers/plunger/nudge) must be suppressed.
bool game_input_blocked();

bool game_high_scores_visible();
void game_set_high_scores_visible(bool visible);

struct HighScoreTableView;
HighScoreTableView game_high_score_table();

// Import SAVE_RAM → engine table (call once after frontend fills SAVE_RAM).
void game_import_save_ram();
// Flush pending high-score queue and export table → SAVE_RAM.
void game_export_save_ram();

void game_handle_input(const InputEvent* events, int count);

// Sync core option scp_music → upstream Options.Music + play/stop.
void game_set_music_enabled(bool enabled);

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
