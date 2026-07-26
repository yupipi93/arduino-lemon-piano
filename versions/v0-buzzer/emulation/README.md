# 🍋 Lemon Piano V0 — interactive Velxio emulation

> Run every command below **from the version directory** (`versions/v0-buzzer/`).

The reference recording for the whole project: the same firmware as the hardware
build, playing a C-major scale up and down forever, in the browser. Use it to
judge "my buzzer doesn't sound as usual" — play this, play your board, compare.

## Play it

```bash
# 1. Bring up the Velxio stack (Docker; first boot pulls a ~3.3 GB image once)
../../../velxio-multi-board-emulator/harness/.venv/bin/velxio-pipeline stack up

# 2. Open http://localhost:3080/editor, click IMPORT (tray-with-down-arrow icon,
#    right of "Libraries"), select  emulation/lemon-piano.vlx , press Run (green ▶).
```

There is nothing to click: V0 has no keys, no switches and no buttons. It just
plays through your speakers (WebAudio), 14 notes per pass, 700 ms between passes.

## Headless regression test

```bash
PIPE=../../../velxio-multi-board-emulator/harness/.venv/bin/velxio-pipeline
$PIPE run --mode verify --spec emulation/lemon-piano.yaml --out emulation/runs
# then refresh the importable project:
cp emulation/runs/<latest>/project.vlx emulation/lemon-piano.vlx
```

`verify` asserts:

| Assertion | Why it matters |
|---|---|
| serial contains `Lemon Piano V0` | the firmware booted |
| serial contains `path: tone()` | the expected playback path was compiled in |
| pin 11 toggles ≥ 500 times in 8 s | the buzzer pin really is being driven (7 496 edges observed) |
| serial contains `scale done` | the scale **finishes and repeats** — i.e. notes actually END |

That last one is the important one: the classic browser failure in this project is
a first note that beeps forever. Last run: **pass** (2026-07-26).

## Why the buzzer is on D11 here (not D8)

Velxio's buzzer part starts a WebAudio note when Timer2 duty goes above 0 and stops
it **only** on a duty→0 event — and duty is polled only on the PWM pins
(3/5/6/9/10/11). On D8 nothing would be polled at all, and `noTone()` leaves
`OCR2A` set, so the note-off never fires. The emulation build therefore:

- moves the buzzer to **D11**, and
- plays every note through `emuTone()` = `tone()` → `delay()` → `noTone()` →
  `OCR2A = 0`.

This is the same shim every other version uses. Hardware builds (macro undefined)
keep the buzzer on **D8** — check the serial banner, which prints the pin it is
actually using.

| Real V0 | Emulation |
|---|---|
| Buzzer on **D8**, `tone()` (or bit-banged `buzz()` with `-DUSE_BUZZ`) | Buzzer on **D11** via `emuTone()` |
| On-board LED D13 lit per note | same (D13 is drawn on the Velxio board) |

## Audio timing note

Stock Velxio's AVR frame loop ran the simulated clock ~1.3× faster than wall time,
which made notes lag and overlap. The harness deploy carries an isolated upstream
patch that makes pacing cycle-accurate (1.00× after, 1.30× before) — see
`velxio-multi-board-emulator/patches/0001-avr-cycle-accurate-frame-pacing.patch`.
On an unpatched Velxio the scale still plays, just with laggy sound — worth knowing
before you conclude your hardware is at fault.

## Files

| File | What |
|---|---|
| `lemon-piano.yaml` | circuit spec (one buzzer) + assertions |
| `lemon-piano.vlx` | generated project — import this into Velxio and Run |
| `runs/` | evidence bundles from verify runs (gitignored) |
