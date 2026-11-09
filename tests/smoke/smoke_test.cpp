// SPDX-License-Identifier: MIT
#include "libretro.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <string>

using RetroApiVersionFn = unsigned (*)(void);
using RetroGetSystemInfoFn = void (*)(struct retro_system_info*);
using RetroSetEnvironmentFn = void (*)(retro_environment_t);
using RetroInitFn = void (*)(void);
using RetroDeinitFn = void (*)(void);
using RetroLoadGameFn = bool (*)(const struct retro_game_info*);
using RetroUnloadGameFn = void (*)(void);
using RetroRunFn = void (*)(void);

static bool env_stub(unsigned cmd, void* data) {
  if (cmd == RETRO_ENVIRONMENT_SET_PIXEL_FORMAT) {
    auto* fmt = static_cast<enum retro_pixel_format*>(data);
    return fmt && *fmt == RETRO_PIXEL_FORMAT_RGB565;
  }
  if (cmd == RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME) return true;
  if (cmd == RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS) return true;
  if (cmd == RETRO_ENVIRONMENT_SET_VARIABLES) return true;
  return false;
}

static int fail(const char* msg) {
  std::fprintf(stderr, "SMOKE FAIL: %s\n", msg);
  return 1;
}

int main(int argc, char** argv) {
  if (argc < 2) return fail("usage: scp_smoke_test <path-to-spacecadetpinball_libretro.so>");

  void* handle = dlopen(argv[1], RTLD_NOW);
  if (!handle) {
    std::fprintf(stderr, "dlopen: %s\n", dlerror());
    return fail("dlopen");
  }

  auto api = reinterpret_cast<RetroApiVersionFn>(dlsym(handle, "retro_api_version"));
  auto get_info = reinterpret_cast<RetroGetSystemInfoFn>(dlsym(handle, "retro_get_system_info"));
  auto set_env = reinterpret_cast<RetroSetEnvironmentFn>(dlsym(handle, "retro_set_environment"));
  auto init = reinterpret_cast<RetroInitFn>(dlsym(handle, "retro_init"));
  auto deinit = reinterpret_cast<RetroDeinitFn>(dlsym(handle, "retro_deinit"));
  auto load = reinterpret_cast<RetroLoadGameFn>(dlsym(handle, "retro_load_game"));
  auto unload = reinterpret_cast<RetroUnloadGameFn>(dlsym(handle, "retro_unload_game"));
  auto run = reinterpret_cast<RetroRunFn>(dlsym(handle, "retro_run"));

  if (!api || !get_info || !set_env || !init || !deinit || !load || !unload || !run) {
    dlclose(handle);
    return fail("missing retro_* symbol");
  }

  if (api() != RETRO_API_VERSION) {
    dlclose(handle);
    return fail("unexpected RETRO_API_VERSION");
  }

  struct retro_system_info info;
  std::memset(&info, 0, sizeof(info));
  get_info(&info);
  if (!info.library_name || std::strcmp(info.library_name, "SpaceCadetPinball") != 0) {
    dlclose(handle);
    return fail("library_name");
  }
  if (!info.need_fullpath) {
    dlclose(handle);
    return fail("need_fullpath should be true");
  }

  set_env(env_stub);
  init();

  struct retro_game_info game;
  std::memset(&game, 0, sizeof(game));
  game.path = "/tmp/this-dat-does-not-exist-scp-smoke.dat";
  // Core returns true with error-screen mode for missing DAT.
  if (!load(&game)) {
    deinit();
    dlclose(handle);
    return fail("load_game should succeed in error mode for missing DAT");
  }
  run();
  unload();
  deinit();

  // Repeat load/unload to catch obvious leaks/crashes.
  init();
  if (!load(&game)) {
    deinit();
    dlclose(handle);
    return fail("second load_game");
  }
  unload();
  deinit();

  dlclose(handle);
  std::puts("SMOKE OK");
  return 0;
}
