# Third-party licenses

| Component | Path | License |
|---|---|---|
| SpaceCadetPinball | `third_party/SpaceCadetPinball` | MIT — see that tree’s `LICENSE` |
| libretro.h | `third_party/libretro/libretro.h` | MIT (header copyright notices) |
| TinySoundFont (`tsf.h`) | `third_party/tinysoundfont/` | MIT — see `third_party/tinysoundfont/LICENSE` |
| TinyMidiLoader (`tml.h`) | `third_party/tinysoundfont/` | Zlib — see notice inside `tml.h` |

New code in `src/`, `tools/`, and `pak/` is MIT — see root `LICENSE`.

Optional music (`SCP_ENABLE_MUSIC`) uses TinySoundFont + TinyMidiLoader. SoundFont
(`.sf2`) and game MIDI files are **not** included; provide your own (see `docs/MUSIC.md`).
