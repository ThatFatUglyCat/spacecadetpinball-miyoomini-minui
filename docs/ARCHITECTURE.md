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
  → game_step(fixed_dt)          // pb::frame
  → vscreen ColorRgba → RGB565   // centered into 640x480
  → video_cb
  → mix SFX → audio_batch_cb     // Phase 5
```

Upstream `render::vscreen` is typically 600×416 (resolution index 0). The core
letterboxes it into the MinUI-native 640×480 RGB565 buffer.

## Upstream pin

See `docs/PHASE0_FINDINGS.md` for the tagged commit and Gate 1 decision
(Option A: submodule + shim).

## Legal boundary

The DAT and sibling files are read only at runtime from `retro_game_info.path`.
The build never opens proprietary assets. See `docs/LEGAL.md`.
