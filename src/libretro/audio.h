// SPDX-License-Identifier: MIT
#pragma once

#include <cstddef>
#include <cstdint>

namespace scp {

constexpr int kAudioSampleRate = 44100;
constexpr int kAudioFps = 60;
constexpr int kAudioMaxChannels = 32;
// Worst case frames/frame when using fractional accumulator (+1).
constexpr int kAudioMaxFramesPerVideoFrame = (kAudioSampleRate / kAudioFps) + 1;

void audio_init(int channels);
void audio_shutdown();
bool audio_ready();

// Fixed-step mix: advances fractional remainder, writes interleaved stereo S16.
// Returns number of stereo frames written (0 .. max_frames). No heap on hot path.
std::size_t audio_mix_frame(int16_t* dst_interleaved, std::size_t max_frames);

// Test helpers / diagnostics.
void audio_reset_accum();
int audio_peek_frames_for_next(int sample_rate, int fps, unsigned& remainder);

}  // namespace scp
