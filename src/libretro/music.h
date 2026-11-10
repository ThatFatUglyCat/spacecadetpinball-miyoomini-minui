// SPDX-License-Identifier: MIT
#pragma once

#include <cstddef>
#include <cstdint>

struct Mix_Music;
struct SDL_RWops;

namespace scp {

// Max simultaneous SoftSynth voices (Miyoo CPU budget).
constexpr int kMusicMaxVoices = 24;

// Called once after Mix_OpenAudio. Looks for pinball.sf2 / default.sf2 under
// system_dir/PINBALL/ (and system_dir/). Silent no-op if SCP_ENABLE_MUSIC is off
// or the soundfont is missing.
void music_set_system_dir(const char* system_dir);
bool music_sf2_ready();

// Mix stereo S16 into dst (additive). No heap. Safe if music disabled/unready.
void music_mix_into(int16_t* dst_interleaved, std::size_t frames);

void music_set_volume(int mix_volume_0_128);  // MIX_MAX_VOLUME scale
int music_get_volume();

}  // namespace scp

// Mix_Music API used by upstream midi.cpp — implemented in music.cpp when
// SCP_ENABLE_MUSIC is ON; otherwise weak stubs remain in audio.cpp.
