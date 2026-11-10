# Legal

This document is informational and is **not legal advice**. For a public
distribution, consider a brief review by counsel familiar with FOSS and
decompilation.

## What we distribute

- Original **MIT**-licensed code for the libretro adapter, shims, tests, and tools.
- A git submodule pin to MIT-licensed [SpaceCadetPinball](https://github.com/k4zmu2a/SpaceCadetPinball).
- Optional vendored TinySoundFont (MIT) / TinyMidiLoader (Zlib) headers — see
  [THIRD_PARTY_LICENSES.md](../THIRD_PARTY_LICENSES.md).
- Documentation and **synthetic** tests (no game bytes).

## What we never distribute

- `PINBALL.DAT`, `CADET.DAT`, MIDI, WAV, sprites, fonts extracted from the game,
  gameplay screenshots, or any derivative of those assets.
- SoundFonts redistributed by us (user-supplied `.sf2` only).
- Links to abandonware / illegal download sites.

## Policy checklist (section 3 of the plan)

1. No original assets in the repo, releases, Docker images, CI caches, or fixtures.
2. No asset derivatives (extracted palettes, WAV snippets from the DAT, etc.).
3. The **build never reads the DAT**; the core reads it only at runtime from
   `retro_game_info.path`.
4. Automated tests never require a DAT (`SCP_DAT_PATH` is local/opt-in only).
5. No gameplay screenshots in README/releases.
6. No links to illegal asset downloads; users use their own legitimate copy.
7. Attribution: upstream authors + original game creators (NOTICE / README).
8. No game logos/art; descriptive naming + “unofficial / not affiliated” notice.
9. PRs containing assets or incompatible copies are rejected.

Automated guards: `tools/check-no-proprietary.sh`, whitelist `tools/package.sh`,
CI, and the PR template.

## Upstream residual risk

SpaceCadetPinball is a decompilation published under MIT by its author. That
license does not by itself clear third-party rights in the original binary or
assets. Publishing a derivative carries the **same residual risk** as upstream.

## Removal requests

Contact the repository maintainers. Assets are never hosted here; code-only
takedown requests will be reviewed promptly.
