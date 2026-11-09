# Building

## Prerequisites

- CMake ≥ 3.16, a C++11 compiler (GCC/Clang)
- For Miyoo Mini: Docker + [union-miyoomini-toolchain](https://github.com/shauninman/union-miyoomini-toolchain)

Optional: place a portable CMake under `/.tools/` (gitignored) if the host
package is unavailable.

## Host (development)

```bash
git clone --recurse-submodules <this-repo>
cd spacecadetpinball-miyoomini-minui
cmake -S . -B build/host -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSCP_BUILD_TESTS=ON
cmake --build build/host -j
ctest --test-dir build/host --output-on-failure
```

Run under RetroArch only with **your** DAT outside the tree:

```bash
retroarch -L build/host/spacecadetpinball_libretro.so "$HOME/games/pinball/PINBALL.DAT"
```

## Cross-compile (Miyoo Mini)

```bash
# inside the toolchain container (make shell), with this repo under ~/workspace
cmake -S . -B build/miyoomini \
  -DCMAKE_TOOLCHAIN_FILE=cmake/miyoomini.toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/miyoomini -j
./tools/package.sh build/miyoomini
```

## Guards

```bash
./tools/check-no-proprietary.sh
./tools/sdl-inventory.sh
```
