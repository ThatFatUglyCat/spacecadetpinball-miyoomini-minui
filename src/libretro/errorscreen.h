// SPDX-License-Identifier: MIT
#pragma once

#include <cstdint>

namespace scp {

constexpr int kScreenW = 640;
constexpr int kScreenH = 480;

// Render a solid RGB565 help screen with a short message (ASCII).
void render_error_screen(uint16_t* dst, int pitch_px, const char* message);

}  // namespace scp
