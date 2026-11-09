# Changelog

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
