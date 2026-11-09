// SPDX-License-Identifier: MIT
// Minimal stubs so input.cpp (which calls into game_*) links in the unit test.
#include "game.h"
#include "input.h"

namespace scp {

bool game_ready() { return true; }
bool game_input_blocked() { return false; }
void game_handle_input(const InputEvent*, int) {}

}  // namespace scp
