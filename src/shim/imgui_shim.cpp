// SPDX-License-Identifier: MIT
#include "imgui.h"
#include "imgui_internal.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace {
ImGuiContext* g_ctx = nullptr;
ImGuiStyle g_style;
}  // namespace

void ImGuiTextBuffer::appendf(const char* fmt, ...) {
  char tmp[1024];
  va_list ap;
  va_start(ap, fmt);
  std::vsnprintf(tmp, sizeof(tmp), fmt, ap);
  va_end(ap);
  Buf += tmp;
}

ImGuiID ImHashStr(const char* data, size_t data_size, ImGuiID seed) {
  ImGuiID hash = seed ? seed : 2166136261u;
  if (!data) return hash;
  if (data_size == 0) data_size = std::strlen(data);
  for (size_t i = 0; i < data_size; ++i) {
    hash ^= static_cast<ImGuiID>(static_cast<unsigned char>(data[i]));
    hash *= 16777619u;
  }
  return hash;
}

namespace ImGui {

ImGuiContext* CreateContext(ImFontAtlas*) {
  if (!g_ctx) g_ctx = new ImGuiContext();
  return g_ctx;
}

void DestroyContext(ImGuiContext* ctx) {
  if (!ctx) ctx = g_ctx;
  if (ctx == g_ctx) g_ctx = nullptr;
  delete ctx;
}

ImGuiContext* GetCurrentContext() { return g_ctx; }
void SetCurrentContext(ImGuiContext* ctx) { g_ctx = ctx; }
ImGuiIO& GetIO() {
  if (!g_ctx) CreateContext();
  return g_ctx->IO;
}
ImGuiStyle& GetStyle() { return g_style; }
void NewFrame() {}
void EndFrame() {}
void Render() {}
ImDrawData* GetDrawData() { return nullptr; }
void StyleColorsDark(ImGuiStyle*) {}

bool Begin(const char*, bool*, ImGuiWindowFlags) { return false; }
void End() {}
bool BeginPopupModal(const char*, bool*, ImGuiWindowFlags) { return false; }
void EndPopup() {}
void OpenPopup(const char*) {}
void CloseCurrentPopup() {}
bool IsPopupOpen(const char*) { return false; }
bool BeginTable(const char*, int, ImGuiTableFlags) { return false; }
void EndTable() {}
void TableSetupColumn(const char*, int, float, ImGuiID) {}
void TableHeadersRow() {}
void TableNextRow() {}
bool TableNextColumn() { return false; }
bool TreeNode(const char*) { return false; }
void TreePop() {}
void Text(const char*, ...) {}
void TextUnformatted(const char*, const char*) {}
void TextWrapped(const char*, ...) {}
bool Button(const char*, const ImVec2&) { return false; }
void SameLine(float, float) {}
void Spacing() {}
void SetItemDefaultFocus() {}
void SetKeyboardFocusHere(int) {}
bool IsWindowAppearing() { return false; }
void SetNextWindowPos(const ImVec2&, int, const ImVec2&) {}
void SetNextWindowSize(const ImVec2&, int) {}
void SetWindowFontScale(float) {}
void PushStyleVar(ImGuiStyleVar, const ImVec2&) {}
void PushStyleVar(ImGuiStyleVar, float) {}
void PopStyleVar(int) {}
void PushStyleColor(ImGuiCol, ImU32) {}
void PushStyleColor(ImGuiCol, const ImVec4&) {}
void PopStyleColor(int) {}
void PushItemWidth(float) {}
void PopItemWidth() {}
bool InputText(const char*, char*, size_t, ImGuiInputTextFlags, void*, void*) { return false; }
bool MenuItem(const char*, const char*, bool, bool) { return false; }
bool BeginMenuBar() { return false; }
void EndMenuBar() {}
bool SliderFloat(const char*, float*, float, float, const char*, int) { return false; }
void Image(void*, const ImVec2&, const ImVec2&, const ImVec2&, const ImVec4&, const ImVec4&) {}
void ShowDemoWindow(bool*) {}
void LoadIniSettingsFromDisk(const char*) {}
void MarkIniSettingsDirty() {}

}  // namespace ImGui

// --- stb_compress + base85 decompress (from Dear ImGui / stb, MIT / public domain) ---
namespace {
unsigned int stb_decompress_length(const unsigned char* input) {
  return (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11];
}
unsigned char *stb__barrier_out_e, *stb__barrier_out_b;
const unsigned char *stb__barrier_in_b;
unsigned char *stb__dout;
void stb__match(const unsigned char *data, unsigned int length) {
  if (stb__dout + length > stb__barrier_out_e) { stb__dout += length; return; }
  if (data < stb__barrier_out_b) { stb__dout = stb__barrier_out_e+1; return; }
  while (length--) *stb__dout++ = *data++;
}
void stb__lit(const unsigned char *data, unsigned int length) {
  if (stb__dout + length > stb__barrier_out_e) { stb__dout += length; return; }
  if (data < stb__barrier_in_b) { stb__dout = stb__barrier_out_e+1; return; }
  std::memcpy(stb__dout, data, length);
  stb__dout += length;
}
#define stb__in2(x)   ((i[x] << 8) + i[(x)+1])
#define stb__in3(x)   ((i[x] << 16) + stb__in2((x)+1))
#define stb__in4(x)   ((i[x] << 24) + stb__in3((x)+1))
const unsigned char *stb_decompress_token(const unsigned char *i) {
  if (*i >= 0x20) {
    if (*i >= 0x80)       stb__match(stb__dout-i[1]-1, i[0] - 0x80 + 1), i += 2;
    else if (*i >= 0x40)  stb__match(stb__dout-(stb__in2(0) - 0x4000 + 1), i[2]+1), i += 3;
    else                  stb__lit(i+1, i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
  } else {
    if (*i >= 0x18)       stb__match(stb__dout-(stb__in3(0) - 0x180000 + 1), i[3]+1), i += 4;
    else if (*i >= 0x10)  stb__match(stb__dout-(stb__in3(0) - 0x100000 + 1), stb__in2(3)+1), i += 5;
    else if (*i >= 0x08)  stb__lit(i+2, stb__in2(0) - 0x0800 + 1), i += 2 + (stb__in2(0) - 0x0800 + 1);
    else if (*i == 0x07)  stb__lit(i+3, stb__in2(1) + 1), i += 3 + (stb__in2(1) + 1);
    else if (*i == 0x06)  stb__match(stb__dout-(stb__in3(1)+1), i[4]+1), i += 5;
    else if (*i == 0x04)  stb__match(stb__dout-(stb__in3(1)+1), stb__in2(4)+1), i += 6;
  }
  return i;
}
unsigned int stb_adler32(unsigned int adler32, unsigned char *buffer, unsigned int buflen) {
  const unsigned long ADLER_MOD = 65521;
  unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
  unsigned long blocklen = buflen % 5552;
  unsigned long i;
  while (buflen) {
    for (i=0; i + 7 < blocklen; i += 8) {
      s1 += buffer[0], s2 += s1; s1 += buffer[1], s2 += s1;
      s1 += buffer[2], s2 += s1; s1 += buffer[3], s2 += s1;
      s1 += buffer[4], s2 += s1; s1 += buffer[5], s2 += s1;
      s1 += buffer[6], s2 += s1; s1 += buffer[7], s2 += s1;
      buffer += 8;
    }
    for (; i < blocklen; ++i) s1 += *buffer++, s2 += s1;
    s1 %= ADLER_MOD, s2 %= ADLER_MOD;
    buflen -= blocklen;
    blocklen = 5552;
  }
  return (unsigned int)(s2 << 16) + (unsigned int)s1;
}
unsigned int stb_decompress(unsigned char *output, const unsigned char *i, unsigned int) {
  if (stb__in4(0) != 0x57bC0000) return 0;
  if (stb__in4(4) != 0) return 0;
  const unsigned int olen = stb_decompress_length(i);
  stb__barrier_in_b = i;
  stb__barrier_out_e = output + olen;
  stb__barrier_out_b = output;
  i += 16;
  stb__dout = output;
  for (;;) {
    const unsigned char *old_i = i;
    i = stb_decompress_token(i);
    if (i == old_i) {
      if (*i == 0x05 && i[1] == 0xfa) {
        if (stb__dout != output + olen) return 0;
        if (stb_adler32(1, output, olen) != (unsigned int) stb__in4(2)) return 0;
        return olen;
      }
      return 0;
    }
    if (stb__dout > output + olen) return 0;
  }
}
unsigned int Decode85Byte(char c) { return c >= '\\' ? c-36 : c-35; }
void Decode85(const unsigned char* src, unsigned char* dst) {
  while (*src) {
    unsigned int tmp = Decode85Byte(src[0]) + 85*(Decode85Byte(src[1]) + 85*(Decode85Byte(src[2]) + 85*(Decode85Byte(src[3]) + 85*Decode85Byte(src[4]))));
    dst[0] = ((tmp >> 0) & 0xFF); dst[1] = ((tmp >> 8) & 0xFF); dst[2] = ((tmp >> 16) & 0xFF); dst[3] = ((tmp >> 24) & 0xFF);
    src += 5; dst += 4;
  }
}
}  // namespace

char* ImFontAtlas::DecompressCompressedBase85Data(const char* compressed_data_base85) {
  int compressed_data_size = (((int)std::strlen(compressed_data_base85) + 4) / 5) * 4;
  void* compressed_data = IM_ALLOC((size_t)compressed_data_size);
  Decode85((const unsigned char*)compressed_data_base85, (unsigned char*)compressed_data);
  const unsigned int buf_decompressed_size = stb_decompress_length((const unsigned char*)compressed_data);
  unsigned char* buf_decompressed_data = (unsigned char*)IM_ALLOC(buf_decompressed_size);
  stb_decompress(buf_decompressed_data, (const unsigned char*)compressed_data, (unsigned int)compressed_data_size);
  IM_FREE(compressed_data);
  return (char*)buf_decompressed_data;
}
