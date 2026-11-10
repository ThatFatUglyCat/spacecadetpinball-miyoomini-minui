# Phase 8 spike — save-state decision

**Date:** 2026-11-09  
**Decision:** **S1 explicit serialization (MVP resume)** — reject S2.

## Why not S2 (arena snapshot)

- Libretro core is a **PIE** `.so` under ASLR; vtables and `timer` callbacks are absolute `.text` addresses.
- Upstream uses `new` + `std::vector` heavily; blind memcpy of heaps breaks internal pointers.
- OnionOS fixed-address TLSF trick does not transfer (see `PHASE0_FINDINGS.md`).

## Why S1 MVP (not full determinism yet)

Full hash-identical A/V replay needs every timer callback, mission FSM edge, and RNG seed — estimated 8–15+ days. MinUI needs **auto-resume / quicksave that continues a playable game**.

MVP restores: ball kinematics, flipper angles, plunger pullback, table scores/ball counts/tilt, per-component `ActiveFlag`/`MessageField`, light on/flash state. Timers are cleared on load (mid-flight events are dropped). Audio/MIDI/render caches are not in the blob.

## Format

See `src/libretro/state.h`: magic `SCPS`, `STATE_VERSION`, build tag, little-endian payload, CRC32 trailer. Fixed `kStateMaxSize` for `retro_serialize_size`.

## Follow-ups (post-MVP)

- Timer pool with callback enum + component index
- Kickout/hole capture state
- Determinism test with `SCP_DAT_PATH` (local only)
