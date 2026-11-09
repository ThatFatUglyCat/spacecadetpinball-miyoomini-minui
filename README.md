# SpaceCadetPinball libretro for MinUI (Miyoo Mini)

Unofficial [libretro](https://docs.libretro.com) core that runs the
[SpaceCadetPinball](https://github.com/k4zmu2a/SpaceCadetPinball) decompilation
under [MinUI](https://github.com/shauninman/MinUI) on the Miyoo Mini / Mini Plus.

**This project does not distribute any game data.** You must provide your own
legitimately obtained `PINBALL.DAT`. The project is not affiliated with
Microsoft, Maxis, Cinematronics, or Shaun Inman.

## Status

Phase 0–1 scaffold: libretro skeleton, DAT validation, error screen, MinUI pak
layout, CI guards, and OnionOS-compatible control descriptors. The game engine
is not linked yet (Phase 2).

## Quick start (developers)

```bash
git clone --recurse-submodules <repo-url>
cd spacecadetpinball-miyoomini-minui
cmake -S . -B build/host -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build/host -j
ctest --test-dir build/host --output-on-failure
./tools/package.sh build/host
```

See [docs/BUILDING.md](docs/BUILDING.md) and [docs/USER_GUIDE.md](docs/USER_GUIDE.md).

## Controls

Defaults follow the Miyoo Mini OnionOS port control scheme (shoulders = flippers,
A/B = plunger, D-pad nudges / high scores, Start = pause, Select = new game).
Details: [docs/INPUT_MAPPING.md](docs/INPUT_MAPPING.md).

## License

MIT for new code — see [LICENSE](LICENSE) and [NOTICE.md](NOTICE.md).
Upstream SpaceCadetPinball is also MIT. Game assets remain proprietary to their
owners and are never included here.
