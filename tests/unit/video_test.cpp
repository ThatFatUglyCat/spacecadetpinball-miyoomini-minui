// SPDX-License-Identifier: MIT
#include "video.h"

#include <cstdio>
#include <cstring>
#include <vector>

static int failures = 0;

static void expect(bool cond, const char* msg) {
  if (!cond) {
    std::fprintf(stderr, "FAIL: %s\n", msg);
    ++failures;
  }
}

int main() {
  {
    expect(scp::rgb565(255, 0, 0) == 0xF800, "red");
    expect(scp::rgb565(0, 255, 0) == 0x07E0, "green");
    expect(scp::rgb565(0, 0, 255) == 0x001F, "blue");
    expect(scp::rgb565(255, 255, 255) == 0xFFFF, "white");
  }

  {
    uint32_t src[8] = {
      0xFF0000FFu,  // blue in 0xAARRGGBB? Wait: blue = 0xFF0000FF means R=0,G=0,B=255
      0xFFFF0000u,  // red
      0xFF00FF00u,  // green
      0xFFFFFFFFu,  // white
      0, 0, 0, 0
    };
    // Layout: alpha<<24|red<<16|green<<8|blue
    src[0] = 0xFF0000FFu;  // R=0 G=0 B=255
    src[1] = 0xFFFF0000u;  // R=255
    src[2] = 0xFF00FF00u;  // G=255
    src[3] = 0xFFFFFFFFu;

    uint16_t dst[4] = {};
    scp::convert_rgba_to_565(src, 4, dst, 4, 4, 1);
    expect(dst[0] == scp::rgb565(0, 0, 255), "rgba blue");
    expect(dst[1] == scp::rgb565(255, 0, 0), "rgba red");
    expect(dst[2] == scp::rgb565(0, 255, 0), "rgba green");
    expect(dst[3] == scp::rgb565(255, 255, 255), "rgba white");
  }

  {
    uint32_t pal[256] = {};
    pal[1] = 0xFFFF0000u;
    pal[2] = 0xFF00FF00u;
    uint16_t lut[256] = {};
    scp::build_pal565_from_rgba(pal, lut);
    expect(lut[1] == scp::rgb565(255, 0, 0), "pal red");
    expect(lut[2] == scp::rgb565(0, 255, 0), "pal green");

    uint8_t idx[4] = {1, 2, 0, 1};
    uint16_t out[4] = {};
    scp::convert_8_to_565(idx, 4, lut, out, 4, 4, 1);
    expect(out[0] == lut[1] && out[1] == lut[2] && out[2] == lut[0] && out[3] == lut[1],
           "indexed convert");
  }

  {
    // 2x2 source centered into 640x480
    uint32_t rgba[4] = {0xFFFF0000u, 0xFF00FF00u, 0xFF0000FFu, 0xFFFFFFFFu};
    scp::FramebufferView view;
    view.rgba = rgba;
    view.width = 2;
    view.height = 2;
    view.stride = 2;
    view.valid = true;

    std::vector<uint16_t> fb(640 * 480, 0xABCDu);
    expect(scp::present_framebuffer(view, fb.data(), true), "present ok");
    const int ox = (640 - 2) / 2;
    const int oy = (480 - 2) / 2;
    expect(fb[0] == 0, "letterbox cleared");
    expect(fb[oy * 640 + ox] == scp::rgb565(255, 0, 0), "center tl");
    expect(fb[oy * 640 + ox + 1] == scp::rgb565(0, 255, 0), "center tr");
    expect(fb[(oy + 1) * 640 + ox] == scp::rgb565(0, 0, 255), "center bl");
    expect(fb[(oy + 1) * 640 + ox + 1] == scp::rgb565(255, 255, 255), "center br");
  }

  {
    scp::FramebufferView bad;
    uint16_t fb[10] = {};
    expect(!scp::present_framebuffer(bad, fb, true), "invalid view rejected");
  }

  if (failures) {
    std::fprintf(stderr, "%d failure(s)\n", failures);
    return 1;
  }
  std::puts("VIDEO UNIT OK");
  return 0;
}
