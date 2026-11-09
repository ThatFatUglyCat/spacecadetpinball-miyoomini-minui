#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Whitelist packaging for PINBALL.pak — never zip the whole tree.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${1:-$ROOT/build/host}"
DIST="$ROOT/dist"
PAK_NAME="PINBALL.pak"
STAGE="$DIST/$PAK_NAME"

CORE_SO=""
for cand in \
  "$BUILD_DIR/spacecadetpinball_libretro.so" \
  "$BUILD_DIR/libspacecadetpinball_libretro.so"; do
  if [[ -f "$cand" ]]; then
    CORE_SO="$cand"
    break
  fi
done

if [[ -z "$CORE_SO" ]]; then
  echo "ERROR: core .so not found under $BUILD_DIR" >&2
  exit 1
fi

rm -rf "$STAGE"
mkdir -p "$STAGE"

cp "$ROOT/pak/PINBALL.pak/launch.sh" "$STAGE/launch.sh"
cp "$ROOT/pak/PINBALL.pak/default.cfg" "$STAGE/default.cfg"
cp "$ROOT/pak/PINBALL.pak/README.txt" "$STAGE/README.txt"
cp "$CORE_SO" "$STAGE/spacecadetpinball_libretro.so"
chmod +x "$STAGE/launch.sh"

# Positive allowlist check
allowed=(launch.sh default.cfg README.txt spacecadetpinball_libretro.so)
while IFS= read -r -d '' f; do
  base="$(basename "$f")"
  ok=0
  for a in "${allowed[@]}"; do
    if [[ "$base" == "$a" ]]; then ok=1; break; fi
  done
  if [[ "$ok" -ne 1 ]]; then
    echo "ERROR: unexpected file in pak staging: $f" >&2
    exit 1
  fi
done < <(find "$STAGE" -type f -print0)

mkdir -p "$DIST"
(
  cd "$DIST"
  rm -f "${PAK_NAME}.zip"
  if command -v zip >/dev/null 2>&1; then
    zip -r "${PAK_NAME}.zip" "$PAK_NAME"
  else
    python3 - <<'PY'
import hashlib, pathlib, zipfile
dist = pathlib.Path(".")
pak = dist / "PINBALL.pak"
zpath = dist / "PINBALL.pak.zip"
with zipfile.ZipFile(zpath, "w", zipfile.ZIP_DEFLATED) as zf:
    for f in sorted(pak.iterdir()):
        if f.is_file():
            zf.write(f, arcname=f"PINBALL.pak/{f.name}")
print("zipped via python")
PY
  fi
  if command -v sha256sum >/dev/null 2>&1; then
    sha256sum "${PAK_NAME}.zip" > SHA256SUMS
  else
    python3 - <<'PY'
import hashlib, pathlib
p = pathlib.Path("PINBALL.pak.zip")
h = hashlib.sha256(p.read_bytes()).hexdigest()
pathlib.Path("SHA256SUMS").write_text(f"{h}  PINBALL.pak.zip\n")
PY
  fi
)

echo "Packaged $DIST/${PAK_NAME}.zip"
