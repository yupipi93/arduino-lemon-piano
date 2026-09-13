#!/usr/bin/env bash
# Flash the probe, then ASK before recording, and wait for the answer.
#
#   nohup ./run-when-ready.sh pcb 60 >/tmp/probe.log 2>&1 &
#
# Why this exists, and why it is the default runner rather than a variant:
#
#   `run.sh` starts recording the instant it is launched. That is only correct
#   if a human is already standing at the board with a hand on the fruit. On
#   2026-09-13 an agent started a 160 s capture and then messaged Sergio to go
#   and play — which is backwards. He said so, and he was right:
#
#     "si vas a pasar una prueba y esperas que yo toque el piano primero le
#      tienes que avisar y tengo que aceptar que estoy delante... no puedes
#      esperar que esté yo siempre disponible, no soy una máquina, soy un
#      humano que tiene un hijo y responsabilidades"
#
#   A measurement that needs a person's hands is not ready when the machine is
#   ready. It is ready when the person says so. So: the board is flashed, two
#   buttons go to his phone, and this script SITS THERE — for hours if that is
#   how long it takes — until he taps "estoy delante". The machine waits for the
#   human. Never the other way round.
set -u

NAME="${1:-pcb}"
SECS="${2:-60}"
TIMEOUT_MIN="${3:-720}"          # 12 h. The window is his, not the script's.
HERE="$(cd "$(dirname "$0")" && pwd)"
PIO="${PIO:-$HOME/.local/venvs/pio/bin/pio}"
PY="${PY:-$HOME/.local/venvs/pio/bin/python}"
APPROVALS="${APPROVALS:-$HOME/code/quantumpc-management-agent/bin/qpc-approvals}"
OUT="$HOME/sonda-$NAME.txt"
ID="probe-$NAME-$(date +%Y%m%d-%H%M%S)"

PORT=$(ls /dev/ttyUSB* 2>/dev/null | head -1)
if [ -z "$PORT" ]; then
  echo "No /dev/ttyUSB* — the Nano is not plugged into THIS machine."
  exit 1
fi

echo "Board on $PORT. Flashing first, so the wait is the last thing that happens."
"$PIO" run -t upload -d "$HERE" || { echo "Upload failed."; exit 1; }

if [ ! -x "$APPROVALS" ]; then
  echo "qpc-approvals not found at $APPROVALS — refusing to record unannounced."
  echo "Run ./run.sh $NAME $SECS by hand, while you are at the board."
  exit 1
fi

"$APPROVALS" ask piano-probe "$ID" \
  --title "🍋 ¿Puedes tocar el piano ahora?" \
  --question "La placa ya está programada. Cuando estés delante y puedas tocar los limones ${SECS} segundos seguidos, dale a empezar y grabo. Si ahora no puedes, dale a ahora no y lo dejo — no corre prisa y no hay que repetir nada." \
  --approve-label "Estoy delante, graba" \
  --reject-label "Ahora no puedo" \
  --ttl-hours 24 || exit 1

echo "Asked. Waiting up to ${TIMEOUT_MIN} min for the answer — nothing is recording yet."
if ! "$APPROVALS" wait piano-probe "$ID" --timeout-minutes "$TIMEOUT_MIN"; then
  echo "Not approved (declined, or the window expired). Nothing recorded, nothing lost."
  exit 0
fi

echo "Approved. Recording ${SECS}s into $OUT"
"$PY" "$HERE/capture.py" "$SECS" "$OUT" "$PORT" >/dev/null || exit 1

echo "================ saved to $OUT ================"
grep -E "verdict:|since boot|baseline=" "$OUT" | tail -20
