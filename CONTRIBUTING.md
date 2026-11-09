# Contributing

- Do **not** commit game assets or derivatives (`PINBALL.DAT`, MIDI, WAV, sprites,
  gameplay screenshots). PRs that include them are rejected.
- New code: C++11, `SPDX-License-Identifier: MIT`, comments in English.
- Do not edit `third_party/SpaceCadetPinball` in place; add `patches/NNNN-*.patch`.
- Before opening a PR:
  - `./tools/check-no-proprietary.sh`
  - host build + `ctest`
- Prefer small, focused patches. Describe the *why* in the PR body.

Checklist for the PR template is under `.github/PULL_REQUEST_TEMPLATE.md`.
