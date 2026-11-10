# SpaceCadetPinball libretro for MinUI (Miyoo Mini)

Unofficial [libretro](https://docs.libretro.com) core that runs the
[SpaceCadetPinball](https://github.com/k4zmu2a/SpaceCadetPinball) decompilation
under [MinUI](https://github.com/shauninman/MinUI) on the Miyoo Mini / Mini Plus.

**This project does not distribute any game data.** You must provide your own
legitimately obtained `PINBALL.DAT`. The project is not affiliated with
Microsoft, Maxis, Cinematronics, or Shaun Inman.

## Status

**0.8.0:** RGB565 video, OnionOS input, SFX, SAVE_RAM high scores, optional MIDI
(`docs/MUSIC.md`), and S1 MVP save states for MinUI quicksave/auto-resume
(`docs/PHASE8_SPIKE.md`).

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
