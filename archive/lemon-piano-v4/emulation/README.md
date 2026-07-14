# 🍋 Lemon Piano V4 — interactive Velxio emulation ❄️ FROZEN

> **Frozen snapshot of the V4 emulation.** The active version is V5 (see the
> top-level `emulation/`). Paths below are written relative to the original
> repo-root `emulation/` location — from this archived folder add one `../`
> level (e.g. the harness is at `../../../../velxio-multi-board-emulator/`).
> See [../README.md](../README.md) for how to run it from here.

Play the 2019 lemon piano in your browser: real firmware, emulated
ATmega328 (avr8js), clickable lemons, audible buzzer, working LEDs and the
water-pump indicator. Built with the
[velxio-multi-board-emulator](../../velxio-multi-board-emulator/) pipeline
harness.

## Play it

```bash
# 1. Bring up the Velxio stack (Docker; first boot pulls a ~3.3 GB image once)
../../velxio-multi-board-emulator/harness/.venv/bin/velxio-pipeline stack up

# 2. Open http://localhost:3080/editor  (the root URL is Velxio's landing
#    page — the editor is at /editor, or click "Try Simulator Free Online").
#    Click the IMPORT button (tray-with-down-arrow icon, right of "Libraries"),
#    select  emulation/lemon-piano.vlx , then press Run (green ▶).
```

- **Keys 1–7** (yellow buttons) are the lemons. Click-and-hold = touch —
  or just **press 1–7 on your PC keyboard** (frontend patch 0002 maps digit
  keys to the pushbuttons labeled with that digit; click the page once first
  so it has focus, and don't have the code editor focused).
- **Game selector switch**: left = game 1 (Mario Main Theme), right =
  game 2 (Underworld, the default). The choice takes effect when you press
  RESTART (or at boot).
- Green LED = correct note, red = wrong (sequence resets), blue LED = the
  water pump firing (a miss from note 7 on), buzzer plays through your
  speakers (WebAudio).
- Secret codes: game 1 `6,5,6,7,2,5,2,1,3,4` · game 2 `3,6,1,4,2,5,3,6,1,4`.

To regenerate the `.vlx` (or run it headlessly) from the spec:

```bash
PIPE=../../velxio-multi-board-emulator/harness/.venv/bin/velxio-pipeline
$PIPE run --mode interactive --spec emulation/lemon-piano.yaml --out emulation/runs
$PIPE run --mode verify      --spec emulation/lemon-piano.yaml --out emulation/runs
$PIPE run --mode document    --spec emulation/lemon-piano.yaml --out emulation/runs
```

`verify` **plays game 2's secret code by injecting timed key presses** into
the headless emulator and asserts the firmware logs `WIN` (plus buzzer
activity and the green LED). It passes in ~15 s with no browser and no
hardware — the regression test for the game logic.

## Why the input layer is shimmed (`VELXIO_EMULATION`)

The real V4 senses lemons **analogically**: each key floats near 0 and rises
above `baseline + 100` when the player closes the 5 V circuit
([docs/HARDWARE.md](../docs/HARDWARE.md)). That divider cannot be reproduced
in the Velxio canvas:

1. Velxio's clickable parts drive pins **digitally, active-LOW** (idle is
   seeded HIGH) — the auto-calibration would set thresholds above 1023 and
   every key would go dead.
2. The browser AVR's `analogRead` only sees voltages injected on ADC
   channels 0–5 (drag-style parts like potentiometers) — unusable as piano
   keys, and channel 6 isn't injectable at all.
3. **A6 has no digital pin** on the ATmega328, so key 7 must move.

Velxio's electrical layer drives AVR inputs from the *solved node voltage*,
with one asymmetry that shaped the design: **digital** injection
(`connectDigitalInputsToMcu.ts`) only covers numerically-named pins (`"9"`),
while **analog** injection (`connectAnalogInputsToMcu.ts`) covers `A0`–`A5`.

So `firmware/src/main.cpp` gained a compile-time input shim
(`-DVELXIO_EMULATION`, see `KEY_PINS` / `keyTouched()`):

- **Keys 1–6**: pushbuttons + 10 kΩ pull-ups on **A0–A5**, read with
  `analogRead` — idle ≈ 1023 (5 V through the pull-up), pressed ≈ 0
  (button closes to GND). Active-low analog, threshold at 512.
- **Key 7**: pushbutton + pull-up on **D9** (A6 has no digital pin),
  `digitalRead`.
- **GAME SELECT / RESTART**: active-low with pull-ups.
- **Buzzer on D11 (not D8), all notes via `emuTone()`**: Velxio's buzzer part
  starts a WebAudio note when Timer2 duty goes >0 and stops it ONLY on a
  duty→0 event; duty is polled only on the PWM pins (3/5/6/9/10/11), and
  `noTone()` leaves OCR2A set. On D8 (or without clearing OCR2A after each
  note) the first tone starts an oscillator that beeps forever — even after
  the simulation stops. `emuTone()` = `tone()` → `delay()` → `noTone()` →
  `OCR2A = 0`. Minimal repro/diagnostic:
  `velxio-multi-board-emulator/circuits/buzzer-test.yaml`.
- **Boot guard**: `calibrate()` waits until the inputs read idle-high before
  starting the game — Velxio's first SPICE solve takes a moment, and until it
  lands every input reads 0 (= "everything pressed"), which used to spam
  notes and restarts (the "continuous beep" bug).

The pull-ups are mandatory: an active-low button with no pull-up floats near
0 V in the solver and reads permanently pressed. They're the emulation's
counterpart of the real build's per-key 220 Ω resistors — they define each
key's idle level. **Game logic, melodies, LEDs and the relay pair are
untouched**, and hardware builds are unaffected (`pio run` on all three
hardware envs produces the same 7296-byte binary as before; the shim path is
compile-checked by the new `emulation` env).

### Emulation pin map (differences vs. [docs/HARDWARE.md](../docs/HARDWARE.md) only)

| Real V4 | Emulation |
|---|---|
| Keys 1–7 = analog touch on A0–A6 (reading *rises* above baseline) | Keys 1–7 = pull-up + button on A0–A5 + **D9** (reading *drops* when pressed) |
| GAME_SELECT (D4) button held **HIGH** at boot ⇒ game 1 | Slide switch on D4: **left (GND)** ⇒ game 1, **right (5V)** ⇒ game 2; applied at RESTART |
| RESTART (D7) active-HIGH | RESTART active-LOW |
| Buzzer on D8 (`tone()` + bit-banged `buzz()`) | Buzzer on **D11** via `emuTone()` (Timer2-duty audio, explicit note-off) |
| Relay pair D5/D6 → water pump | Blue LED on D5 = pump indicator |

## Audio timing note

Stock Velxio's AVR frame loop ran the simulated clock ~1.3× faster than wall
time (it executed N *instructions* against an N-*cycle* budget), which made
buzzer notes lag seconds behind the clicks and overlap. The harness deploy
carries an isolated upstream patch that makes pacing cycle-accurate
(measured 1.00× after, 1.30× before) — see
`velxio-multi-board-emulator/patches/0001-avr-cycle-accurate-frame-pacing.patch`.
On an unpatched Velxio the game still works, just with laggy sound.

## Files

| File | What |
|---|---|
| `lemon-piano.yaml` | Circuit spec (source of truth): components, wiring, secret-code input script, assertions |
| `lemon-piano.vlx` | Generated project — import this into Velxio and play |
| `runs/` | Evidence bundles from verify/document runs (gitignored) |
