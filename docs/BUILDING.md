# Building

## Prerequisites

- CMake ≥ 3.16, a C++11 compiler (GCC/Clang)
- Git with submodules
- For Miyoo Mini: Docker +
  [union-miyoomini-toolchain](https://github.com/shauninman/union-miyoomini-toolchain)

Optional: portable CMake under `/.tools/` (gitignored) if the host package is missing.

## Host (development)

```bash
git clone --recurse-submodules <this-repo>
cd spacecadetpinball-miyoomini-minui
cmake -S . -B build/host -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSCP_BUILD_TESTS=ON
cmake --build build/host -j
ctest --test-dir build/host --output-on-failure
./tools/check-no-proprietary.sh
./tools/package.sh build/host
```

Optional MIDI support (larger binary; needs user `.sf2` at runtime):

```bash
cmake -S . -B build/host -DSCP_ENABLE_MUSIC=ON -DSCP_BUILD_TESTS=ON
cmake --build build/host -j && ctest --test-dir build/host --output-on-failure
```

Run under RetroArch only with **your** DAT **outside** the tree:

```bash
retroarch -L build/host/spacecadetpinball_libretro.so "$HOME/games/pinball/PINBALL.DAT"
```

## Cross-compile (Miyoo Mini)

```bash
# inside the toolchain container (`make shell`), repo under ~/workspace
cmake -S . -B build/miyoomini \
  -DCMAKE_TOOLCHAIN_FILE=cmake/miyoomini.toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release
# optional: -DSCP_ENABLE_MUSIC=ON
cmake --build build/miyoomini -j
./tools/package.sh build/miyoomini
```

Output: `dist/PINBALL.pak.zip` + `dist/SHA256SUMS` (whitelist only — never zip the tree).

## Release checklist

1. `CHANGELOG.md` updated; version in `retro_get_system_info` matches.
2. `./tools/check-no-proprietary.sh` and host `ctest` green.
3. Package host and/or miyoomini builds; confirm zip contains only
   `launch.sh`, `default.cfg`, `README.txt`, `spacecadetpinball_libretro.so`.
4. Tag `vX.Y.Z` and attach zip + SHA256SUMS + release notes (see `.github/workflows/release.yml`).

## Guards

```bash
./tools/check-no-proprietary.sh
./tools/sdl-inventory.sh   # inventory only; does not fail the build
```

## Performance notes

Device frame-time targets are in the implementation plan (§11). Host CI does not
profile the A7; use `scp_show_fps` on device and record results before claiming
60 fps budgets. See [PERFORMANCE.md](PERFORMANCE.md).
