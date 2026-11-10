# Third-party licenses

Reviewed for the v1.0.0 release. No `dr_wav` / `font8x8` vendoring in this tree
(WAV load and the error screen use project code / upstream paths).

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
