# Patches

Patches are applied against `third_party/SpaceCadetPinball` at the pinned tag
by `cmake/patches.cmake` during configure.

| Patch | Status | Purpose |
|---|---|---|
| `0001-plunger-release-window.patch.candidate` | optional | 150ms plunger release window (rename to `*.patch` to enable) |
| `0003-libretro-highscore-flush.patch` | apply | `high_score::flush_pending()` commits queued scores without ImGui |
| `0002-libretro-pch.patch` | required | `SCP_LIBRETRO` pch path using `include/shim` |

`cmake/patches.cmake` applies every `patches/*.patch` at configure time.
