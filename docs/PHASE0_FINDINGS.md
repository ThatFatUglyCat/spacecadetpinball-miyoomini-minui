# Phase 0 findings

**Date:** 2026-11-09  
**Upstream pin:** `Release_2.1.0` (`6a30ccbef12c7b7781ccf89788d77461fa20a90a`)  
**Inventory:** see `docs/SDL_INVENTORY.txt` (from `tools/sdl-inventory.sh`)

## Existing libretro search

Repeated search (GitHub / topics `libretro-cores`):

| Candidate | Result |
|---|---|
| `danprice142/SpaceCadetPinball-Libretro` | Fork of upstream tagged with libretro topics; tree is still the SDL2 desktop game (no `retro_*`, no core build). Not a usable core. |
| `danprice142/SpaceCadetPinball-Libreto` | Same content / typo rename. |
| libretro-super / buildbot | No Space Cadet Pinball core listed. |

**Conclusion:** proceed with this MinUI-targeted core. Core / pak tag name: `spacecadetpinball` / `PINBALL`.

## Gate 1 — integration strategy

**Decision: Option A (submodule + SDL/mixer/ImGui shim + small patches).**

Evidence from inventory (Release_2.1.0):

- `SDL_*` hits (~464) concentrate in `winmain.cpp`, `imgui_impl_sdl*`, `Sound.cpp`, `midi.cpp`, `options.cpp`, `gdrv`/`render`/`zdrv`, `pb`, `pch.h`.
- `Mix_*` hits (~42) concentrate in `Sound.cpp` and `midi.cpp`.
- ImGui is large (~11k `ImGui*` hits) but lives in dedicated files that can be **omitted from the build**; game UI is not required for play.
- Game logic (`pb`, table components, physics, `partman`) has little/no SDL.

Gate 2 (no residual SDL on `retro_run` path) remains open until Phase 2 stubs the shim and replaces `winmain`’s loop with `init/step/shutdown`.

Fallback: Option B (vendored fork) only if Phase 2 proves the shim surface too large.

## DAT magic

Confirmed in `partman.cpp`:

```text
PARTOUT(4.0)RESOURCE
```

Used by `src/libretro/content.cpp` for runtime validation (not derived from asset bytes in the repo).

## minarch (MinUI) environment subset

Reviewed `workspace/all/minarch/minarch.c` from `shauninman/MinUI`:

| Feature | Supported |
|---|---|
| `SET_PIXEL_FORMAT` RGB565 | Yes (required) |
| `SET_INPUT_DESCRIPTORS` | Yes |
| `SET_VARIABLES` (legacy) | Yes |
| `SET_CORE_OPTIONS` / `GET_CORE_OPTIONS_VERSION` | Yes (v1+) |
| `GET_SYSTEM_DIRECTORY` / `GET_SAVE_DIRECTORY` | Yes |
| `GET_LOG_INTERFACE` | Yes |
| L2 / R2 button IDs and `bind` labels (`L2`, `R2`) | Yes |
| OpenGL HW render | No — software RGB565 only |

**Core options policy:** register with legacy `SET_VARIABLES` for widest minarch compatibility; descriptors and `default.cfg` bind labels must match exactly.

## Controls baseline (from OnionOS port)

Adopted from [cacuracaptors/spacecadetpinball-miyoomini](https://github.com/cacuracaptors/spacecadetpinball-miyoomini)
(`miyooRemapKeycode` + README). Full table in `docs/INPUT_MAPPING.md`.

Not ported: `/dev/input` Menu keymon, Game Switcher, save-at-power-off, MIDI conversion launcher.

## Plunger patch candidate

OnionOS replaced post-release collision window:

| | Upstream 2.1.0 | OnionOS port |
|---|---|---|
| On release | `timer::set(PullbackDelay, …)` with `PullbackDelay = 0.025f` | `timer::set(0.15f, …)` |

**Decision:** keep as candidate patch `patches/0001-plunger-release-window.patch` for Phase 2/4 playtest. With a fixed 60 Hz step the 25 ms window may be enough; apply only if fully charged shots miss on device/host.

## Performance note

OnionOS dirty-frame present cut paused CPU ~21% → ~6% of one core. Phase 3 acceptance: if full-frame 8bpp→RGB565 costs > ~2 ms on device, prefer dirty rectangles before other optimisations.

## Save states (preview for Phase 8)

OnionOS `miyoo_state` relies on a non-PIE binary + fixed-address TLSF heap. A libretro `.so` is PIE, so that snapshot approach does **not** transfer. Prefer explicit serialisation (S1); revisit relocatable arena (S2) only after a spike.

## Host baseline

Full upstream desktop build + DAT playtest deferred until a local DAT is available outside the repo (`SCP_DAT_PATH`). Phase 0 acceptance for CI/host is the empty core + inventory + findings (this document).
