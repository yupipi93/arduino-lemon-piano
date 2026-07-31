#!/usr/bin/env bash
# Export the board as interactive 3D models (GLB for browsers, STEP for CAD).
#
#   ./pcb/tools/export_3d.sh            # uses the current lemon-piano.kicad_pcb
#
# Why this script exists: the eda-pcb-designer Docker image ships KiCad 9 but
# NOT kicad-packages3d (kept small on purpose), so a bare `kicad-cli pcb export
# glb` produces a board with no component bodies. This fetches only the ~15
# STEP files this board actually references (~1.3 MB, cached in pcb/3d/models/)
# and points KICAD9_3DMODEL_DIR at them.
#
# Outputs land in pcb/3d/ and ARE committed, so a clone gives you a rotatable
# board with no KiCad toolchain. Only models/ (the fetched upstream body cache)
# is gitignored — third-party files, re-downloaded in seconds.
set -euo pipefail

PARENT="$(cd "$(dirname "$0")/../../.." && pwd)"
PROJ="$PARENT/arduino-lemon-piano/pcb"
PCB="$PROJ/kicad/lemon-piano.kicad_pcb"
OUT="$PROJ/3d"
MODELS="$OUT/models"
IMG=eda-pcb-designer:latest
VER="$(grep -m1 '^  version:' "$PROJ/lemon-piano.yaml" | awk '{print $2}')"
BASE="https://gitlab.com/kicad/libraries/kicad-packages3D/-/raw/master"

mkdir -p "$MODELS"

# ── fetch the models this board references, skipping ones already cached ────
echo "== collecting 3D models referenced by the board =="
fetch() {  # fetch <relative/path> -> 0 on success, 1 if upstream has no such file
  local p="$1"
  [ -s "$MODELS/$p" ] && return 0
  mkdir -p "$MODELS/$(dirname "$p")"
  if curl -sfL --max-time 60 "$BASE/$p" -o "$MODELS/$p" && [ -s "$MODELS/$p" ]; then
    return 0
  fi
  rm -f "$MODELS/$p"
  return 1
}

mapfile -t PATHS < <(grep -o '\${KICAD9_3DMODEL_DIR}/[^"]*' "$PCB" \
                     | sed 's|${KICAD9_3DMODEL_DIR}/||' | LC_ALL=C sort -u)
for p in "${PATHS[@]}"; do
  if [ -s "$MODELS/$p" ]; then
    continue
  elif fetch "$p"; then
    echo "  fetched  $p"
  elif [[ "$p" == *LED_D3.0mm_Orange.step ]]; then
    # No orange body exists upstream (ADR-021: the render service ships a
    # recoloured copy). Fall back to the base red one so the export completes
    # — the two orange LEDs just show red in these files. Fetch the base
    # explicitly rather than assuming the loop reached it first.
    if fetch "LED_THT.3dshapes/LED_D3.0mm.step"; then
      cp "$MODELS/LED_THT.3dshapes/LED_D3.0mm.step" "$MODELS/$p"
      echo "  substituted $p  (upstream has no orange body; using red)"
    else
      echo "  MISSING  $p  — D9/D10 will have no body"
    fi
  else
    echo "  MISSING  $p  — that component will have no body"
  fi
done

dk() { docker run --rm --user "$(id -u):$(id -g)" -e HOME=/tmp \
         -e KICAD9_3DMODEL_DIR=/models \
         -v "$PARENT":/work -v "$MODELS":/models -v "$OUT":/out \
         -w /work --entrypoint kicad-cli "$IMG" "$@"; }

REL="arduino-lemon-piano/pcb/kicad/lemon-piano.kicad_pcb"
COMMON=(--include-tracks --include-pads --include-zones
        --include-silkscreen --include-soldermask --subst-models -f)

echo "== GLB (drag into a browser / Windows 3D Viewer) =="
dk pcb export glb "${COMMON[@]}" -o "/out/lemon-piano-$VER.glb" "$REL" | tail -2

echo "== STEP (FreeCAD, Fusion, enclosure design) =="
dk pcb export step "${COMMON[@]}" -o "/out/lemon-piano-$VER.step" "$REL" | tail -2

echo
echo "Done — files in pcb/3d/:"
ls -lh "$OUT"/lemon-piano-"$VER".* | awk '{print "  ", $9, $5}'
echo
echo "View them:"
echo "  VS Code  double-click the .glb (or .step) — needs the extension this repo"
echo "           recommends: code --install-extension thingraph.cad-viewer"
echo "  web      drag the .glb onto https://3dviewer.net (renders in-browser)"
echo "  native   sudo apt install f3d   then   f3d $OUT/lemon-piano-$VER.glb"
