// SPDX-License-Identifier: MIT
#pragma once

#include "SDL.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MIX_DEFAULT_FREQUENCY 44100
#define MIX_DEFAULT_FORMAT 0x8010
#define MIX_MAX_VOLUME 128
#define MIX_INIT_MID 0x00000008
#define MIX_INIT_FLUIDSYNTH MIX_INIT_MID
#define SDL_MIXER_MAJOR_VERSION 2
#define SDL_MIXER_MINOR_VERSION 0
#define SDL_MIXER_PATCHLEVEL 4

typedef struct Mix_Chunk {
  int allocated;
  Uint8* abuf;
  Uint32 alen;
  Uint8 volume;
} Mix_Chunk;

typedef struct Mix_Music Mix_Music;

int Mix_Init(int flags);
void Mix_Quit(void);
int Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize);
void Mix_CloseAudio(void);
int Mix_AllocateChannels(int numchans);
int Mix_Volume(int channel, int volume);
int Mix_VolumeMusic(int volume);
int Mix_PlayChannel(int channel, Mix_Chunk* chunk, int loops);
int Mix_HaltChannel(int channel);
int Mix_Playing(int channel);
void Mix_Pause(int channel);
void Mix_Resume(int channel);
int Mix_SetPosition(int channel, Sint16 angle, Uint8 distance);
Mix_Chunk* Mix_LoadWAV(const char* file);
Mix_Chunk* Mix_LoadWAV_RW(SDL_RWops* src, int freesrc);
void Mix_FreeChunk(Mix_Chunk* chunk);
Mix_Music* Mix_LoadMUS(const char* file);
Mix_Music* Mix_LoadMUS_RW(SDL_RWops* src, int freesrc);
void Mix_FreeMusic(Mix_Music* music);
int Mix_PlayMusic(Mix_Music* music, int loops);
int Mix_HaltMusic(void);
int Mix_PausedMusic(void);
void Mix_PauseMusic(void);
void Mix_ResumeMusic(void);
int Mix_PlayingMusic(void);

#ifdef __cplusplus
}
#endif
