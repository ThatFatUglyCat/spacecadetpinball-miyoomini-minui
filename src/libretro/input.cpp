// SPDX-License-Identifier: MIT
#include "input.h"

#include "game.h"

namespace scp {
namespace {

// Labels must match pak/PINBALL.pak/default.cfg bind names and docs/INPUT_MAPPING.md.
const struct retro_input_descriptor kDescriptors[] = {
  {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,      "Left Flipper"},
  {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2,     "Left Flipper"},
  {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,      "Right Flipper"},
  {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2,     "Right Flipper"},
  {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,      "Plunger"},
  {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,      "Plunger"},
  {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,   "Nudge Left"},
  {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  "Nudge Right"},
  {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,     "Nudge Bottom"},
  {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,   "High Scores"},
  {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,      "Toggle Sound"},
  {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,      "Toggle Music"},
  {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,  "Pause"},
  {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "New Game"},
  {0, 0, 0, 0, nullptr},
};

InputButtons g_prev{};
InputHoldState g_holds{};

bool joy(retro_input_state_t state_cb, unsigned id) {
  return state_cb && state_cb(0, RETRO_DEVICE_JOYPAD, 0, id) != 0;
}

int push_event(InputEvent* out, int max_out, int count, InputEventKind kind) {
  if (count >= max_out) return count;
  out[count].kind = kind;
  return count + 1;
}

int sync_hold(bool desired,
              bool& engine_held,
              InputEventKind down_kind,
              InputEventKind up_kind,
              InputEvent* out,
              int max_out,
              int count) {
  if (desired == engine_held) return count;
  if (desired) {
    count = push_event(out, max_out, count, down_kind);
    engine_held = true;
  } else {
    count = push_event(out, max_out, count, up_kind);
    engine_held = false;
  }
  return count;
}

}  // namespace

void register_input_descriptors(retro_environment_t env_cb) {
  if (!env_cb) return;
  env_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS,
         const_cast<retro_input_descriptor*>(kDescriptors));
}

InputButtons read_input_buttons(retro_input_state_t state_cb) {
  InputButtons b;
  b.left_flipper = joy(state_cb, RETRO_DEVICE_ID_JOYPAD_L) ||
                   joy(state_cb, RETRO_DEVICE_ID_JOYPAD_L2);
  b.right_flipper = joy(state_cb, RETRO_DEVICE_ID_JOYPAD_R) ||
                    joy(state_cb, RETRO_DEVICE_ID_JOYPAD_R2);
  b.plunger = joy(state_cb, RETRO_DEVICE_ID_JOYPAD_A) ||
              joy(state_cb, RETRO_DEVICE_ID_JOYPAD_B);
  b.nudge_left = joy(state_cb, RETRO_DEVICE_ID_JOYPAD_LEFT);
  b.nudge_right = joy(state_cb, RETRO_DEVICE_ID_JOYPAD_RIGHT);
  b.nudge_bottom = joy(state_cb, RETRO_DEVICE_ID_JOYPAD_UP);
  b.high_scores = joy(state_cb, RETRO_DEVICE_ID_JOYPAD_DOWN);
  b.toggle_sound = joy(state_cb, RETRO_DEVICE_ID_JOYPAD_X);
  b.toggle_music = joy(state_cb, RETRO_DEVICE_ID_JOYPAD_Y);
  b.pause = joy(state_cb, RETRO_DEVICE_ID_JOYPAD_START);
  b.new_game = joy(state_cb, RETRO_DEVICE_ID_JOYPAD_SELECT);
  return b;
}

int translate_input_edges(const InputButtons& now,
                          InputButtons& prev,
                          InputHoldState& holds,
                          const InputOptions& opts,
                          bool gameplay_blocked,
                          InputEvent* out,
                          int max_out) {
  if (!out || max_out <= 0) return 0;
  int n = 0;
  bool blocked = gameplay_blocked;

  if (now.pause && !prev.pause) {
    n = push_event(out, max_out, n, InputEventKind::Pause);
    blocked = !blocked;
  }
  if (now.new_game && !prev.new_game) {
    n = push_event(out, max_out, n, InputEventKind::NewGame);
    blocked = false;
  }
  if (now.high_scores && !prev.high_scores) {
    n = push_event(out, max_out, n, InputEventKind::HighScores);
    blocked = true;
  }
  if (now.toggle_sound && !prev.toggle_sound)
    n = push_event(out, max_out, n, InputEventKind::ToggleSound);
  if (now.toggle_music && !prev.toggle_music)
    n = push_event(out, max_out, n, InputEventKind::ToggleMusic);

  const bool allow = !blocked;

  n = sync_hold(allow && now.left_flipper, holds.left_flipper,
                InputEventKind::LeftFlipperDown, InputEventKind::LeftFlipperUp,
                out, max_out, n);
  n = sync_hold(allow && now.right_flipper, holds.right_flipper,
                InputEventKind::RightFlipperDown, InputEventKind::RightFlipperUp,
                out, max_out, n);

  if (opts.plunger_tap) {
    if (allow && now.plunger && !prev.plunger) {
      n = push_event(out, max_out, n, InputEventKind::PlungerDown);
      n = push_event(out, max_out, n, InputEventKind::PlungerUp);
      holds.plunger = false;
    } else if (!allow && holds.plunger) {
      n = push_event(out, max_out, n, InputEventKind::PlungerUp);
      holds.plunger = false;
    }
  } else {
    n = sync_hold(allow && now.plunger, holds.plunger,
                  InputEventKind::PlungerDown, InputEventKind::PlungerUp,
                  out, max_out, n);
  }

  const bool nudge = opts.nudge_enabled;
  n = sync_hold(allow && nudge && now.nudge_left, holds.nudge_left,
                InputEventKind::NudgeLeftDown, InputEventKind::NudgeLeftUp,
                out, max_out, n);
  n = sync_hold(allow && nudge && now.nudge_right, holds.nudge_right,
                InputEventKind::NudgeRightDown, InputEventKind::NudgeRightUp,
                out, max_out, n);
  n = sync_hold(allow && nudge && now.nudge_bottom, holds.nudge_bottom,
                InputEventKind::NudgeBottomDown, InputEventKind::NudgeBottomUp,
                out, max_out, n);

  prev = now;
  return n;
}

void poll_input(retro_input_poll_t poll_cb,
                retro_input_state_t state_cb,
                const InputOptions& opts) {
  if (poll_cb) poll_cb();
  if (!game_ready()) return;

  const InputButtons now = read_input_buttons(state_cb);
  InputEvent events[kMaxInputEvents];
  const int count = translate_input_edges(
      now, g_prev, g_holds, opts, game_input_blocked(), events, kMaxInputEvents);
  if (count > 0)
    game_handle_input(events, count);
}

void input_reset() {
  g_prev = InputButtons{};
  g_holds = InputHoldState{};
}

}  // namespace scp
