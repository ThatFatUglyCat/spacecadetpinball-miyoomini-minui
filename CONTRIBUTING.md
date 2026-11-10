# Contributing

- Do **not** commit game assets or derivatives (`PINBALL.DAT`, MIDI, WAV, sprites,
  gameplay screenshots). PRs that include them are rejected.
- New code: C++11, `SPDX-License-Identifier: MIT`, comments in English.
- Do not edit `third_party/SpaceCadetPinball` in place; add `patches/NNNN-*.patch`.
- Default controls follow the OnionOS Miyoo Mini map (see `docs/INPUT_MAPPING.md`);
  do not reopen that map without an explicit decision recorded under `docs/`.
- Before opening a PR:
  - `./tools/check-no-proprietary.sh`
  - host build + `ctest`
  - keep `THIRD_PARTY_LICENSES.md` accurate if you vendor anything
- Prefer small, focused patches. Describe the *why* in the PR body.
- Releases: tag `vX.Y.Z` triggers `.github/workflows/release.yml` (host pak).
  Cross-build the Miyoo `.so` with the Docker toolchain before attaching a
  device pak (`docs/BUILDING.md`).

Checklist for the PR template is under `.github/PULL_REQUEST_TEMPLATE.md`.
