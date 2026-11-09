// SPDX-License-Identifier: MIT
// Minimal Dear ImGui shim — UI is not rendered under libretro.
#pragma once

#include <cfloat>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

struct ImGuiContext;
struct ImGuiIO;
struct ImFontAtlas;
struct ImFont;
struct ImDrawData;
struct ImGuiStyle;
struct ImGuiSettingsHandler;
struct ImGuiTextBuffer;

using ImU32 = unsigned int;
using ImGuiID = unsigned int;
using ImWchar = unsigned short;

template <typename T>
struct ImVector {
  int Size = 0;
  int Capacity = 0;
  T* Data = nullptr;
  ImVector() = default;
  ~ImVector() { clear(); }
  void clear() {
    if (Data) std::free(Data);
    Data = nullptr;
    Size = Capacity = 0;
  }
  void push_back(const T& v) {
    if (Size + 1 > Capacity) {
      Capacity = Capacity ? Capacity * 2 : 8;
      Data = static_cast<T*>(std::realloc(Data, sizeof(T) * static_cast<size_t>(Capacity)));
    }
    Data[Size++] = v;
  }
  T& operator[](int i) { return Data[i]; }
  const T& operator[](int i) const { return Data[i]; }
  int size() const { return Size; }
  bool empty() const { return Size == 0; }
  T* begin() { return Data; }
  T* end() { return Data + Size; }
  const T* begin() const { return Data; }
  const T* end() const { return Data + Size; }
};

#define IM_COL32(R,G,B,A) (((ImU32)(A)<<24)|((ImU32)(B)<<16)|((ImU32)(G)<<8)|((ImU32)(R)))
#define IM_ARRAYSIZE(_ARR) ((int)(sizeof(_ARR)/sizeof(*(_ARR))))
#define IMGUI_CHECKVERSION() ((void)0)
#define IM_ALLOC(_SIZE) std::malloc(_SIZE)
#define IM_FREE(_PTR) std::free(_PTR)
#define IM_ASSERT(_EXPR) ((void)0)

#include <cstdlib>

struct ImVec2 {
  float x, y;
  ImVec2() : x(0), y(0) {}
  ImVec2(float _x, float _y) : x(_x), y(_y) {}
};

struct ImVec4 {
  float x, y, z, w;
  ImVec4() : x(0), y(0), z(0), w(0) {}
  ImVec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
};

enum ImGuiWindowFlags_ {
  ImGuiWindowFlags_None = 0,
  ImGuiWindowFlags_NoTitleBar = 1 << 0,
  ImGuiWindowFlags_NoResize = 1 << 1,
  ImGuiWindowFlags_NoMove = 1 << 2,
  ImGuiWindowFlags_NoScrollbar = 1 << 3,
  ImGuiWindowFlags_AlwaysAutoResize = 1 << 6,
  ImGuiWindowFlags_NoBackground = 1 << 7,
  ImGuiWindowFlags_NoInputs = 1 << 9,
  ImGuiWindowFlags_MenuBar = 1 << 10,
  ImGuiWindowFlags_NoSavedSettings = 1 << 8,
  ImGuiWindowFlags_NoFocusOnAppearing = 1 << 12,
  ImGuiWindowFlags_HorizontalScrollbar = 1 << 11,
  ImGuiWindowFlags_NoDecoration = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar,
};
typedef int ImGuiWindowFlags;

enum ImGuiTableFlags_ {
  ImGuiTableFlags_None = 0,
  ImGuiTableFlags_Borders = 1 << 7,
  ImGuiTableFlags_NoSavedSettings = 1 << 8,
  ImGuiTableFlags_SizingStretchSame = 1 << 13,
};
typedef int ImGuiTableFlags;

enum ImGuiInputTextFlags_ {
  ImGuiInputTextFlags_None = 0,
  ImGuiInputTextFlags_EnterReturnsTrue = 1 << 5,
  ImGuiInputTextFlags_AutoSelectAll = 1 << 7,
  ImGuiInputTextFlags_CallbackAlways = 1 << 8,
};
typedef int ImGuiInputTextFlags;

enum ImGuiConfigFlags_ {
  ImGuiConfigFlags_NavEnableKeyboard = 1 << 0,
  ImGuiConfigFlags_NavEnableGamepad = 1 << 1,
};
typedef int ImGuiConfigFlags;

enum ImGuiCol_ {
  ImGuiCol_Text = 0,
  ImGuiCol_Button = 1,
  ImGuiCol_COUNT = 2,
};
typedef int ImGuiCol;

enum ImGuiStyleVar_ {
  ImGuiStyleVar_WindowMinSize = 0,
  ImGuiStyleVar_CellPadding = 1,
};
typedef int ImGuiStyleVar;

struct ImFontGlyphRangesBuilder {
  void AddRanges(const ImWchar*) {}
  void AddText(const char*, const char* = nullptr) {}
  void BuildRanges(ImVector<ImWchar>* out) {
    if (!out) return;
    out->push_back(0);
  }
};

struct ImFontAtlas {
  bool Build() { return true; }
  ImFont* AddFontDefault(void* = nullptr) { return nullptr; }
  ImFont* AddFontFromFileTTF(const char*, float, void* = nullptr, const ImWchar* = nullptr) { return nullptr; }
  void Clear() {}
  const ImWchar* GetGlyphRangesDefault() {
    static const ImWchar ranges[] = {0x0020, 0x00FF, 0};
    return ranges;
  }
  // Upstream GroupData.cpp uses this ImGui helper to unpack embedded font bytes.
  static char* DecompressCompressedBase85Data(const char* compressed_data_base85);
};

struct ImGuiIO {
  ImFontAtlas* Fonts = &fontsStorage;
  const char* IniFilename = nullptr;
  ImGuiConfigFlags ConfigFlags = 0;
  bool WantCaptureMouse = false;
  bool WantCaptureKeyboard = false;
  float FontGlobalScale = 1.0f;
  ImFontAtlas fontsStorage;
};

struct ImGuiStyle {
  ImVec4 Colors[ImGuiCol_COUNT];
};

// Full ImGuiSettingsHandler lives in imgui_internal.h; keep a minimal
// definition here so ImGuiContext is complete for options::InitPrimary.
struct ImGuiSettingsHandler {
  const char* TypeName = nullptr;
  ImGuiID TypeHash = 0;
  void* (*ReadOpenFn)(ImGuiContext*, ImGuiSettingsHandler*, const char*) = nullptr;
  void (*ReadLineFn)(ImGuiContext*, ImGuiSettingsHandler*, void*, const char*) = nullptr;
  void (*WriteAllFn)(ImGuiContext*, ImGuiSettingsHandler*, ImGuiTextBuffer*) = nullptr;
  void* UserData = nullptr;
};

struct ImGuiContext {
  ImGuiIO IO;
  bool SettingsLoaded = false;
  std::vector<ImGuiSettingsHandler> SettingsHandlers;
};

namespace ImGui {
ImGuiContext* CreateContext(ImFontAtlas* shared = nullptr);
void DestroyContext(ImGuiContext* ctx = nullptr);
ImGuiContext* GetCurrentContext();
void SetCurrentContext(ImGuiContext* ctx);
ImGuiIO& GetIO();
ImGuiStyle& GetStyle();
void NewFrame();
void EndFrame();
void Render();
ImDrawData* GetDrawData();
void StyleColorsDark(ImGuiStyle* = nullptr);

bool Begin(const char*, bool* p_open = nullptr, ImGuiWindowFlags = 0);
void End();
bool BeginPopupModal(const char*, bool* = nullptr, ImGuiWindowFlags = 0);
void EndPopup();
void OpenPopup(const char*);
void CloseCurrentPopup();
bool IsPopupOpen(const char*);
bool BeginTable(const char*, int, ImGuiTableFlags = 0);
void EndTable();
void TableSetupColumn(const char*, int = 0, float = 0, ImGuiID = 0);
void TableHeadersRow();
void TableNextRow();
bool TableNextColumn();
bool TreeNode(const char*);
void TreePop();
void Text(const char* fmt, ...);
void TextUnformatted(const char* text, const char* text_end = nullptr);
void TextWrapped(const char* fmt, ...);
bool Button(const char* label, const ImVec2& size = ImVec2(0, 0));
void SameLine(float = 0, float = -1);
void Spacing();
void SetItemDefaultFocus();
void SetKeyboardFocusHere(int = 0);
bool IsWindowAppearing();
void SetNextWindowPos(const ImVec2&, int = 0, const ImVec2& = ImVec2(0, 0));
void SetNextWindowSize(const ImVec2&, int = 0);
void SetWindowFontScale(float);
void PushStyleVar(ImGuiStyleVar, const ImVec2&);
void PushStyleVar(ImGuiStyleVar, float);
void PopStyleVar(int = 1);
void PushStyleColor(ImGuiCol, ImU32);
void PushStyleColor(ImGuiCol, const ImVec4&);
void PopStyleColor(int = 1);
void PushItemWidth(float);
void PopItemWidth();
bool InputText(const char*, char*, size_t, ImGuiInputTextFlags = 0, void* = nullptr, void* = nullptr);
bool MenuItem(const char*, const char* = nullptr, bool = false, bool = true);
bool BeginMenuBar();
void EndMenuBar();
bool SliderFloat(const char*, float*, float, float, const char* = "%.3f", int = 0);
void Image(void*, const ImVec2&, const ImVec2& = ImVec2(0, 0), const ImVec2& = ImVec2(1, 1),
           const ImVec4& = ImVec4(1, 1, 1, 1), const ImVec4& = ImVec4(0, 0, 0, 0));
void ShowDemoWindow(bool* = nullptr);
void LoadIniSettingsFromDisk(const char*);
void MarkIniSettingsDirty();
}  // namespace ImGui
