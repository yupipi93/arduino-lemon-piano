#!/usr/bin/env bash
# One command, one file. Flash the probe, record the serial log for a fixed
# window, and print the verdict lines at the end.
#
#   ./run.sh pcb          # the assembled v0.7.1 board
#   ./run.sh proto        # the working breadboard
#   ./run.sh pcb 120      # ...for 120 s instead of the default 75
#
# Written because doing this by hand cost a round trip on 2026-09-12: the upload
# succeeded, a second environment then failed, the `&&` chain broke, and the
# monitor never ran — so a correctly flashed board printed to nobody.
set -u

NAME="${1:-probe}"
SECS="${2:-75}"
HERE="$(cd "$(dirname "$0")" && pwd)"
PIO="${PIO:-$HOME/.local/venvs/pio/bin/pio}"
OUT="$HOME/sonda-$NAME.txt"

[ -x "$PIO" ] || { echo "No PlatformIO at $PIO — set PIO=/path/to/pio"; exit 1; }

PORT=$(ls /dev/ttyUSB* 2>/dev/null | head -1)
if [ -z "$PORT" ]; then
  echo "No /dev/ttyUSB* — the Nano is not plugged into THIS machine."
  echo "(/dev/ttyS0 is the motherboard's own serial port, not the board.)"
  exit 1
fi
echo "Board on $PORT — recording ${SECS}s into $OUT"

"$PIO" run -t upload -d "$HERE" || { echo "Upload failed."; exit 1; }

echo
echo ">>> TOUCH THE FRUIT NOW, the way you normally play. ${SECS} seconds. <<<"
echo
timeout "$SECS" "$PIO" device monitor -d "$HERE" --port "$PORT" --quiet \
  2>/dev/null | tee "$OUT"

echo
echo "================ saved to $OUT ================"
grep -E "verdict:|since boot|baseline=" "$OUT" | tail -20
