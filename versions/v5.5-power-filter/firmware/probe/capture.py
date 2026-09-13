#!/usr/bin/env python3
"""Record the probe's serial output to a file, with a timestamp per line.

Deliberately NOT `pio device monitor`. That wraps pySerial's miniterm, which is
an interactive console: point its stdout at a pipe or a file and it writes
nothing at all. On 2026-09-12 that produced a zero-byte log from a correctly
flashed board, twice, and the second time it cost the user a whole evening.

Every log opens with a header naming the LABEL the operator chose, the port and
the USB path. The label is the only thing that says which physical rig produced
the log -- the script records whatever is on /dev/ttyUSB0, and both Nanos here
are indistinguishable CH340s (1a86:7523, no serial number). On 2026-09-13 a
capture arrived that could have been either rig, and the two readings of it were
opposite conclusions. A log that cannot say what it measured is not evidence.

Usage:  capture.py <seconds> <outfile> [port] [label]
"""
import sys, time, glob, subprocess, datetime
import serial


def usb_path(port):
    try:
        out = subprocess.run(["udevadm", "info", "-q", "property", "-n", port],
                             capture_output=True, text=True, timeout=5).stdout
        for line in out.splitlines():
            if line.startswith("ID_PATH="):
                return line.split("=", 1)[1]
    except Exception:
        pass
    return "unknown"


def main() -> int:
    secs = float(sys.argv[1]) if len(sys.argv) > 1 else 75.0
    path = sys.argv[2] if len(sys.argv) > 2 else "probe.txt"
    port = sys.argv[3] if len(sys.argv) > 3 else ""

    if not port:
        ports = sorted(glob.glob("/dev/ttyUSB*"))
        if not ports:
            print("No /dev/ttyUSB* — the Nano is not plugged into this machine.")
            print("(/dev/ttyS0 is the motherboard's own port, not the board.)")
            return 1
        port = ports[0]

    ser = serial.Serial(port, 9600, timeout=1)
    # Toggling DTR resets the Nano, so the log always starts at the boot block:
    # the seven baselines are the first thing that has to be in the file.
    ser.setDTR(False)
    time.sleep(0.1)
    ser.setDTR(True)

    label = sys.argv[4] if len(sys.argv) > 4 else "unlabelled"

    started = time.time()
    with open(path, "w") as out:
        for line in ("# probe capture",
                     "# label      : %s   <-- which rig the operator said this is" % label,
                     "# started    : %s" % datetime.datetime.now().isoformat(timespec="seconds"),
                     "# port       : %s" % port,
                     "# usb path   : %s" % usb_path(port),
                     "# seconds    : %g" % secs,
                     "#"):
            out.write(line + "\n")
            print(line)
        while time.time() - started < secs:
            line = ser.readline()
            if not line:
                continue
            text = "[%6.1f] %s" % (time.time() - started,
                                   line.decode("utf-8", "replace"))
            out.write(text)
            out.flush()          # flush per line: a killed run still has a log
            sys.stdout.write(text)
            sys.stdout.flush()
    return 0


if __name__ == "__main__":
    sys.exit(main())
