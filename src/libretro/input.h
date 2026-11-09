// SPDX-License-Identifier: MIT
#pragma once

#include "libretro.h"

namespace scp {

struct InputOptions {
  bool plunger_tap = false;   // false = hold-release
  bool nudge_enabled = true;
};

// Aggregated RetroPad state after OnionOS aliases (L|L2, R|R2, A|B).
struct InputButtons {
  bool left_flipper = false;
  bool right_flipper = false;
  bool plunger = false;
  bool nudge_left = false;
  bool nudge_right = false;
  bool nudge_bottom = false;
  bool high_scores = false;
  bool toggle_sound = false;
  bool toggle_music = false;
  bool pause = false;
  bool new_game = false;
};

// What the translator last told the game for hold-style actions.
struct InputHoldState {
  bool left_flipper = false;
  bool right_flipper = false;
  bool plunger = false;
  bool nudge_left = false;
  bool nudge_right = false;
  bool nudge_bottom = false;
};

enum class InputEventKind {
  LeftFlipperDown,
  LeftFlipperUp,
  RightFlipperDown,
  RightFlipperUp,
  PlungerDown,
  PlungerUp,
  NudgeLeftDown,
  NudgeLeftUp,
  NudgeRightDown,
  NudgeRightUp,
  NudgeBottomDown,
  NudgeBottomUp,
  HighScores,
  ToggleSound,
  ToggleMusic,
  Pause,
  NewGame,
};

struct InputEvent {
  InputEventKind kind;
};

constexpr int kMaxInputEvents = 32;

void register_input_descriptors(retro_environment_t env_cb);

InputButtons read_input_buttons(retro_input_state_t state_cb);

// Pure edge translator (no I/O). `gameplay_blocked` is the pre-frame pause/overlay
// state; Pause toggles it for same-frame hold sync, High Scores forces blocked,
// New Game clears blocked.
int translate_input_edges(const InputButtons& now,
                          InputButtons& prev,
                          InputHoldState& holds,
                          const InputOptions& opts,
                          bool gameplay_blocked,
                          InputEvent* out,
                          int max_out);

void poll_input(retro_input_poll_t poll_cb,
                retro_input_state_t state_cb,
                const InputOptions& opts);

void input_reset();

}  // namespace scp
