// SPDX-License-Identifier: MIT
#include "audio.h"

#include "SDL_mixer.h"
#include "music.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <new>
#include <vector>

namespace {

struct Channel {
  Mix_Chunk* chunk = nullptr;
  std::size_t pos = 0;
  std::size_t length = 0;
  int volume = MIX_MAX_VOLUME;
  float gain_l = 1.0f;
  float gain_r = 1.0f;
  bool active = false;
  bool paused = false;
};

bool g_open = false;
bool g_enabled = true;
int g_master_volume = MIX_MAX_VOLUME;
int g_num_channels = 8;
Channel g_channels[scp::kAudioMaxChannels];
unsigned g_frame_remainder = 0;

inline int16_t saturate_i32(int32_t v) {
  if (v > 32767) return 32767;
  if (v < -32768) return -32768;
  return static_cast<int16_t>(v);
}

inline int16_t apply_volume(int16_t sample, int chunk_vol, int chan_vol, int master) {
  int32_t v = sample;
  v = (v * chunk_vol) / MIX_MAX_VOLUME;
  v = (v * chan_vol) / MIX_MAX_VOLUME;
  v = (v * master) / MIX_MAX_VOLUME;
  return saturate_i32(v);
}

void clear_channels() {
  for (int i = 0; i < scp::kAudioMaxChannels; ++i) {
    g_channels[i] = Channel{};
    g_channels[i].volume = g_master_volume;
  }
}

void halt_channel(int ch) {
  if (ch < 0 || ch >= g_num_channels) return;
  g_channels[ch].active = false;
  g_channels[ch].chunk = nullptr;
  g_channels[ch].pos = 0;
  g_channels[ch].length = 0;
  g_channels[ch].paused = false;
  g_channels[ch].gain_l = 1.0f;
  g_channels[ch].gain_r = 1.0f;
}

int first_free_channel() {
  for (int i = 0; i < g_num_channels; ++i) {
    if (!g_channels[i].active) return i;
  }
  return -1;
}

int playing_count() {
  int n = 0;
  for (int i = 0; i < g_num_channels; ++i)
    if (g_channels[i].active && !g_channels[i].paused) ++n;
  return n;
}

Mix_Chunk* load_wav_file(const char* path) {
  if (!path) return nullptr;
  FILE* f = std::fopen(path, "rb");
  if (!f) return nullptr;

  unsigned char hdr[12];
  if (std::fread(hdr, 1, 12, f) != 12 ||
      std::memcmp(hdr, "RIFF", 4) != 0 ||
      std::memcmp(hdr + 8, "WAVE", 4) != 0) {
    std::fclose(f);
    return nullptr;
  }

  uint16_t audio_format = 0;
  uint16_t channels = 0;
  uint32_t sample_rate = 0;
  uint16_t bits = 0;
  std::vector<uint8_t> data;

  while (true) {
    unsigned char chunk_hdr[8];
    if (std::fread(chunk_hdr, 1, 8, f) != 8) break;
    const uint32_t chunk_size =
        static_cast<uint32_t>(chunk_hdr[4]) |
        (static_cast<uint32_t>(chunk_hdr[5]) << 8) |
        (static_cast<uint32_t>(chunk_hdr[6]) << 16) |
        (static_cast<uint32_t>(chunk_hdr[7]) << 24);

    if (std::memcmp(chunk_hdr, "fmt ", 4) == 0) {
      if (chunk_size < 16) {
        std::fclose(f);
        return nullptr;
      }
      std::vector<uint8_t> fmt(chunk_size);
      if (std::fread(fmt.data(), 1, chunk_size, f) != chunk_size) {
        std::fclose(f);
        return nullptr;
      }
      audio_format = static_cast<uint16_t>(fmt[0] | (fmt[1] << 8));
      channels = static_cast<uint16_t>(fmt[2] | (fmt[3] << 8));
      sample_rate = static_cast<uint32_t>(fmt[4] | (fmt[5] << 8) |
                                         (fmt[6] << 16) | (fmt[7] << 24));
      bits = static_cast<uint16_t>(fmt[14] | (fmt[15] << 8));
      if (chunk_size & 1u) std::fseek(f, 1, SEEK_CUR);
    } else if (std::memcmp(chunk_hdr, "data", 4) == 0) {
      data.resize(chunk_size);
      if (chunk_size && std::fread(data.data(), 1, chunk_size, f) != chunk_size) {
        std::fclose(f);
        return nullptr;
      }
      if (chunk_size & 1u) std::fseek(f, 1, SEEK_CUR);
      break;
    } else {
      if (std::fseek(f, static_cast<long>(chunk_size + (chunk_size & 1u)), SEEK_CUR) != 0)
        break;
    }
  }
  std::fclose(f);

  if (audio_format != 1 || channels < 1 || sample_rate == 0 || data.empty())
    return nullptr;
  if (bits != 8 && bits != 16) return nullptr;

  const int bytes_per_sample = bits / 8;
  const int frame_bytes = bytes_per_sample * static_cast<int>(channels);
  if (frame_bytes <= 0) return nullptr;
  const std::size_t src_frames = data.size() / static_cast<std::size_t>(frame_bytes);
  if (src_frames == 0) return nullptr;

  std::vector<int16_t> mono(src_frames);
  for (std::size_t i = 0; i < src_frames; ++i) {
    const uint8_t* p = data.data() + i * static_cast<std::size_t>(frame_bytes);
    int32_t acc = 0;
    for (int c = 0; c < static_cast<int>(channels); ++c) {
      if (bits == 8) {
        acc += (static_cast<int16_t>(p[c]) - 128) << 8;
      } else {
        acc += static_cast<int16_t>(p[c * 2] | (p[c * 2 + 1] << 8));
      }
    }
    mono[i] = saturate_i32(acc / static_cast<int>(channels));
  }

  std::vector<int16_t> out;
  if (sample_rate == static_cast<uint32_t>(scp::kAudioSampleRate)) {
    out.swap(mono);
  } else {
    const double ratio =
        static_cast<double>(sample_rate) / static_cast<double>(scp::kAudioSampleRate);
    std::size_t dst_frames = static_cast<std::size_t>(
        std::ceil(static_cast<double>(src_frames) *
                  static_cast<double>(scp::kAudioSampleRate) /
                  static_cast<double>(sample_rate)));
    if (dst_frames == 0) dst_frames = 1;
    out.resize(dst_frames);
    for (std::size_t i = 0; i < dst_frames; ++i) {
      const double src_pos = static_cast<double>(i) * ratio;
      const std::size_t i0 = static_cast<std::size_t>(src_pos);
      const std::size_t i1 = std::min(i0 + 1, src_frames - 1);
      const float t = static_cast<float>(src_pos - static_cast<double>(i0));
      const float s = static_cast<float>(mono[i0]) * (1.0f - t) +
                      static_cast<float>(mono[i1]) * t;
      out[i] = saturate_i32(static_cast<int32_t>(s));
    }
  }

  Mix_Chunk* chunk = new (std::nothrow) Mix_Chunk();
  if (!chunk) return nullptr;
  chunk->allocated = 1;
  chunk->volume = MIX_MAX_VOLUME;
  chunk->alen = static_cast<Uint32>(out.size() * sizeof(int16_t));
  chunk->abuf = static_cast<Uint8*>(std::malloc(chunk->alen));
  if (!chunk->abuf) {
    delete chunk;
    return nullptr;
  }
  std::memcpy(chunk->abuf, out.data(), chunk->alen);
  return chunk;
}

}  // namespace

namespace scp {

void audio_init(int channels) {
  if (channels < 1) channels = 8;
  if (channels > kAudioMaxChannels) channels = kAudioMaxChannels;
  g_num_channels = channels;
  g_master_volume = MIX_MAX_VOLUME;
  g_enabled = true;
  g_frame_remainder = 0;
  clear_channels();
  g_open = true;
}

void audio_shutdown() {
  clear_channels();
  g_open = false;
  g_frame_remainder = 0;
}

bool audio_ready() { return g_open; }

void audio_reset_accum() { g_frame_remainder = 0; }

int audio_peek_frames_for_next(int sample_rate, int fps, unsigned& remainder) {
  if (fps <= 0 || sample_rate <= 0) return 0;
  const unsigned total = static_cast<unsigned>(sample_rate) + remainder;
  const int frames = static_cast<int>(total / static_cast<unsigned>(fps));
  remainder = total % static_cast<unsigned>(fps);
  return frames;
}

std::size_t audio_mix_frame(int16_t* dst_interleaved, std::size_t max_frames) {
  if (!dst_interleaved || max_frames == 0) return 0;

  int frames = audio_peek_frames_for_next(kAudioSampleRate, kAudioFps, g_frame_remainder);
  if (frames < 0) frames = 0;
  if (static_cast<std::size_t>(frames) > max_frames)
    frames = static_cast<int>(max_frames);

  std::memset(dst_interleaved, 0,
              static_cast<std::size_t>(frames) * 2u * sizeof(int16_t));
  if (!g_open || !g_enabled || frames == 0)
    return static_cast<std::size_t>(frames);

  for (int f = 0; f < frames; ++f) {
    int32_t mix_l = 0;
    int32_t mix_r = 0;

    for (int c = 0; c < g_num_channels; ++c) {
      Channel& ch = g_channels[c];
      if (!ch.active || ch.paused || !ch.chunk || !ch.chunk->abuf) continue;
      if (ch.pos >= ch.length) {
        halt_channel(c);
        continue;
      }

      const int16_t* samples = reinterpret_cast<const int16_t*>(ch.chunk->abuf);
      const int16_t s =
          apply_volume(samples[ch.pos], ch.chunk->volume, ch.volume, g_master_volume);
      mix_l += static_cast<int32_t>(static_cast<float>(s) * ch.gain_l);
      mix_r += static_cast<int32_t>(static_cast<float>(s) * ch.gain_r);
      ++ch.pos;
      if (ch.pos >= ch.length) halt_channel(c);
    }

    dst_interleaved[f * 2] = saturate_i32(mix_l);
    dst_interleaved[f * 2 + 1] = saturate_i32(mix_r);
  }

  // Additive music bed (no-op if SCP_ENABLE_MUSIC is off or no SF2/track).
  music_mix_into(dst_interleaved, static_cast<std::size_t>(frames));

  return static_cast<std::size_t>(frames);
}

}  // namespace scp

extern "C" {

int Mix_Init(int) { return 0; }
void Mix_Quit(void) {}

int Mix_OpenAudio(int, Uint16, int, int) {
  scp::audio_init(8);
  return 0;
}

void Mix_CloseAudio(void) { scp::audio_shutdown(); }

int Mix_AllocateChannels(int numchans) {
  if (!scp::audio_ready()) return 0;
  scp::audio_init(numchans);
  return g_num_channels;
}

int Mix_Volume(int channel, int volume) {
  if (volume < 0) {
    if (channel == -1) return g_master_volume;
    if (channel >= 0 && channel < g_num_channels) return g_channels[channel].volume;
    return 0;
  }
  if (volume > MIX_MAX_VOLUME) volume = MIX_MAX_VOLUME;
  if (!scp::audio_ready()) return volume;
  if (channel == -1) {
    g_master_volume = volume;
    for (int i = 0; i < g_num_channels; ++i)
      g_channels[i].volume = volume;
    return volume;
  }
  if (channel >= 0 && channel < g_num_channels) {
    g_channels[channel].volume = volume;
    return volume;
  }
  return 0;
}

int Mix_PlayChannel(int channel, Mix_Chunk* chunk, int) {
  if (!scp::audio_ready() || !g_enabled || !chunk || !chunk->abuf || chunk->alen < 2)
    return -1;
  if (channel == -1) {
    channel = first_free_channel();
    if (channel < 0) return -1;
  }
  if (channel < 0 || channel >= g_num_channels) return -1;

  Channel& ch = g_channels[channel];
  ch.chunk = chunk;
  ch.pos = 0;
  ch.length = chunk->alen / sizeof(int16_t);
  ch.active = true;
  ch.paused = false;
  return channel;
}

int Mix_HaltChannel(int channel) {
  if (!scp::audio_ready()) return 0;
  if (channel == -1) {
    for (int i = 0; i < g_num_channels; ++i) halt_channel(i);
    return 0;
  }
  halt_channel(channel);
  return 0;
}

int Mix_Playing(int channel) {
  if (!scp::audio_ready()) return 0;
  if (channel == -1) return playing_count();
  if (channel < 0 || channel >= g_num_channels) return 0;
  return (g_channels[channel].active && !g_channels[channel].paused) ? 1 : 0;
}

void Mix_Pause(int channel) {
  if (!scp::audio_ready()) return;
  if (channel == -1) {
    for (int i = 0; i < g_num_channels; ++i)
      if (g_channels[i].active) g_channels[i].paused = true;
    return;
  }
  if (channel >= 0 && channel < g_num_channels && g_channels[channel].active)
    g_channels[channel].paused = true;
}

void Mix_Resume(int channel) {
  if (!scp::audio_ready()) return;
  if (channel == -1) {
    for (int i = 0; i < g_num_channels; ++i)
      g_channels[i].paused = false;
    return;
  }
  if (channel >= 0 && channel < g_num_channels)
    g_channels[channel].paused = false;
}

int Mix_SetPosition(int channel, Sint16 angle, Uint8 distance) {
  if (!scp::audio_ready() || channel < 0 || channel >= g_num_channels) return 0;
  float atten = 1.0f - (static_cast<float>(distance) / 255.0f);
  if (atten < 0.0f) atten = 0.0f;
  const float rad = static_cast<float>(angle) * 3.14159265f / 180.0f;
  const float pan01 = (std::sin(rad) + 1.0f) * 0.5f;
  g_channels[channel].gain_l = atten * (1.0f - pan01);
  g_channels[channel].gain_r = atten * pan01;
  return 1;
}

Mix_Chunk* Mix_LoadWAV(const char* file) {
  if (!scp::audio_ready()) return nullptr;
  return load_wav_file(file);
}

Mix_Chunk* Mix_LoadWAV_RW(SDL_RWops*, int) { return nullptr; }

void Mix_FreeChunk(Mix_Chunk* chunk) {
  if (!chunk) return;
  for (int i = 0; i < g_num_channels; ++i) {
    if (g_channels[i].chunk == chunk) halt_channel(i);
  }
  if (chunk->allocated && chunk->abuf) std::free(chunk->abuf);
  delete chunk;
}

// Mix_Music* APIs are implemented in music.cpp.

}  // extern "C"
