// SPDX-License-Identifier: MIT
#include "music.h"

#include "SDL_mixer.h"

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
  // Without SCP_ENABLE_MUSIC (default CI build): silent stubs.
  expect(!scp::music_sf2_ready(), "no sf2 by default");
  scp::music_set_system_dir("/nonexistent");
  expect(!scp::music_sf2_ready(), "missing dir still silent");

  std::vector<int16_t> buf(64, 100);
  scp::music_mix_into(buf.data(), 32);
  expect(buf[0] == 100 && buf[1] == 100, "mix_into no-op without music");

  expect(Mix_LoadMUS("nope.mid") == nullptr, "LoadMUS stub/fail");
  expect(Mix_PlayMusic(nullptr, -1) == -1, "PlayMusic fail");
  expect(Mix_PlayingMusic() == 0, "not playing");

  scp::music_set_volume(64);
  expect(scp::music_get_volume() == 64, "volume set");
  expect(Mix_VolumeMusic(-1) == 64, "VolumeMusic query");

  if (failures) {
    std::fprintf(stderr, "%d failure(s)\n", failures);
    return 1;
  }
  std::puts("music_unit ok");
  return 0;
}
