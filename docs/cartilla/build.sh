#!/usr/bin/env bash
# Build the organiser's booklet: A4 landscape, two sides, folds into an A5.
#
# The four level codes are NOT typed into the template. `seqs.py` reads them out
# of the firmware and resolves each level's note sequence against that level's
# own seven-note keyboard row, because which lemon plays a note is different on
# every level. Change a melody in main.cpp, run this, and the booklet is right
# again; type the codes by hand instead and you have a second source of truth
# that nothing checks.
#
#   ./build.sh            # -> cartilla.pdf  (+ pg-1.png / pg-2.png to look at)
#   ./build.sh --print    # ...and send it to the laser, duplex, short edge
set -euo pipefail
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
out="$(mktemp -d)"; trap 'rm -rf "$out"' EXIT
mkdir -p "$out/fonts"

# Press Start 2P is the pixel face; DejaVu carries everything with an accent,
# because Press Start 2P has no accented capitals and draws "Í" as "í".
cp "$here/../../../3d-modeling-agent/recetas/piano-limones-caja-v7/fuentes/PressStart2P-Regular.ttf" \
   "$out/fonts/" 2>/dev/null || cp "$(fc-match 'Press Start 2P' --format='%{file}')" "$out/fonts/PressStart2P-Regular.ttf"
cp "$(fc-match 'DejaVu Sans'      --format='%{file}')" "$out/fonts/body.ttf"
cp "$(fc-match 'DejaVu Sans:bold' --format='%{file}')" "$out/fonts/body-bold.ttf"

python3 "$here/seqs.py" "$out/codes.json"
python3 - "$here/cartilla.html" "$out/codes.json" "$out/cartilla.built.html" <<'PY'
import json, sys
tpl, codes, dst = sys.argv[1:4]
c = json.load(open(codes))
html = open(tpl).read()
assert '__CODES__' in html, 'the template lost its injection point'
open(dst, 'w').write(html.replace('__CODES__',
    json.dumps({k: {'keys': v['keys']} for k, v in c.items()})))
PY
cp "$here/pdf.js" "$out/"
docker run --rm -v "$out:/work" -w /usr/src/app -e NODE_PATH=/usr/src/app/node_modules \
  --entrypoint node zenika/alpine-chrome:with-puppeteer /work/pdf.js
cp "$out/cartilla.pdf" "$here/cartilla.pdf"
pdftoppm -r 100 -png "$here/cartilla.pdf" "$out/pg" && cp "$out"/pg-*.png "$here/" 2>/dev/null || true
echo "built $here/cartilla.pdf"

if [ "${1:-}" = "--print" ]; then
  # The content is LANDSCAPE, so the side that reads like a book is the one
  # flipped about the paper's SHORT edge. Long-edge would come out upside down.
  lp -d laser -n 1 -o media=A4 -o ColorModel=Gray -o sides=two-sided-short-edge \
     -t "Cartilla Piano de Limones" "$here/cartilla.pdf"
fi
