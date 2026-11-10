# SpaceCadetPinball libretro for MinUI (Miyoo Mini)

Unofficial [libretro](https://docs.libretro.com) core that runs the
[SpaceCadetPinball](https://github.com/k4zmu2a/SpaceCadetPinball) decompilation
under [MinUI](https://github.com/shauninman/MinUI) on the Miyoo Mini / Mini Plus.

**This project does not distribute any game data.** Provide your own legitimately
obtained `PINBALL.DAT`. Not affiliated with Microsoft, Maxis, Cinematronics, or
Shaun Inman.

## Status

**v1.0.0** — playable MinUI pak: RGB565 video, OnionOS-compatible controls, SFX,
SAVE_RAM high scores, optional MIDI (`docs/MUSIC.md`), S1 MVP save states
(`docs/PHASE8_SPIKE.md`).

## Players

Download **`PINBALL.pak-miyoomini-*.zip`** from Releases for the handheld
(`PINBALL.pak-host-*.zip` is desktop-only). See
**[docs/USER_GUIDE.md](docs/USER_GUIDE.md)** (EN/PT) for SD layout, controls,
saves, and troubleshooting.

## Developers

```bash
git clone --recurse-submodules <repo-url>
cd spacecadetpinball-miyoomini-minui
cmake -S . -B build/host -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSCP_BUILD_TESTS=ON
cmake --build build/host -j
ctest --test-dir build/host --output-on-failure
./tools/check-no-proprietary.sh
./tools/package.sh build/host
```

More: [docs/BUILDING.md](docs/BUILDING.md) · [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) ·
[CONTRIBUTING.md](CONTRIBUTING.md)

## Controls

Shoulders = flippers, A/B = plunger, D-pad = nudges / high scores, Start = pause,
Select = new game. Details: [docs/INPUT_MAPPING.md](docs/INPUT_MAPPING.md).

## License

MIT for new code — [LICENSE](LICENSE), [NOTICE.md](NOTICE.md),
[docs/LEGAL.md](docs/LEGAL.md), [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md).
Upstream SpaceCadetPinball is MIT. Game assets remain proprietary and are never
included here.
