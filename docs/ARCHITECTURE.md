# Architecture

## Layers

1. **MinUI / minarch** — loads `spacecadetpinball_libretro.so`, presents RGB565
   frames, mixes audio, polls RetroPad buttons.
2. **`src/libretro/`** — new MIT code: lifecycle, video convert, audio mixer,
   input, options, clock, error screen.
3. **`include/shim/`** — minimal stand-ins for SDL2 / SDL_mixer / ImGui (Phase 2).
4. **`third_party/SpaceCadetPinball`** — upstream submodule at a fixed tag, plus
   `patches/*.patch`.

## Frame flow (target)

```
retro_run
  → input_poll / input_state
  → translate edges → game_handle_input
  → game_step(fixed_dt)          // pb::frame (skipped while paused)
  → vscreen ColorRgba → RGB565   // centered into 640x480
  → video_cb
  → audio_mix_frame → audio_batch_cb
```

Upstream `render::vscreen` is typically 600×416 (resolution index 0). The core
letterboxes it into the MinUI-native 640×480 RGB565 buffer.

Input: `input.cpp` ORs shoulder/face aliases, tracks hold state, and emits edges.
Pause / high-scores block gameplay holds; releases (or re-press after unpause) stay
in sync so flippers cannot stick. See `docs/INPUT_MAPPING.md`.

Audio: upstream `Sound` / `loader` still load WAV paths beside the DAT. The `Mix_*`
shim in `audio.cpp` decodes PCM to mono S16 @ 44100 Hz, plays on N channels, and
`audio_mix_frame` sums with saturation (no heap on the hot path). Optional music
(`SCP_ENABLE_MUSIC`) uses TinySoundFont + TinyMidiLoader; see `docs/MUSIC.md`.

Persistence: `persist.cpp` exposes a 192-byte `SAVE_RAM` blob (`SCPH` + version + five
name/score entries + CRC32). After `load_game`, the first `retro_run` imports into
`high_score::highscore_table`; each frame re-exports so the frontend can write `.srm`.

Save states: `state.cpp` implements S1 MVP (`SCPS` + version + build id + CRC). See
`docs/PHASE8_SPIKE.md`. Size is a fixed 48 KiB ceiling for `retro_serialize_size`.

## Upstream pin

See `docs/PHASE0_FINDINGS.md` for the tagged commit and Gate 1 decision
(Option A: submodule + shim).

## Legal boundary

The DAT and sibling files are read only at runtime from `retro_game_info.path`.
The build never opens proprietary assets. See `docs/LEGAL.md`.
