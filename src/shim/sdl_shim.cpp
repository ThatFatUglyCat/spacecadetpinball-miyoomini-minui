// SPDX-License-Identifier: MIT
#include "SDL.h"

#include "clock.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

struct SDL_Window { int w, h; };
struct SDL_Renderer { int w, h; };
struct SDL_Texture {
  int w = 0, h = 0;
  std::vector<uint8_t> pixels;
};

namespace {
std::string g_error;
std::map<std::string, std::string> g_hints;
Uint8 g_keyboard[512]{};
}  // namespace

extern "C" {

int SDL_Init(Uint32) { return 0; }
void SDL_Quit(void) {}
const char* SDL_GetError(void) { return g_error.c_str(); }
void SDL_ClearError(void) { g_error.clear(); }
void SDL_free(void* mem) { std::free(mem); }
void* SDL_malloc(size_t size) { return std::malloc(size); }

Uint32 SDL_GetTicks(void) {
  return static_cast<Uint32>(scp::clock_ms());
}
Uint64 SDL_GetPerformanceCounter(void) {
  return static_cast<Uint64>(scp::clock_us());
}
Uint64 SDL_GetPerformanceFrequency(void) { return 1000000ull; }
void SDL_Delay(Uint32) {}

const char* SDL_GetHint(const char* name) {
  auto it = g_hints.find(name ? name : "");
  return it == g_hints.end() ? nullptr : it->second.c_str();
}
SDL_bool SDL_SetHint(const char* name, const char* value) {
  if (name && value) g_hints[name] = value;
  return SDL_TRUE;
}

char* SDL_GetBasePath(void) {
  char* p = static_cast<char*>(std::malloc(3));
  if (p) { p[0] = '.'; p[1] = '/'; p[2] = 0; }
  return p;
}
char* SDL_GetPrefPath(const char*, const char*) {
  return SDL_GetBasePath();
}

SDL_Window* SDL_CreateWindow(const char*, int, int, int w, int h, Uint32) {
  return new SDL_Window{w, h};
}
void SDL_DestroyWindow(SDL_Window* window) { delete window; }
void SDL_ShowWindow(SDL_Window*) {}
void SDL_SetWindowSize(SDL_Window* window, int w, int h) {
  if (window) { window->w = w; window->h = h; }
}
int SDL_SetWindowFullscreen(SDL_Window*, Uint32) { return 0; }
void SDL_SetWindowGrab(SDL_Window*, SDL_bool) {}
int SDL_ShowCursor(int) { return 0; }

SDL_Renderer* SDL_CreateRenderer(SDL_Window* window, int, Uint32) {
  auto* r = new SDL_Renderer;
  r->w = window ? window->w : 640;
  r->h = window ? window->h : 480;
  return r;
}
void SDL_DestroyRenderer(SDL_Renderer* renderer) { delete renderer; }
int SDL_GetRendererInfo(SDL_Renderer*, SDL_RendererInfo* info) {
  if (!info) return -1;
  std::memset(info, 0, sizeof(*info));
  info->name = "shim";
  return 0;
}
int SDL_GetRendererOutputSize(SDL_Renderer* renderer, int* w, int* h) {
  if (w) *w = renderer ? renderer->w : 640;
  if (h) *h = renderer ? renderer->h : 480;
  return 0;
}
int SDL_SetRenderDrawColor(SDL_Renderer*, Uint8, Uint8, Uint8, Uint8) { return 0; }
int SDL_RenderClear(SDL_Renderer*) { return 0; }
int SDL_RenderFillRect(SDL_Renderer*, const SDL_Rect*) { return 0; }
int SDL_RenderCopy(SDL_Renderer*, SDL_Texture*, const SDL_Rect*, const SDL_Rect*) { return 0; }
int SDL_RenderCopyF(SDL_Renderer*, SDL_Texture*, const SDL_Rect*, const SDL_FRect*) { return 0; }
void SDL_RenderPresent(SDL_Renderer*) {}

SDL_Texture* SDL_CreateTexture(SDL_Renderer*, Uint32, int, int w, int h) {
  auto* t = new SDL_Texture;
  t->w = w;
  t->h = h;
  t->pixels.assign(static_cast<size_t>(w) * static_cast<size_t>(h) * 4u, 0);
  return t;
}
void SDL_DestroyTexture(SDL_Texture* texture) { delete texture; }
int SDL_SetTextureBlendMode(SDL_Texture*, int) { return 0; }
int SDL_UpdateTexture(SDL_Texture* texture, const SDL_Rect*, const void* pixels, int pitch) {
  if (!texture || !pixels) return -1;
  const int row = texture->w * 4;
  auto* dst = texture->pixels.data();
  const auto* src = static_cast<const uint8_t*>(pixels);
  for (int y = 0; y < texture->h; ++y)
    std::memcpy(dst + y * row, src + y * pitch, static_cast<size_t>(row));
  return 0;
}
int SDL_LockTexture(SDL_Texture* texture, const SDL_Rect*, void** pixels, int* pitch) {
  if (!texture || !pixels || !pitch) return -1;
  *pixels = texture->pixels.data();
  *pitch = texture->w * 4;
  return 0;
}
void SDL_UnlockTexture(SDL_Texture*) {}

int SDL_PollEvent(SDL_Event*) { return 0; }
int SDL_PushEvent(SDL_Event*) { return 1; }
const Uint8* SDL_GetKeyboardState(int* numkeys) {
  if (numkeys) *numkeys = 512;
  return g_keyboard;
}

int SDL_ShowSimpleMessageBox(Uint32, const char*, const char*, SDL_Window*) { return 0; }

SDL_RWops* SDL_RWFromFile(const char* file, const char* mode) {
  if (!file) return nullptr;
  FILE* fp = std::fopen(file, mode ? mode : "rb");
  if (!fp) return nullptr;
  auto* rw = new SDL_RWops();
  std::memset(rw, 0, sizeof(*rw));
  rw->kind = 0;
  rw->handle = fp;
  return rw;
}

SDL_RWops* SDL_RWFromMem(void* mem, int size) {
  if (!mem || size <= 0) return nullptr;
  auto* rw = new SDL_RWops();
  std::memset(rw, 0, sizeof(*rw));
  rw->kind = 1;
  rw->base = static_cast<const Uint8*>(mem);
  rw->here = rw->base;
  rw->stop = rw->base + size;
  return rw;
}

const char* SDL_GetKeyName(SDL_Keycode) { return ""; }

int scp_rwops_close(SDL_RWops* rw) {
  if (!rw) return -1;
  if (rw->kind == 0 && rw->handle) {
    std::fclose(static_cast<FILE*>(rw->handle));
    rw->handle = nullptr;
  }
  delete rw;
  return 0;
}

int scp_rwops_read_all(SDL_RWops* rw, void** out_buf, std::size_t* out_size) {
  if (!rw || !out_buf || !out_size) return -1;
  *out_buf = nullptr;
  *out_size = 0;
  if (rw->kind == 1) {
    const std::size_t n = static_cast<std::size_t>(rw->stop - rw->base);
    void* buf = std::malloc(n);
    if (!buf) return -1;
    std::memcpy(buf, rw->base, n);
    *out_buf = buf;
    *out_size = n;
    return 0;
  }
  if (rw->kind == 0 && rw->handle) {
    FILE* fp = static_cast<FILE*>(rw->handle);
    if (std::fseek(fp, 0, SEEK_END) != 0) return -1;
    long sz = std::ftell(fp);
    if (sz < 0) return -1;
    if (std::fseek(fp, 0, SEEK_SET) != 0) return -1;
    void* buf = std::malloc(static_cast<std::size_t>(sz));
    if (!buf) return -1;
    if (sz > 0 &&
        std::fread(buf, 1, static_cast<std::size_t>(sz), fp) !=
            static_cast<std::size_t>(sz)) {
      std::free(buf);
      return -1;
    }
    *out_buf = buf;
    *out_size = static_cast<std::size_t>(sz);
    return 0;
  }
  return -1;
}

// Mix_* (SFX) live in audio.cpp; Mix_* (music) live in music.cpp.

}  // extern "C"
