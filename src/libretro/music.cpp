// SPDX-License-Identifier: MIT
#include "music.h"

#include "SDL.h"
#include "SDL_mixer.h"
#include "audio.h"
#include "log.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

extern "C" {
int scp_rwops_read_all(SDL_RWops* rw, void** out_buf, std::size_t* out_size);
int scp_rwops_close(SDL_RWops* rw);
}

#ifdef SCP_ENABLE_MUSIC
#define TSF_IMPLEMENTATION
#include "tsf.h"
#define TML_IMPLEMENTATION
#include "tml.h"

struct Mix_Music {
  tml_message* first;
};

namespace {
tsf* g_sf = nullptr;
Mix_Music* g_playing = nullptr;
tml_message* g_cursor = nullptr;
double g_msec = 0.0;
bool g_looping = false;
bool g_paused = false;
}  // namespace
#endif

namespace {
int g_music_volume = MIX_MAX_VOLUME;
std::string g_system_dir;

std::string join_path(const std::string& a, const std::string& b) {
  if (a.empty()) return b;
  if (a.back() == '/' || a.back() == '\\') return a + b;
  return a + "/" + b;
}
}  // namespace

namespace scp {

void music_set_system_dir(const char* system_dir) {
  g_system_dir = system_dir ? system_dir : "";
#ifdef SCP_ENABLE_MUSIC
  if (g_sf) {
    tsf_close(g_sf);
    g_sf = nullptr;
  }
  if (g_system_dir.empty()) return;

  const char* names[] = {
      "PINBALL/pinball.sf2",
      "PINBALL/default.sf2",
      "pinball.sf2",
      "default.sf2",
  };
  for (const char* n : names) {
    const std::string path = join_path(g_system_dir, n);
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) continue;
    std::fclose(f);
    tsf* sf = tsf_load_filename(path.c_str());
    if (!sf) {
      log(RETRO_LOG_WARN, "music: failed to load SF2 %s\n", path.c_str());
      continue;
    }
    tsf_set_output(sf, TSF_STEREO_INTERLEAVED, kAudioSampleRate, 0.0f);
    tsf_set_max_voices(sf, kMusicMaxVoices);
    g_sf = sf;
    log(RETRO_LOG_INFO, "music: loaded SF2 %s (max voices %d)\n", path.c_str(),
        kMusicMaxVoices);
    return;
  }
  log(RETRO_LOG_INFO,
      "music: no SF2 under %s (want PINBALL/pinball.sf2); MIDI silent\n",
      g_system_dir.c_str());
#else
  (void)system_dir;
#endif
}

bool music_sf2_ready() {
#ifdef SCP_ENABLE_MUSIC
  return g_sf != nullptr;
#else
  return false;
#endif
}

void music_set_volume(int mix_volume_0_128) {
  if (mix_volume_0_128 < 0) mix_volume_0_128 = 0;
  if (mix_volume_0_128 > MIX_MAX_VOLUME) mix_volume_0_128 = MIX_MAX_VOLUME;
  g_music_volume = mix_volume_0_128;
}

int music_get_volume() { return g_music_volume; }

void music_mix_into(int16_t* dst_interleaved, std::size_t frames) {
#ifdef SCP_ENABLE_MUSIC
  if (!dst_interleaved || frames == 0) return;
  if (!g_sf || !g_playing || !g_playing->first || g_paused) return;

  const double frame_ms =
      (1000.0 * static_cast<double>(frames)) / static_cast<double>(kAudioSampleRate);
  const double until = g_msec + frame_ms;
  while (g_cursor && g_cursor->time <= until) {
    const tml_message* msg = g_cursor;
    switch (msg->type) {
      case TML_PROGRAM_CHANGE:
        tsf_channel_set_presetnumber(g_sf, msg->channel, msg->program,
                                     (msg->channel == 9));
        break;
      case TML_NOTE_ON:
        tsf_channel_note_on(g_sf, msg->channel, msg->key,
                            msg->velocity / 127.0f);
        break;
      case TML_NOTE_OFF:
        tsf_channel_note_off(g_sf, msg->channel, msg->key);
        break;
      case TML_PITCH_BEND:
        tsf_channel_set_pitchwheel(g_sf, msg->channel, msg->pitch_bend);
        break;
      case TML_CONTROL_CHANGE:
        tsf_channel_midi_control(g_sf, msg->channel, msg->control,
                                 msg->control_value);
        break;
      default:
        break;
    }
    g_cursor = g_cursor->next;
  }
  g_msec = until;

  if (!g_cursor) {
    if (g_looping) {
      tsf_note_off_all(g_sf);
      g_cursor = g_playing->first;
      g_msec = 0.0;
    } else {
      g_playing = nullptr;
      return;
    }
  }

  short tmp[kAudioMaxFramesPerVideoFrame * 2];
  if (frames > static_cast<std::size_t>(kAudioMaxFramesPerVideoFrame))
    frames = static_cast<std::size_t>(kAudioMaxFramesPerVideoFrame);
  std::memset(tmp, 0, frames * 2 * sizeof(short));
  tsf_render_short(g_sf, tmp, static_cast<int>(frames), 0);

  for (std::size_t i = 0; i < frames * 2; ++i) {
    int32_t s = (static_cast<int32_t>(tmp[i]) * g_music_volume) / MIX_MAX_VOLUME;
    int32_t mixed = static_cast<int32_t>(dst_interleaved[i]) + s;
    if (mixed > 32767) mixed = 32767;
    if (mixed < -32768) mixed = -32768;
    dst_interleaved[i] = static_cast<int16_t>(mixed);
  }
#else
  (void)dst_interleaved;
  (void)frames;
#endif
}

}  // namespace scp

extern "C" {

#ifdef SCP_ENABLE_MUSIC

Mix_Music* Mix_LoadMUS(const char* file) {
  if (!file) return nullptr;
  FILE* f = std::fopen(file, "rb");
  if (!f) return nullptr;
  if (std::fseek(f, 0, SEEK_END) != 0) {
    std::fclose(f);
    return nullptr;
  }
  long sz = std::ftell(f);
  if (sz <= 0) {
    std::fclose(f);
    return nullptr;
  }
  std::vector<uint8_t> buf(static_cast<std::size_t>(sz));
  std::rewind(f);
  if (std::fread(buf.data(), 1, buf.size(), f) != buf.size()) {
    std::fclose(f);
    return nullptr;
  }
  std::fclose(f);
  tml_message* msg = tml_load_memory(buf.data(), static_cast<int>(buf.size()));
  if (!msg) return nullptr;
  Mix_Music* m = new Mix_Music();
  m->first = msg;
  return m;
}

Mix_Music* Mix_LoadMUS_RW(SDL_RWops* src, int freesrc) {
  void* raw = nullptr;
  std::size_t sz = 0;
  if (scp_rwops_read_all(src, &raw, &sz) != 0 || !raw || sz == 0) {
    if (freesrc && src) scp_rwops_close(src);
    return nullptr;
  }
  tml_message* msg = tml_load_memory(raw, static_cast<int>(sz));
  std::free(raw);
  if (freesrc && src) scp_rwops_close(src);
  if (!msg) return nullptr;
  Mix_Music* m = new Mix_Music();
  m->first = msg;
  return m;
}

void Mix_FreeMusic(Mix_Music* music) {
  if (!music) return;
  if (g_playing == music) {
    g_playing = nullptr;
    g_cursor = nullptr;
  }
  if (music->first) tml_free(music->first);
  delete music;
}

int Mix_PlayMusic(Mix_Music* music, int loops) {
  if (!music || !music->first) return -1;
  if (!scp::music_sf2_ready())
    scp::log(RETRO_LOG_WARN, "music: play without SF2 — silent\n");
  if (g_sf) {
    tsf_note_off_all(g_sf);
    tsf_channel_set_bank_preset(g_sf, 0, 0, 0);
  }
  g_playing = music;
  g_cursor = music->first;
  g_msec = 0.0;
  g_looping = (loops < 0);
  g_paused = false;
  return 0;
}

int Mix_HaltMusic(void) {
  if (g_sf) tsf_note_off_all(g_sf);
  g_playing = nullptr;
  g_cursor = nullptr;
  g_msec = 0.0;
  g_paused = false;
  return 0;
}

int Mix_PausedMusic(void) { return g_paused ? 1 : 0; }
void Mix_PauseMusic(void) { g_paused = true; }
void Mix_ResumeMusic(void) { g_paused = false; }
int Mix_PlayingMusic(void) { return (g_playing != nullptr && !g_paused) ? 1 : 0; }

#else

Mix_Music* Mix_LoadMUS(const char*) { return nullptr; }
Mix_Music* Mix_LoadMUS_RW(SDL_RWops*, int) { return nullptr; }
void Mix_FreeMusic(Mix_Music*) {}
int Mix_PlayMusic(Mix_Music*, int) { return -1; }
int Mix_HaltMusic(void) { return 0; }
int Mix_PausedMusic(void) { return 0; }
void Mix_PauseMusic(void) {}
void Mix_ResumeMusic(void) {}
int Mix_PlayingMusic(void) { return 0; }

#endif

int Mix_VolumeMusic(int volume) {
  if (volume < 0) return scp::music_get_volume();
  scp::music_set_volume(volume);
  return scp::music_get_volume();
}

}  // extern "C"
