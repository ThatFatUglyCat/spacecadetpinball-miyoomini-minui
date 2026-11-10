# Optional MIDI music (Phase 7)

Background music is **optional** and behind CMake `SCP_ENABLE_MUSIC`.

## Build

```bash
cmake -S . -B build/host -DSCP_ENABLE_MUSIC=ON
cmake --build build/host -j
```

Default builds keep `SCP_ENABLE_MUSIC=OFF` (SFX only, smaller binary).

## Runtime files (user-provided — not distributed)

| File | Location |
|---|---|
| SoundFont | `{system}/PINBALL/pinball.sf2` (also tries `default.sf2`, or `{system}/pinball.sf2`) |
| MIDI | Next to `PINBALL.DAT` as `PINBALL.MID` (upstream name; Full Tilt uses `SOUND/TABA*.MID`) |

On MinUI, `{system}` is the libretro system directory (`Bios/` style path from
`RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY`).

**Do not commit `.sf2` / `.mid` files.** Choose a SoundFont whose license you accept
and document it for your own build; this repo does not ship one.

## Core option

`scp_music` — `disabled` (default) | `enabled`

Y still toggles music in-game (same as the option when the engine is running).

## Behaviour

- Missing SF2 or MIDI → silent fallback (no crash, no error screen).
- SoftSynth voice cap: 24 (`kMusicMaxVoices`) for Miyoo CPU budget.
- Music is mixed additively with SFX each `retro_run` frame @ 44100 Hz stereo.
