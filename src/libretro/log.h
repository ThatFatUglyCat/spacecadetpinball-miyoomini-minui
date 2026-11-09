// SPDX-License-Identifier: MIT
#pragma once

#include "libretro.h"

#include <cstdarg>
#include <cstdio>

namespace scp {

extern retro_log_printf_t g_log_cb;

inline void log(enum retro_log_level level, const char* fmt, ...) {
  if (!g_log_cb) return;
  va_list ap;
  va_start(ap, fmt);
  char buf[512];
  std::vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  g_log_cb(level, "%s", buf);
}

}  // namespace scp
