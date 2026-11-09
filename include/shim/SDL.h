// SPDX-License-Identifier: MIT
// Minimal SDL2 shim for building SpaceCadetPinball as a libretro core.
#pragma once

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>

#ifdef __cplusplus
extern "C" {
#endif

typedef int8_t Sint8;
typedef uint8_t Uint8;
typedef int16_t Sint16;
typedef uint16_t Uint16;
typedef int32_t Sint32;
typedef uint32_t Uint32;
typedef int64_t Sint64;
typedef uint64_t Uint64;
typedef int SDL_bool;
#define SDL_FALSE 0
#define SDL_TRUE 1

#define SDL_VERSION_ATLEAST(X, Y, Z) 1
#define SDL_VERSIONNUM(X, Y, Z) ((X)*1000 + (Y)*100 + (Z))

#define SDL_HINT_RENDER_SCALE_QUALITY "SDL_HINT_RENDER_SCALE_QUALITY"
#define SDL_WINDOW_FULLSCREEN_DESKTOP 0x00001001
#define SDL_WINDOWPOS_UNDEFINED 0
#define SDL_MESSAGEBOX_ERROR 0x00000010
#define SDL_MESSAGEBOX_WARNING 0x00000020
#define SDL_MESSAGEBOX_INFORMATION 0x00000040
#define SDL_BUTTON_LEFT 1
#define SDL_BUTTON_MIDDLE 2
#define SDL_BUTTON_RIGHT 3
#define SDL_BUTTON_X1 4
#define SDL_BUTTON_X2 5

#define SDL_PIXELFORMAT_BGRA32 0x16362004u
#define SDL_PIXELFORMAT_ARGB8888 0x16362004u
#define SDL_TEXTUREACCESS_STATIC 0
#define SDL_TEXTUREACCESS_STREAMING 1
#define SDL_TEXTUREACCESS_TARGET 2
#define SDL_BLENDMODE_NONE 0x00000000
#define SDL_BLENDMODE_BLEND 0x00000001

#define SDL_INIT_VIDEO 0x00000020u
#define SDL_INIT_AUDIO 0x00000010u
#define SDL_INIT_GAMECONTROLLER 0x00002000u
#define SDL_INIT_TIMER 0x00000001u

typedef Sint32 SDL_Keycode;
enum {
  SDLK_UNKNOWN = 0,
  SDLK_RETURN = '\r',
  SDLK_ESCAPE = '\033',
  SDLK_BACKSPACE = '\b',
  SDLK_TAB = '\t',
  SDLK_SPACE = ' ',
  SDLK_PERIOD = '.',
  SDLK_SLASH = '/',
  SDLK_0 = '0',
  SDLK_a = 'a',
  SDLK_e = 'e',
  SDLK_g = 'g',
  SDLK_o = 'o',
  SDLK_t = 't',
  SDLK_x = 'x',
  SDLK_y = 'y',
  SDLK_z = 'z',
  SDLK_UP = 1073741906,
  SDLK_DOWN = 1073741905,
  SDLK_RIGHT = 1073741903,
  SDLK_LEFT = 1073741904,
  SDLK_LCTRL = 1073742048,
  SDLK_LSHIFT = 1073742049,
  SDLK_LALT = 1073742050,
  SDLK_RCTRL = 1073742052,
  SDLK_F1 = 1073741882,
  SDLK_F2 = 1073741883,
  SDLK_F3 = 1073741884,
  SDLK_F4 = 1073741885,
  SDLK_F5 = 1073741886,
  SDLK_F6 = 1073741887,
  SDLK_F8 = 1073741889,
  SDLK_F9 = 1073741890,
  SDLK_F10 = 1073741891,
  SDLK_F11 = 1073741892,
  SDLK_F12 = 1073741893,
};

typedef enum {
  SDL_SCANCODE_ESCAPE = 41,
  SDL_SCANCODE_RETURN = 40,
} SDL_Scancode;

typedef enum {
  SDL_CONTROLLER_BUTTON_INVALID = -1,
  SDL_CONTROLLER_BUTTON_A = 0,
  SDL_CONTROLLER_BUTTON_B = 1,
  SDL_CONTROLLER_BUTTON_X = 2,
  SDL_CONTROLLER_BUTTON_Y = 3,
  SDL_CONTROLLER_BUTTON_BACK = 4,
  SDL_CONTROLLER_BUTTON_START = 6,
  SDL_CONTROLLER_BUTTON_LEFTSHOULDER = 9,
  SDL_CONTROLLER_BUTTON_RIGHTSHOULDER = 10,
  SDL_CONTROLLER_BUTTON_DPAD_UP = 11,
  SDL_CONTROLLER_BUTTON_DPAD_DOWN = 12,
  SDL_CONTROLLER_BUTTON_DPAD_LEFT = 13,
  SDL_CONTROLLER_BUTTON_DPAD_RIGHT = 14,
  SDL_CONTROLLER_BUTTON_MAX = 15,
} SDL_GameControllerButton;

typedef enum {
  SDL_QUIT = 0x100,
  SDL_KEYDOWN = 0x300,
  SDL_KEYUP,
  SDL_TEXTEDITING,
  SDL_TEXTINPUT,
  SDL_MOUSEMOTION = 0x400,
  SDL_MOUSEBUTTONDOWN,
  SDL_MOUSEBUTTONUP,
  SDL_MOUSEWHEEL,
  SDL_CONTROLLERBUTTONDOWN = 0x653,
  SDL_CONTROLLERBUTTONUP,
  SDL_WINDOWEVENT = 0x200,
} SDL_EventType;

typedef struct SDL_Keysym {
  SDL_Scancode scancode;
  SDL_Keycode sym;
  Uint16 mod;
  Uint32 unused;
} SDL_Keysym;

typedef struct SDL_KeyboardEvent {
  Uint32 type;
  Uint32 timestamp;
  Uint32 windowID;
  Uint8 state;
  Uint8 repeat;
  Uint8 padding2;
  Uint8 padding3;
  SDL_Keysym keysym;
} SDL_KeyboardEvent;

typedef struct SDL_Event {
  Uint32 type;
  SDL_KeyboardEvent key;
  Uint8 padding[56];
} SDL_Event;

typedef struct SDL_Rect {
  int x, y;
  int w, h;
} SDL_Rect;

typedef struct SDL_FRect {
  float x, y, w, h;
} SDL_FRect;

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Surface;
typedef struct SDL_RWops SDL_RWops;

typedef struct SDL_RendererInfo {
  const char* name;
  Uint32 flags;
  Uint32 num_texture_formats;
  Uint32 texture_formats[16];
  int max_texture_width;
  int max_texture_height;
} SDL_RendererInfo;

int SDL_Init(Uint32 flags);
void SDL_Quit(void);
const char* SDL_GetError(void);
void SDL_ClearError(void);
void SDL_free(void* mem);
void* SDL_malloc(size_t size);

Uint32 SDL_GetTicks(void);
Uint64 SDL_GetPerformanceCounter(void);
Uint64 SDL_GetPerformanceFrequency(void);
void SDL_Delay(Uint32 ms);

const char* SDL_GetHint(const char* name);
SDL_bool SDL_SetHint(const char* name, const char* value);

char* SDL_GetBasePath(void);
char* SDL_GetPrefPath(const char* org, const char* app);

SDL_Window* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags);
void SDL_DestroyWindow(SDL_Window* window);
void SDL_ShowWindow(SDL_Window* window);
void SDL_SetWindowSize(SDL_Window* window, int w, int h);
int SDL_SetWindowFullscreen(SDL_Window* window, Uint32 flags);
void SDL_SetWindowGrab(SDL_Window* window, SDL_bool grabbed);
int SDL_ShowCursor(int toggle);

SDL_Renderer* SDL_CreateRenderer(SDL_Window* window, int index, Uint32 flags);
void SDL_DestroyRenderer(SDL_Renderer* renderer);
int SDL_GetRendererInfo(SDL_Renderer* renderer, SDL_RendererInfo* info);
int SDL_GetRendererOutputSize(SDL_Renderer* renderer, int* w, int* h);
int SDL_SetRenderDrawColor(SDL_Renderer* renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
int SDL_RenderClear(SDL_Renderer* renderer);
int SDL_RenderFillRect(SDL_Renderer* renderer, const SDL_Rect* rect);
int SDL_RenderCopy(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect);
int SDL_RenderCopyF(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_FRect* dstrect);
void SDL_RenderPresent(SDL_Renderer* renderer);

SDL_Texture* SDL_CreateTexture(SDL_Renderer* renderer, Uint32 format, int access, int w, int h);
void SDL_DestroyTexture(SDL_Texture* texture);
int SDL_SetTextureBlendMode(SDL_Texture* texture, int blendMode);
int SDL_UpdateTexture(SDL_Texture* texture, const SDL_Rect* rect, const void* pixels, int pitch);
int SDL_LockTexture(SDL_Texture* texture, const SDL_Rect* rect, void** pixels, int* pitch);
void SDL_UnlockTexture(SDL_Texture* texture);

int SDL_PollEvent(SDL_Event* event);
int SDL_PushEvent(SDL_Event* event);
const Uint8* SDL_GetKeyboardState(int* numkeys);

int SDL_ShowSimpleMessageBox(Uint32 flags, const char* title, const char* message, SDL_Window* window);
SDL_RWops* SDL_RWFromFile(const char* file, const char* mode);
SDL_RWops* SDL_RWFromMem(void* mem, int size);
const char* SDL_GetKeyName(SDL_Keycode key);

#ifdef __cplusplus
}
#endif

// C++ helper used by UsingSdlHint in pb.h — keep after extern "C"
#ifdef __cplusplus
#endif
