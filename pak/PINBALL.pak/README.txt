Space Cadet Pinball — MinUI pak (Miyoo Mini / Mini Plus)
=======================================================

Unofficial libretro core. Not affiliated with Microsoft, Maxis, or Cinematronics.
This pak does NOT include any game data.

Install
-------
1. Copy PINBALL.pak to Emus/miyoomini/ on your MinUI SD card.
2. Create folder:  Roms/Space Cadet Pinball (PINBALL)/
3. Copy your legitimate PINBALL.DAT into that folder, plus the WAV sound files
   that ship with the game (same directory as the DAT). Optional music:
   PINBALL.MID beside the DAT, and a SoundFont at Bios/.../PINBALL/pinball.sf2
   (core must be built with SCP_ENABLE_MUSIC=ON; see docs/MUSIC.md).
4. Refresh ROMs in MinUI and launch PINBALL.DAT.
5. High scores persist via MinUI/libretro SAVE_RAM (.srm) automatically.

Controls (defaults)
-------------------
L1/L2  Left flipper     R1/R2  Right flipper
A/B    Plunger (hold/release)
Up     Nudge bottom     Left/Right  Nudge sides
Down   High scores
X      Toggle sound     Y  Toggle music (if built + SF2)
Start  Pause            Select  New game

Logs
----
/.userdata/miyoomini/logs/PINBALL.txt

PT-BR
-----
Copie o PINBALL.DAT da sua cópia legítima do jogo para
Roms/Space Cadet Pinball (PINBALL)/. Nenhum arquivo do jogo é distribuído
neste pak.
