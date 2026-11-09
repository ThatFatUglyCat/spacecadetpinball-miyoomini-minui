# Patches

Patches are applied against `third_party/SpaceCadetPinball` at the pinned tag.

`0001-plunger-release-window.patch` is a **candidate**. Do not enable it in
`cmake/patches.cmake` until Phase 2/4 playtesting shows missed fully-charged
shots with the fixed 60 Hz step. Source: OnionOS Miyoo Mini port behaviour.
