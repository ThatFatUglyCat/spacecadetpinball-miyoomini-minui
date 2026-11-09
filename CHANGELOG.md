# Changelog

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
