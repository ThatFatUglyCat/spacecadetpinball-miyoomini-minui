#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Inventory SDL / Mix / ImGui usage in the upstream submodule.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC="$ROOT/third_party/SpaceCadetPinball/SpaceCadetPinball"

if [[ ! -d "$SRC" ]]; then
  echo "ERROR: upstream sources missing at $SRC" >&2
  exit 1
fi

echo "# SDL / Mix / ImGui inventory"
echo "# Generated: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
echo "# Upstream: $(git -C "$ROOT/third_party/SpaceCadetPinball" describe --tags --always)"
echo
printf '%-40s %6s %6s %6s %6s\n' "file" "SDL_" "Mix_" "ImGui" "imgui"
printf '%-40s %6s %6s %6s %6s\n' "----------------------------------------" "------" "------" "------" "------"

total_sdl=0
total_mix=0
total_imgui=0
total_imgui_h=0

while IFS= read -r -d '' f; do
  rel="${f#$SRC/}"
  sdl=$(grep -Eoc 'SDL_[A-Za-z0-9_]+' "$f" 2>/dev/null || true)
  mix=$(grep -Eoc 'Mix_[A-Za-z0-9_]+' "$f" 2>/dev/null || true)
  imgui=$(grep -Eoc 'ImGui[A-Za-z0-9_]*' "$f" 2>/dev/null || true)
  imgui_h=$(grep -Eioc 'imgui' "$f" 2>/dev/null || true)
  [[ -z "$sdl" ]] && sdl=0
  [[ -z "$mix" ]] && mix=0
  [[ -z "$imgui" ]] && imgui=0
  [[ -z "$imgui_h" ]] && imgui_h=0
  if (( sdl + mix + imgui + imgui_h > 0 )); then
    printf '%-40s %6d %6d %6d %6d\n' "$rel" "$sdl" "$mix" "$imgui" "$imgui_h"
  fi
  total_sdl=$((total_sdl + sdl))
  total_mix=$((total_mix + mix))
  total_imgui=$((total_imgui + imgui))
  total_imgui_h=$((total_imgui_h + imgui_h))
done < <(find "$SRC" -type f \( -name '*.cpp' -o -name '*.h' -o -name '*.c' \) -print0 | sort -z)

echo
printf '%-40s %6d %6d %6d %6d\n' "TOTAL" "$total_sdl" "$total_mix" "$total_imgui" "$total_imgui_h"
