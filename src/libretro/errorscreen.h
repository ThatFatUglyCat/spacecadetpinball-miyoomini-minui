// SPDX-License-Identifier: MIT
#pragma once

#include <cstdint>

namespace scp {

constexpr int kScreenW = 640;
constexpr int kScreenH = 480;

inline uint16_t pack_rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return static_cast<uint16_t>(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

// Render a solid RGB565 help screen with a short message (ASCII).
void render_error_screen(uint16_t* dst, int pitch_px, const char* message);

// Shared 5x7 bitmap text (ASCII). Used by error screen and high-score overlay.
void draw_text_rgb565(uint16_t* dst, int pitch_px, int x, int y, const char* text, uint16_t fg);

struct HighScoreTableView;

// Panel + table of high scores over an existing frame.
void render_high_score_overlay(uint16_t* dst, int pitch_px, const HighScoreTableView& table);

}  // namespace scp
