#!/usr/bin/env bash
# One command, one file. Flash the probe, record the serial log for a fixed
# window, and print the verdict lines at the end.
#
#   ./run.sh pcb          # the assembled v0.7.1 board
#   ./run.sh proto        # the working breadboard
#   ./run.sh pcb 120      # ...for 120 s instead of the default 75
#
# Two bugs are pinned here, both found the hard way on 2026-09-12:
#   - the upload used to run for BOTH environments, the second failed out of
#     sync against a just-reset board, and the `&&` chain broke before the
#     monitor ever started (fixed by default_envs in platformio.ini);
#   - recording used `pio device monitor`, i.e. miniterm, which writes NOTHING
#     when its stdout is a pipe or a file. It produced a zero-byte log from a
#     correctly flashed board. capture.py talks to pySerial directly instead.
set -u

NAME="${1:-probe}"
SECS="${2:-75}"
HERE="$(cd "$(dirname "$0")" && pwd)"
PIO="${PIO:-$HOME/.local/venvs/pio/bin/pio}"
PY="${PY:-$HOME/.local/venvs/pio/bin/python}"
OUT="$HOME/sonda-$NAME.txt"

[ -x "$PIO" ] || { echo "No PlatformIO at $PIO — set PIO=/path/to/pio"; exit 1; }

PORT=$(ls /dev/ttyUSB* 2>/dev/null | head -1)
if [ -z "$PORT" ]; then
  echo "No /dev/ttyUSB* — the Nano is not plugged into THIS machine."
  echo "(/dev/ttyS0 is the motherboard's own serial port, not the board.)"
  exit 1
fi
echo "Board on $PORT"

"$PIO" run -t upload -d "$HERE" || { echo "Upload failed."; exit 1; }

echo
echo "=================================================================="
echo ">>> TOUCH THE FRUIT NOW, the way you normally play. ${SECS} s. <<<"
echo "=================================================================="
echo
"$PY" "$HERE/capture.py" "$SECS" "$OUT" "$PORT" >/dev/null || exit 1

echo
echo "================ saved to $OUT ================"
grep -E "verdict:|since boot|baseline=" "$OUT" | tail -20
