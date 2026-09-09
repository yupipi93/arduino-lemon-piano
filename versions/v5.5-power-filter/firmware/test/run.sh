#!/usr/bin/env bash
# Host tests for the V5.5 firmware. No Arduino, no emulator, no hardware:
# plain g++, two binaries, exit 0 = green.
#
#   versions/v5.5-power-filter/firmware/test/run.sh
#
#   1. ui_gestures_test  — the two-button state machine on its own: the mode
#      menu, the sensitivity knob and the smart-adjust gesture sharing two
#      buttons without colliding (include/ui_gestures.h).
#   2. piano_sim_test    — the WHOLE firmware (src/main.cpp, verbatim) driven on
#      a fake board: note mapping, free play, the wheel, winning, cancelling.
#
# Run both before every commit that touches the firmware. They are the only
# regression tests that reach the mode wheel at all: the Velxio emulation has no
# pins left for the two buttons (see ../../emulation/README.md).
set -euo pipefail
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
out="$(mktemp -d)"
trap 'rm -rf "$out"' EXIT

g++ -std=c++11 -Wall -Wextra -Werror -O1 \
    -o "$out/ui_gestures_test" "$here/ui_gestures_test.cpp"
"$out/ui_gestures_test"

# -Wall only: this one compiles the firmware, which is written for avr-gcc and
# is not expected to be clean under the host compiler's stricter -Wextra.
g++ -std=c++11 -Wall -O1 -I "$here/arduino" -I "$here/../include" \
    -o "$out/piano_sim_test" "$here/piano_sim_test.cpp"
"$out/piano_sim_test" "$@"
