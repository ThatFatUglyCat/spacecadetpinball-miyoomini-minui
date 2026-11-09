// SPDX-License-Identifier: MIT
#pragma once

#include "libretro.h"

namespace scp {

void register_input_descriptors(retro_environment_t env_cb);

// Phase 4 will translate RetroPad state into game events.
// For Phase 1 this only records descriptors for minarch / default.cfg.
void poll_input(retro_input_poll_t poll_cb, retro_input_state_t state_cb);

}  // namespace scp
