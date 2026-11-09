// SPDX-License-Identifier: MIT
#pragma once

#include "game.h"

#include <cstdint>

namespace scp {

inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return static_cast<uint16_t>(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

// Convert upstream ColorRgba buffer (0xAARRGGBB) to RGB565.
// src_stride / dst_pitch are in pixels.
void convert_rgba_to_565(const uint32_t* src, int src_stride,
                         uint16_t* dst, int dst_pitch_px,
                         int w, int h);

// 8bpp indexed path (palette LUT). Used by unit tests and future dirty-rect opts.
void convert_8_to_565(const uint8_t* src, int src_stride,
                      const uint16_t* pal565, uint16_t* dst, int dst_pitch_px,
                      int w, int h);

void build_pal565_from_rgba(const uint32_t* palette256, uint16_t* out565);

// Present game framebuffer into a fixed 640x480 RGB565 buffer (centered).
// Returns false if view is invalid (dst left unchanged aside from optional clear).
bool present_framebuffer(const FramebufferView& view, uint16_t* dst640x480, bool clear_letterbox = true);

void video_set_show_fps(bool enabled);
void video_on_frame_presented();  // call once per presented frame when FPS overlay is on
void video_draw_fps_overlay(uint16_t* dst640x480);

}  // namespace scp
