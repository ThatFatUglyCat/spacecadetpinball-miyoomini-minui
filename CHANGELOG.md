# Changelog

## Unreleased

- Release / CI publish **both** device and desktop paks on tag `v*`:
  `PINBALL.pak-miyoomini-*.zip` (ARM ELF32 via pinned `union-miyoomini-toolchain`)
  and `PINBALL.pak-host-*.zip` (x86_64), with a combined `SHA256SUMS`
- `tools/check-miyoomini-elf.sh` rejects non-ARM32 cores and dynamic
  `libstdc++` / `libgcc_s` before packaging
- User guide points players at the **miyoomini** zip; host zip is for desktop only

## 1.0.0 — Phase 9 polish & release

- Documentation complete for a new player: `README`, `docs/USER_GUIDE.md` (EN/PT),
  `BUILDING`, `LEGAL`, `PERFORMANCE`, plus existing architecture/input/music docs
- `THIRD_PARTY_LICENSES.md` reviewed; SPDX MIT on all `src/` files
- Release workflow on tag `v*`: host build, whitelist pak zip, `SHA256SUMS`, notes
  (Miyoo cross-build remained manual in this tag — see later Unreleased notes)
- Core `library_version` → `1.0.0`
- On-device frame-time profiling left to maintainers with hardware
  (`docs/PERFORMANCE.md`); no further code changes required for acceptance docs

## 0.8.0 — Phase 8 save states (S1 MVP)

- Decision: explicit serialization (S1); reject arena snapshot (S2) — see `docs/PHASE8_SPIKE.md`
- `retro_serialize*` restores balls, flippers, plunger, scores, component flags, lights
- Timers cleared on load; CRC + version gate; unit tests for header rejection

## 0.7.0 — Phase 7 optional MIDI

- TinySoundFont + TinyMidiLoader vendored under `third_party/tinysoundfont/`
- CMake `SCP_ENABLE_MUSIC` (default OFF); user SF2 in `{system}/PINBALL/`
- Mix music into SFX each frame; silent fallback without SF2/MID
- Core option `scp_music`; see `docs/MUSIC.md`

## 0.6.0 — Phase 6 MVP

- High scores in `RETRO_MEMORY_SAVE_RAM` (SCPH v1 + CRC32)
- `high_score::flush_pending` patch commits scores without ImGui
- D-pad Down toggles RGB565 high-score overlay
- Core options labels polished; `retro_reset` = new game; clearer error/log text
- Internal MVP milestone (pak + DAT + controls + SFX + persistent scores)

## 0.5.0 — Phase 5 SFX audio

- Software mixer: N channels, saturated int16 stereo @ 44100 Hz
- `Mix_*` shim loads PCM WAVs (next to the DAT) and feeds the mixer
- Fixed-step frames with fractional remainder; `audio_batch_cb` each `retro_run`
- Unit tests for frame remainder and mix saturation

## 0.4.0 — Phase 4 input

- RetroPad → engine edges (flippers, plunger, nudges, pause, new game)
- OnionOS aliases (L|L2, R|R2, A|B) and same-frame release on pause
- Core options `scp_plunger_mode` (hold-release|tap) and `scp_nudge`
- Unit tests for edge translation; `retro_reset` starts a new game

## 0.3.0 — Phase 3 video

- Convert upstream vscreen `ColorRgba` → RGB565 for libretro
- Center table framebuffer in a fixed 640×480 letterbox
- Optional FPS overlay via `scp_show_fps` core option
- Unit tests for palette/RGBA conversion and present centering

## 0.2.0 — Phase 2 engine link (no SDL)

- SDL / SDL_mixer / ImGui shims under `include/shim/`
- `scp::game_init` / `game_step` / `game_shutdown` driving upstream `pb::`
- Host `.so` has no libSDL2 dependency (Gate 2)
- Patch `0002-libretro-pch.patch`; plunger window remains `.candidate`

## 0.1.0 — Phase 0/1 scaffold

- Repository guards, legal notices, upstream submodule at `Release_2.1.0`
- Phase 0 findings (Gate 1: submodule + shim; plunger patch candidate)
- Libretro core skeleton with RGB565 error screen and DAT magic validation
- OnionOS-compatible input descriptors and MinUI `default.cfg`
- Host CMake build, smoke/unit tests, whitelist `package.sh`
