// SPDX-License-Identifier: MIT
#pragma once

#include "imgui.h"

#include <string>

// ImGuiSettingsHandler is defined in imgui.h for this shim.
struct ImGuiTextBuffer {
  std::string Buf;
  void append(const char* str) { if (str) Buf += str; }
  void appendf(const char* fmt, ...);
  const char* c_str() const { return Buf.c_str(); }
  void clear() { Buf.clear(); }
};

ImGuiID ImHashStr(const char* data, size_t data_size = 0, ImGuiID seed = 0);
