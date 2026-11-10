#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Fail if the core .so is not a 32-bit ARM ELF with static libstdc++/libgcc.
set -euo pipefail

SO="${1:?usage: check-miyoomini-elf.sh <spacecadetpinball_libretro.so>}"

if [[ ! -f "$SO" ]]; then
  echo "ERROR: missing core: $SO" >&2
  exit 1
fi

if ! command -v readelf >/dev/null 2>&1; then
  echo "ERROR: readelf not found (binutils)" >&2
  exit 1
fi

file_out="$(file -b "$SO" 2>/dev/null || true)"
echo "file: $file_out"

# Prefer readelf Machine / Class over string matching file(1).
class="$(readelf -h "$SO" | awk '/Class:/{print $2}')"
machine="$(readelf -h "$SO" | awk '/Machine:/{print $2}')"

if [[ "$class" != "ELF32" ]]; then
  echo "ERROR: expected ELF32 (Miyoo Mini), got Class=$class" >&2
  exit 1
fi

# ARM soft/hard float toolchains report "ARM".
if [[ "$machine" != "ARM" ]]; then
  echo "ERROR: expected Machine=ARM, got Machine=$machine" >&2
  exit 1
fi

needed="$(readelf -d "$SO" | awk '/NEEDED/{print $5}' | tr -d '[]' || true)"
echo "NEEDED:"
echo "$needed"

while IFS= read -r lib; do
  [[ -z "$lib" ]] && continue
  case "$lib" in
    libstdc++.so*|libgcc_s.so*)
      echo "ERROR: dynamic $lib — link with -static-libstdc++ -static-libgcc" >&2
      exit 1
      ;;
  esac
done <<< "$needed"

echo "OK: $SO is ELF32 ARM without dynamic libstdc++/libgcc_s"
