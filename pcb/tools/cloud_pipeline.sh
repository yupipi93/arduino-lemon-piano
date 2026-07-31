#!/usr/bin/env bash
# Lemon Piano V5.5 — one full iteration through the pcb-designer CLOUD API.
#
#   ./pcb/tools/cloud_pipeline.sh v0.1.1 [--skip-route] [--fab]
#
# The board lives in THIS repo (arduino-lemon-piano/pcb/); the toolkit is
# the sibling repo ../eda-pcb-designer (same protocol as wiring_diagrams.py
# consuming ../eda-wirewright). Docker mounts the common parent so both
# repos resolve inside the container.
#
# Stages (cloud for everything that has an endpoint):
#   build   (local Docker; generative, no endpoint)   -> base .kicad_pcb
#   /place  (cloud)  YAML placements applied           -> placed board
#   /route  (cloud)  freerouting + zone fill           -> routed board
#   post    (local Docker; widths + zone heal, no endpoint)
#   /drc    (cloud)  JSON report  -> validation/drc-<ver>.json  [HARD GATE]
#   /render (cloud)  raytraced    -> renders/<ver>-top.png / -bottom.png
#   ERC + verify_placement + verify_holes + geometry_gate       [HARD GATES]
#   /export3d (cloud)  GLB + STEP -> 3d/  (rotatable model; not a gate)
#   /fab    (cloud, only with --fab and all gates green)
set -euo pipefail

VER="${1:?usage: cloud_pipeline.sh vX.Y.Z [--skip-route] [--fab]}"
SKIP_ROUTE="${2:-}"
API="https://pcb-designer.scv.multitecua.com"
PARENT="$(cd "$(dirname "$0")/../../.." && pwd)"      # common parent of both repos
PROJ="$PARENT/arduino-lemon-piano/pcb"
PCB="$PROJ/kicad/lemon-piano.kicad_pcb"
CFG="$PROJ/lemon-piano.yaml"
IMG=eda-pcb-designer:latest

# LEMON_PCB_REBUILD=1 tells build_board.py that discarding the routing on the
# existing artefact is intentional here: /route re-applies it two steps down.
# Standalone runs lack the flag and are refused (build_board.guard_routed_board).
dk() { docker run --rm --user "$(id -u):$(id -g)" -e HOME=/tmp \
        -e LEMON_PCB_REBUILD=1 \
        -v "$PARENT":/work -w /work --entrypoint python3 "$IMG" "$@"; }

cd "$PARENT"
mkdir -p "$PROJ/validation" "$PROJ/renders"

echo "== [$VER] build (local docker, generative) =="
dk arduino-lemon-piano/pcb/tools/build_board.py

echo "== [$VER] cloud /place =="
curl -sf -F "pcb=@$PCB" -F "config=@$CFG" "$API/place" -o "$PCB.placed"
mv "$PCB.placed" "$PCB"

if [ "$SKIP_ROUTE" != "--skip-route" ]; then
  echo "== [$VER] cloud /route =="
  curl -sf -F "pcb=@$PCB" "$API/route?passes=30&optim=5" -o "$PCB.routed" \
    || { echo "ROUTE FAILED"; curl -s -F "pcb=@$PCB" "$API/route?format=json" | head -c 2000; exit 1; }
  mv "$PCB.routed" "$PCB"

  echo "== [$VER] post-route (local docker: widths + zone heal) =="
  dk arduino-lemon-piano/pcb/tools/post_route.py "arduino-lemon-piano/pcb/kicad/lemon-piano.kicad_pcb"
fi

echo "== [$VER] cloud /drc =="
curl -sf -F "pcb=@$PCB" "$API/drc" -o "$PROJ/validation/drc-$VER.json"
DRC_OK=1
python3 - "$PROJ/validation/drc-$VER.json" <<'EOF' || DRC_OK=0
import json, sys
d = json.load(open(sys.argv[1]))
rep = d.get("report", d)
v = rep.get("violations", [])
u = rep.get("unconnected_items", [])
sev = {}
for item in v:
    sev[item.get("severity", "?")] = sev.get(item.get("severity", "?"), 0) + 1
print(f"DRC: {len(v)} violations {sev}, {len(u)} unconnected")
for item in v[:12]:
    print(f"  [{item.get('severity')}] {item.get('type')}: {item.get('description', '')[:110]}")
if len(v) > 12:
    print(f"  ... {len(v) - 12} more")
sys.exit(0 if sev.get("error", 0) == 0 and len(u) == 0 else 1)
EOF

echo "== [$VER] cloud /render suite (normal + dim + realistic + overlay) =="
# naming: <ver>-<style>-<side>.png in renders/; the overlay composite also
# lands in renders/ (<ver>-overlay-*), while overlays/ keeps only the
# module photos + modules.yaml (the client-side assets).
for STYLE in normal dim realistic; do
  API_STYLE=$STYLE; [ "$STYLE" = "normal" ] && API_STYLE=bare
  curl -sf -F "pcb=@$PCB" "$API/render?side=both&style=$API_STYLE" \
    -o "/tmp/renders-$VER-$STYLE.zip"
  python3 - "/tmp/renders-$VER-$STYLE.zip" "$PROJ/renders" "$VER" "$STYLE" <<'EOF'
import sys, zipfile, shutil, os
zf, outdir, ver, style = sys.argv[1:5]
with zipfile.ZipFile(zf) as z:
    for n in z.namelist():
        side = "top" if "top" in n else "bottom"
        with z.open(n) as src, open(os.path.join(outdir, f"{ver}-{style}-{side}.png"), "wb") as dst:
            shutil.copyfileobj(src, dst)
        print("  saved", f"{ver}-{style}-{side}.png")
EOF
done
curl -sf -F "pcb=@$PCB" -F "modules=@$PROJ/overlays/modules.yaml" \
  -F "images=@$PROJ/overlays/component-images/arduino-nano.png" \
  "$API/render?side=both&style=overlay&calibration=green_bbox" \
  -o "/tmp/renders-$VER-overlay.zip"
python3 - "/tmp/renders-$VER-overlay.zip" "$PROJ/renders" "$VER" "overlay" <<'EOF'
import sys, zipfile, shutil, os
zf, outdir, ver, style = sys.argv[1:5]
with zipfile.ZipFile(zf) as z:
    for n in z.namelist():
        side = "top" if "top" in n else "bottom"
        with z.open(n) as src, open(os.path.join(outdir, f"{ver}-{style}-{side}.png"), "wb") as dst:
            shutil.copyfileobj(src, dst)
        print("  saved", f"{ver}-{style}-{side}.png")
EOF

if [ "$DRC_OK" != "1" ]; then
  echo "== [$VER] DRC GATE FAILED (errors or unconnected > 0) — artefacts archived, stopping before fab =="
  exit 1
fi

echo "== [$VER] schematic (local docker, generative) + ERC =="
dk arduino-lemon-piano/pcb/tools/build_schematic.py
docker run --rm --user "$(id -u):$(id -g)" -e HOME=/tmp -v "$PARENT":/work -w /work \
  --entrypoint bash "$IMG" -c '
mkdir -p /tmp/.config/kicad/9.0
{ echo "(sym_lib_table (version 7)"
  for f in /usr/share/kicad/symbols/*.kicad_sym; do
    n=$(basename "$f" .kicad_sym)
    echo " (lib (name \"$n\")(type \"KiCad\")(uri \"$f\")(options \"\")(descr \"\"))"
  done
  echo ")"; } > /tmp/.config/kicad/9.0/sym-lib-table
kicad-cli sch erc --output arduino-lemon-piano/pcb/validation/erc-'"$VER"'.txt \
  --exit-code-violations arduino-lemon-piano/pcb/kicad/lemon-piano.kicad_sch' \
  && echo "  ERC clean" || { echo "ERC VIOLATIONS"; exit 1; }

echo "== [$VER] cloud /export3d (rotatable GLB + STEP) =="
# The service ships the kicad-packages3d library, so component bodies always
# resolve there — a local export from the slim Docker image would silently
# drop them (see eda-pcb-designer export3d docs). Outputs are regenerable and
# multi-MB, so pcb/3d/ is gitignored.
mkdir -p "$PROJ/3d"
if curl -sf -F "pcb=@$PCB" "$API/export3d?format=both&version=$VER" \
     -o "/tmp/3d-$VER.zip"; then
  python3 - "/tmp/3d-$VER.zip" "$PROJ/3d" "$VER" <<'EOF3D'
import os, shutil, sys, zipfile
zf, outdir, ver = sys.argv[1:4]
with zipfile.ZipFile(zf) as z:
    for n in z.namelist():
        ext = n.rsplit(".", 1)[-1]
        dest = os.path.join(outdir, f"lemon-piano-{ver}.{ext}")
        with z.open(n) as src, open(dest, "wb") as dst:
            shutil.copyfileobj(src, dst)
        print(f"  saved {os.path.basename(dest)} "
              f"({os.path.getsize(dest) / 1e6:.1f} MB)")
EOF3D
  echo "  view it: drag the .glb onto https://3dviewer.net, or: f3d $PROJ/3d/lemon-piano-$VER.glb"
else
  echo "  [WARN] /export3d failed — skipping the 3D models (not a release gate)"
fi

echo "== [$VER] vision inputs (transparent-bg bare renders, LL §22) =="
curl -sf -F "pcb=@$PCB" "$API/render?side=both&style=bare&background=transparent" \
  -o "/tmp/vision-$VER.zip"
python3 - "/tmp/vision-$VER.zip" "$PROJ/validation" "$VER" <<'EOF2'
import sys, zipfile, shutil, os
zf, outdir, ver = sys.argv[1:4]
with zipfile.ZipFile(zf) as z:
    for n in z.namelist():
        side = "top" if "top" in n else "bottom"
        with z.open(n) as src, open(os.path.join(outdir, f"vision-{ver}-{side}.png"), "wb") as dst:
            shutil.copyfileobj(src, dst)
EOF2

echo "== [$VER] gates: verify_placement + verify_holes + geometry =="
python3 "$PROJ/tools/verify_placement.py" > "$PROJ/validation/verify-placement-$VER.txt" \
  && echo "  verify_placement PASS" || { tail -30 "$PROJ/validation/verify-placement-$VER.txt"; exit 1; }
dk arduino-lemon-piano/pcb/tools/verify_holes.py --version "$VER" \
  > "$PROJ/validation/verify-holes-$VER.txt" \
  && echo "  verify_holes PASS" || { cat "$PROJ/validation/verify-holes-$VER.txt"; exit 1; }
python3 "$PROJ/tools/geometry_gate.py" > "$PROJ/validation/geometry-$VER.txt" \
  && echo "  geometry_gate PASS" || { cat "$PROJ/validation/geometry-$VER.txt"; exit 1; }

if [ "${3:-}" = "--fab" ] || [ "$SKIP_ROUTE" = "--fab" ]; then
  echo "== [$VER] cloud /fab =="
  mkdir -p "$PROJ/releases/$VER"
  curl -sf -F "pcb=@$PCB" -F "sch=@$PROJ/kicad/lemon-piano.kicad_sch" \
    "$API/fab?version=$VER" -o "$PROJ/releases/$VER/lemon-piano-$VER-fab.zip"
  unzip -l "$PROJ/releases/$VER/lemon-piano-$VER-fab.zip" | tail -20
fi

echo "== [$VER] done =="
