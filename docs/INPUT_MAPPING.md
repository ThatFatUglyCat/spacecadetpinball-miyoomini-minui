# Input mapping

Default controls for the Miyoo Mini / Mini Plus under MinUI match the scheme
proven by the OnionOS standalone port
[cacuracaptors/spacecadetpinball-miyoomini](https://github.com/cacuracaptors/spacecadetpinball-miyoomini).

Menu on MinUI opens `minarch`; it is never bound to a game action. Demo mode
(OnionOS: Menu+Start) is exposed later as a core option, not a button combo.

## RetroPad / physical buttons

| Action | RetroPad IDs | Physical (Miyoo) | Notes |
|---|---|---|---|
| Left Flipper | `L`, `L2` | L1, L2 | Hold / release edges |
| Right Flipper | `R`, `R2` | R1, R2 | Hold / release edges |
| Plunger | `A`, `B` | A, B | Hold to pull, release to launch |
| Nudge Left | `LEFT` | D-pad Left | |
| Nudge Right | `RIGHT` | D-pad Right | |
| Nudge Bottom | `UP` | D-pad Up | Table bottom nudge |
| High Scores | `DOWN` | D-pad Down | Overlay; pauses running game |
| Toggle Sound | `X` | X | Effects on/off |
| Toggle Music | `Y` | Y | No-op until music (Phase 7) |
| Pause | `START` | Start | |
| New Game | `SELECT` | Select | |

Descriptor labels (must match `pak/PINBALL.pak/default.cfg` bind names):

- `Left Flipper`, `Right Flipper`, `Plunger`
- `Nudge Left`, `Nudge Right`, `Nudge Bottom`
- `High Scores`, `Toggle Sound`, `Toggle Music`
- `Pause`, `New Game`

## Aliases in the core

`default.cfg` binds one MinUI button per action name. Inside `input.cpp` the
core always ORs:

- `L2` with Left Flipper
- `R2` with Right Flipper
- `B` with Plunger

so both shoulders and both face buttons work like the OnionOS port without
duplicate `bind` lines.

## Edge behaviour

Flippers, plunger, and nudges use press/release edges (equivalent to upstream
`InputDown` / `InputUp`). Key repeat is ignored for gameplay. If High Scores
opens while a flipper is held, the flipper must still receive a release when
the overlay closes.

## Source of truth

- OnionOS remap: `miyooRemapKeycode()` in that port’s `winmain.cpp`
- This core: `src/libretro/input.cpp` + `SET_INPUT_DESCRIPTORS`
- MinUI binds: `pak/PINBALL.pak/default.cfg`
