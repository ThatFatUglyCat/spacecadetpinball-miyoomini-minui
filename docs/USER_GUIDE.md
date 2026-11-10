# User guide

Unofficial Space Cadet Pinball libretro core for **MinUI** on Miyoo Mini / Mini Plus.
This project does **not** ship game data.

## English

### Install

1. Download **`PINBALL.pak-miyoomini-*.zip`** from a
   [release](https://github.com/ThatFatUglyCat/spacecadetpinball-miyoomini-minui/releases)
   and verify `SHA256SUMS` if provided.
   Do **not** install `PINBALL.pak-host-*.zip` on the handheld — that build is
   x86_64 for desktop RetroArch and fails with `wrong ELF class: ELFCLASS64`.
2. Unzip so you have `PINBALL.pak/` with `launch.sh`, `default.cfg`, `README.txt`,
   and `spacecadetpinball_libretro.so`.
3. Copy that folder to `Emus/miyoomini/` on the MinUI SD card.
4. Create `Roms/Space Cadet Pinball (PINBALL)/` (the name in parentheses must match
   the pak tag `PINBALL`).
5. Copy **your** legitimate `PINBALL.DAT` into that ROM folder, plus the WAV sound
   files that belong next to the DAT in the original game install.
6. Refresh ROMs in MinUI and launch `PINBALL.DAT`.

### Optional music

Requires a core built with `-DSCP_ENABLE_MUSIC=ON`, `PINBALL.MID` beside the DAT,
and a SoundFont at the libretro system path, typically:

`Bios/.../PINBALL/pinball.sf2`

See [MUSIC.md](MUSIC.md). Enable **Background Music** in core options (or press **Y**).

### Controls

| Action | Buttons |
|---|---|
| Left / right flipper | L1+L2 / R1+R2 |
| Plunger | A or B (hold to pull, release to launch) |
| Nudge | D-pad Left / Right / Up |
| High scores overlay | D-pad Down |
| Toggle sound / music | X / Y |
| Pause / new game | Start / Select |

Full map: [INPUT_MAPPING.md](INPUT_MAPPING.md).

### Saves

- High scores: libretro SAVE_RAM (MinUI `.srm`).
- Quicksave / auto-resume: MinUI state slots via `retro_serialize*` (S1 MVP;
  mid-flight timers are cleared on load — see [PHASE8_SPIKE.md](PHASE8_SPIKE.md)).

### Troubleshooting

- Error screen / “DAT not found”: check the ROM path and that the file starts with
  the original DAT header (do not rename random files to `.dat`).
- No sound: ensure WAVs sit next to `PINBALL.DAT`; press **X** if sound was toggled off.
- Returns to the game list immediately / log shows `wrong ELF class: ELFCLASS64`:
  you installed the **host** zip. Replace the `.so` with the **miyoomini** release asset.
- Logs: `/.userdata/miyoomini/logs/PINBALL.txt`.

## Português

### Instalação

1. Baixe **`PINBALL.pak-miyoomini-*.zip`** do
   [release](https://github.com/ThatFatUglyCat/spacecadetpinball-miyoomini-minui/releases)
   e confira o `SHA256SUMS` se existir.
   Não use `PINBALL.pak-host-*.zip` no aparelho — esse build é x86_64 (PC) e
   falha com `wrong ELF class: ELFCLASS64`.
2. Extraia a pasta `PINBALL.pak/` e copie para `Emus/miyoomini/` no cartão MinUI.
3. Crie `Roms/Space Cadet Pinball (PINBALL)/`.
4. Coloque o **seu** `PINBALL.DAT` legítimo nessa pasta, junto com os WAVs do jogo.
5. Atualize a lista de ROMs no MinUI e abra o DAT.

Música opcional, controles e saves: veja as seções em inglês acima e
[INPUT_MAPPING.md](INPUT_MAPPING.md) / [MUSIC.md](MUSIC.md).

Log: `/.userdata/miyoomini/logs/PINBALL.txt`. Se o log mostrar `wrong ELF class`,
troque pelo zip **miyoomini** do release.
