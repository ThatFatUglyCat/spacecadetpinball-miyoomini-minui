// SPDX-License-Identifier: MIT
#include "input.h"

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

}  // namespace

void register_input_descriptors(retro_environment_t env_cb) {
  if (!env_cb) return;
  env_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS,
         const_cast<retro_input_descriptor*>(kDescriptors));
}

void poll_input(retro_input_poll_t poll_cb, retro_input_state_t /*state_cb*/) {
  if (poll_cb) poll_cb();
  // Game event translation lands in Phase 4.
}

}  // namespace scp
