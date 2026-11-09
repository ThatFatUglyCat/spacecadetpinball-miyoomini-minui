// SPDX-License-Identifier: MIT
#include "audio.h"

#include "SDL_mixer.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

static int failures = 0;

static void expect(bool cond, const char* msg) {
  if (!cond) {
    std::fprintf(stderr, "FAIL: %s\n", msg);
    ++failures;
  }
}

static Mix_Chunk* make_tone(int16_t sample, std::size_t frames) {
  Mix_Chunk* chunk = new Mix_Chunk();
  chunk->allocated = 1;
  chunk->volume = MIX_MAX_VOLUME;
  chunk->alen = static_cast<Uint32>(frames * sizeof(int16_t));
  chunk->abuf = static_cast<Uint8*>(std::malloc(chunk->alen));
  auto* s = reinterpret_cast<int16_t*>(chunk->abuf);
  for (std::size_t i = 0; i < frames; ++i) s[i] = sample;
  return chunk;
}

int main() {
  // Fractional remainder: 22050/60 accumulates (367 or 368).
  {
    unsigned rem = 0;
    int a = scp::audio_peek_frames_for_next(22050, 60, rem);
    int b = scp::audio_peek_frames_for_next(22050, 60, rem);
    expect(a == 367 || a == 368, "22050 first");
    expect(b == 367 || b == 368, "22050 second");
    expect(a + b == 735, "22050 two-frame sum");
  }

  // Exact 44100/60 = 735 with zero remainder.
  {
    unsigned rem = 0;
    int n = scp::audio_peek_frames_for_next(44100, 60, rem);
    expect(n == 735 && rem == 0, "44100 exact");
  }

  // Mixer saturation: two full-scale channels clamp to int16 max.
  {
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
    Mix_AllocateChannels(2);
    Mix_Volume(-1, MIX_MAX_VOLUME);

    Mix_Chunk* a = make_tone(20000, 800);
    Mix_Chunk* b = make_tone(20000, 800);
    expect(Mix_PlayChannel(0, a, 0) == 0, "play a");
    expect(Mix_PlayChannel(1, b, 0) == 1, "play b");

    int16_t out[scp::kAudioMaxFramesPerVideoFrame * 2];
    scp::audio_reset_accum();
    const std::size_t frames =
        scp::audio_mix_frame(out, scp::kAudioMaxFramesPerVideoFrame);
    expect(frames == 735, "mix frames");
    expect(out[0] == 32767 && out[1] == 32767, "saturate high");

    Mix_HaltChannel(-1);
    Mix_FreeChunk(a);
    Mix_FreeChunk(b);
    Mix_CloseAudio();
  }

  // Silence when nothing is playing.
  {
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
    int16_t out[scp::kAudioMaxFramesPerVideoFrame * 2];
    std::memset(out, 0x7f, sizeof(out));
    scp::audio_reset_accum();
    const std::size_t frames =
        scp::audio_mix_frame(out, scp::kAudioMaxFramesPerVideoFrame);
    expect(frames == 735, "silent frames");
    expect(out[0] == 0 && out[1] == 0, "silent samples");
    Mix_CloseAudio();
  }

  if (failures) {
    std::fprintf(stderr, "%d failure(s)\n", failures);
    return 1;
  }
  std::puts("audio_unit ok");
  return 0;
}
