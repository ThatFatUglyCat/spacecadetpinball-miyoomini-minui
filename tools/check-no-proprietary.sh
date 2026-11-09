#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Fail if the repository appears to contain proprietary game assets.
set -euo pipefail

fail=0
say() { echo "ERROR: $*" >&2; fail=1; }

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  say "not a git repository"
  exit 1
fi

# 1) Forbidden names/extensions in tracked files
while IFS= read -r f; do
  lower="${f,,}"
  case "$lower" in
    *.dat|*.mid|*.midi|*.sf2|*.exe|*.dll|*.bin|*.wav|*.bmp)
      say "forbidden tracked file: $f" ;;
  esac
  base="$(basename "$lower")"
  case "$base" in
    pinball.dat|cadet.dat|pinball.mid|cadet.mid)
      say "suspicious game asset name: $f" ;;
  esac
done < <(git ls-files)

# 2) Large files (> 2 MiB) outside an explicit allowlist
allowlist="tools/large-file-allowlist.txt"
while IFS= read -r f; do
  [[ -f "$f" ]] || continue
  case "$f" in
    third_party/*) continue ;;
  esac
  size=$(stat -c %s "$f" 2>/dev/null || stat -f %z "$f")
  if [[ "$size" -gt 2097152 ]]; then
    if [[ -f "$allowlist" ]] && grep -qxF "$f" "$allowlist"; then
      continue
    fi
    say "large file not allowlisted: $f ($size bytes)"
  fi
done < <(git ls-files)

# 3) DAT magic header outside tools/docs that mention it intentionally
if git grep -Iq 'PARTOUT(4.0)RESOURCE' -- . \
    ':!tools/check-no-proprietary.sh' \
    ':!docs/' \
    ':!third_party/' \
    ':!src/libretro/content.cpp' \
    ':!src/libretro/content.h' \
    ':!tests/unit/content_test.cpp' 2>/dev/null; then
  say "found DAT magic header in unexpected tracked file"
fi

exit "$fail"
