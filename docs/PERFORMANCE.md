# Performance

Targets from the implementation plan (§11), for Miyoo Mini (Cortex-A7) at 60 fps:

| Stage | Budget |
|---|---|
| Full frame (physics + present + audio) average | ≤ 10 ms |
| Full frame p99 | ≤ 16 ms |
| RGB565 present | ≤ ~1.5–2 ms (prefer dirty rects if over) |
| SFX mix | ≤ 0.5 ms |
| Optional MIDI (`SCP_ENABLE_MUSIC`) | ≤ 3 ms |
| Resident memory | measure on device; keep well under available RAM |

## How to measure

- Enable core option **Show FPS overlay** (`scp_show_fps`).
- On device, note MinUI CPU mode while testing and record it with the results.
- Host: `perf` / callgrind for hotspots (not a substitute for device timing).

## Known trade-offs

- Full-frame vscreen → RGB565 letterbox (Phase 3); dirty rectangles deferred.
- Music softsynth capped at 24 voices (`kMusicMaxVoices`).
- Save-state MVP clears mid-flight timers on load (cheaper / safer than full timer graph).

Device acceptance for v1.0.0 is “playable 60 fps with SFX”; formal p99 tables should
be filled after on-device profiling and kept in release notes if measured.
