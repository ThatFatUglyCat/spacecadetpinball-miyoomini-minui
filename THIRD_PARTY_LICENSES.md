# Third-party licenses

Reviewed for the v1.0.0 release (updated for dual host/miyoomini release artifacts).
No `dr_wav` / `font8x8` vendoring in this tree (WAV load and the error screen use
project code / upstream paths).

| Component | Path | License |
|---|---|---|
| SpaceCadetPinball | `third_party/SpaceCadetPinball` | MIT — see that tree’s `LICENSE` |
| libretro.h | `third_party/libretro/libretro.h` | MIT (header copyright notices) |
| TinySoundFont (`tsf.h`) | `third_party/tinysoundfont/` | MIT — see `third_party/tinysoundfont/LICENSE` |
| TinyMidiLoader (`tml.h`) | `third_party/tinysoundfont/` | Zlib — see notice inside `tml.h` |

New code in `src/`, `tools/`, and `pak/` is MIT — see root `LICENSE`.
Every file under `src/` carries `SPDX-License-Identifier: MIT`.

Optional music (`SCP_ENABLE_MUSIC`) uses TinySoundFont + TinyMidiLoader. SoundFont
(`.sf2`) and game MIDI files are **not** included; provide your own (see `docs/MUSIC.md`).

Game assets (`PINBALL.DAT`, WAV, MIDI, sprites) are proprietary and never shipped —
see `docs/LEGAL.md`.

## Miyoo Mini release binary (runtime link)

The `PINBALL.pak-miyoomini-*.zip` core is linked with `-static-libstdc++ -static-libgcc`
([`cmake/miyoomini.toolchain.cmake`](cmake/miyoomini.toolchain.cmake)). Static
copies of libstdc++ / libgcc in that `.so` are covered by the
[GCC Runtime Library Exception](https://www.gnu.org/licenses/gcc-exception-3.1.html)
to GPLv3: the exception allows linking those runtime libraries into independent
works without forcing the whole binary under GPL.

What a release **does not** redistribute:

- The Docker image or tarball of
  [union-miyoomini-toolchain](https://github.com/shauninman/union-miyoomini-toolchain)
  / `miyoomini-toolchain-buildroot` (build-only tools; pinned by commit in CI).
- Game data, SoundFonts, or MIDI.
