// SPDX-License-Identifier: MIT
#include "video.h"

#include "errorscreen.h"

#include <cstdio>
#include <cstring>

namespace scp {
namespace {

bool g_show_fps = false;
unsigned g_frame_counter = 0;
unsigned g_fps_display = 0;
unsigned g_fps_frames = 0;

// Tiny digit rendering reused conceptually from errorscreen (inline for overlay only).
void put_pixel(uint16_t* dst, int x, int y, uint16_t color) {
  if (x < 0 || y < 0 || x >= kScreenW || y >= kScreenH) return;
  dst[y * kScreenW + x] = color;
}

void draw_char5x7(uint16_t* dst, int x, int y, char ch, uint16_t fg) {
  // Minimal digits + letters needed for "FPS:123"
  static const uint8_t font[16][7] = {
    {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E}, // 0
    {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E},
    {0x0E,0x11,0x01,0x06,0x08,0x10,0x1F},
    {0x1F,0x01,0x02,0x06,0x01,0x11,0x0E},
    {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02},
    {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E},
    {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E},
    {0x1F,0x01,0x02,0x04,0x08,0x08,0x08},
    {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E},
    {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C}, // 9
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10}, // F
    {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10}, // P
    {0x0E,0x11,0x10,0x0E,0x01,0x11,0x0E}, // S
    {0x00,0x00,0x00,0x1F,0x00,0x00,0x00}, // -
    {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C}, // :
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // space
  };
  int idx = 15;
  if (ch >= '0' && ch <= '9') idx = ch - '0';
  else if (ch == 'F') idx = 10;
  else if (ch == 'P') idx = 11;
  else if (ch == 'S') idx = 12;
  else if (ch == '-') idx = 13;
  else if (ch == ':') idx = 14;
  for (int row = 0; row < 7; ++row) {
    uint8_t bits = font[idx][row];
    for (int col = 0; col < 5; ++col) {
      if (bits & (1u << (4 - col))) put_pixel(dst, x + col, y + row, fg);
    }
  }
}

void draw_text(uint16_t* dst, int x, int y, const char* text, uint16_t fg) {
  int cx = x;
  for (const char* p = text; *p; ++p) {
    draw_char5x7(dst, cx, y, *p, fg);
    cx += 6;
  }
}

}  // namespace

void convert_rgba_to_565(const uint32_t* src, int src_stride,
                         uint16_t* dst, int dst_pitch_px,
                         int w, int h) {
  for (int y = 0; y < h; ++y) {
    const uint32_t* s = src + static_cast<std::size_t>(y) * static_cast<std::size_t>(src_stride);
    uint16_t* d = dst + static_cast<std::size_t>(y) * static_cast<std::size_t>(dst_pitch_px);
    for (int x = 0; x < w; ++x) {
      const uint32_t c = s[x];
      const uint8_t r = static_cast<uint8_t>((c >> 16) & 0xffu);
      const uint8_t g = static_cast<uint8_t>((c >> 8) & 0xffu);
      const uint8_t b = static_cast<uint8_t>(c & 0xffu);
      d[x] = rgb565(r, g, b);
    }
  }
}

void convert_8_to_565(const uint8_t* src, int src_stride,
                      const uint16_t* pal565, uint16_t* dst, int dst_pitch_px,
                      int w, int h) {
  for (int y = 0; y < h; ++y) {
    const uint8_t* s = src + static_cast<std::size_t>(y) * static_cast<std::size_t>(src_stride);
    uint16_t* d = dst + static_cast<std::size_t>(y) * static_cast<std::size_t>(dst_pitch_px);
    for (int x = 0; x < w; ++x) d[x] = pal565[s[x]];
  }
}

void build_pal565_from_rgba(const uint32_t* palette256, uint16_t* out565) {
  for (int i = 0; i < 256; ++i) {
    const uint32_t c = palette256[i];
    out565[i] = rgb565(static_cast<uint8_t>((c >> 16) & 0xffu),
                       static_cast<uint8_t>((c >> 8) & 0xffu),
                       static_cast<uint8_t>(c & 0xffu));
  }
}

bool present_framebuffer(const FramebufferView& view, uint16_t* dst640x480, bool clear_letterbox) {
  if (!dst640x480 || !view.valid || !view.rgba || view.width <= 0 || view.height <= 0)
    return false;

  if (clear_letterbox)
    std::memset(dst640x480, 0, static_cast<std::size_t>(kScreenW * kScreenH) * sizeof(uint16_t));

  const int w = view.width < kScreenW ? view.width : kScreenW;
  const int h = view.height < kScreenH ? view.height : kScreenH;
  const int ox = (kScreenW - w) / 2;
  const int oy = (kScreenH - h) / 2;

  const auto* src = static_cast<const uint32_t*>(view.rgba);
  for (int y = 0; y < h; ++y) {
    const uint32_t* s = src + static_cast<std::size_t>(y) * static_cast<std::size_t>(view.stride);
    uint16_t* d = dst640x480 + static_cast<std::size_t>(oy + y) * kScreenW + ox;
    for (int x = 0; x < w; ++x) {
      const uint32_t c = s[x];
      d[x] = rgb565(static_cast<uint8_t>((c >> 16) & 0xffu),
                    static_cast<uint8_t>((c >> 8) & 0xffu),
                    static_cast<uint8_t>(c & 0xffu));
    }
  }
  return true;
}

void video_set_show_fps(bool enabled) {
  if (g_show_fps == enabled) return;
  g_show_fps = enabled;
  g_frame_counter = 0;
  g_fps_frames = 0;
  g_fps_display = 0;
}

void video_on_frame_presented() {
  if (!g_show_fps) return;
  ++g_frame_counter;
  ++g_fps_frames;
  // Rough 1-second window at 60 Hz target.
  if (g_frame_counter >= 60) {
    g_fps_display = g_fps_frames;
    g_fps_frames = 0;
    g_frame_counter = 0;
  }
}

void video_draw_fps_overlay(uint16_t* dst640x480) {
  if (!g_show_fps || !dst640x480) return;
  char buf[32];
  std::snprintf(buf, sizeof(buf), "FPS:%u", g_fps_display);
  draw_text(dst640x480, 8, 8, buf, rgb565(255, 255, 0));
}

}  // namespace scp
