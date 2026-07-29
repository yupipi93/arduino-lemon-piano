# 🍋 Lemon Piano V5 — interactive Velxio emulation

Play the V5 lemon piano in your browser: real firmware, emulated ATmega328
(avr8js), clickable lemons, audible buzzer, and the **ten-green-LED progress
bar**. Built with the
[velxio-multi-board-emulator](../../../../velxio-multi-board-emulator/) pipeline
harness. (The V4 emulation — red/green LEDs + water-pump indicator — is just as active, in
[`../../v4-water-pump/emulation/`](../../v4-water-pump/emulation/).)

> Run every command below **from the version directory** (`versions/v5-led-bar/`).

## Play it

```bash
# 1. Bring up the Velxio stack (Docker; first boot pulls a ~3.3 GB image once)
../../../velxio-multi-board-emulator/harness/.venv/bin/velxio-pipeline stack up

# 2. Open http://localhost:3080/editor  (the root URL is Velxio's landing
#    page — the editor is at /editor, or click "Try Simulator Free Online").
#    Click the IMPORT button (tray-with-down-arrow icon, right of "Libraries"),
#    select  emulation/lemon-piano.vlx , then press Run (green ▶).
```

- **Keys 1–7** (yellow buttons) are the lemons. Click-and-hold = touch —
  or just **press 1–7 on your PC keyboard** (frontend patch 0002 maps digit
  keys to the pushbutton labeled with that digit; click the page once first
  so it has focus, and don't have the code editor focused).
- The **ten green LEDs (labeled 1–10)** are the progress bar: each correct
  note lights the next LED; a wrong note blanks all ten and the sequence
  restarts from the first note. Light all ten → the theme plays.
- **No game-select switch, no restart button.** The browser starts at **game 1
  (Mario Main Theme)**; when you win, the theme plays and the game
  **auto-advances** to game 2 (Underworld), then back — both games cycle from
  one starting point. (On real hardware the A7 switch picks the starting game
  and D7 restarts; those pins don't exist in the browser build — see below.)
- Buzzer plays through your speakers (WebAudio).
- Secret codes: game 1 `6,5,6,7,2,5,2,1,3,4` · game 2 `3,6,1,4,2,5,3,6,1,4`.

To regenerate the `.vlx` (or run it headlessly) from the spec:

```bash
PIPE=../../../velxio-multi-board-emulator/harness/.venv/bin/velxio-pipeline
$PIPE run --mode verify      --spec emulation/lemon-piano.yaml --out emulation/runs
$PIPE run --mode interactive --spec emulation/lemon-piano.yaml --out emulation/runs
# then copy the generated runs/<latest>/project.vlx over emulation/lemon-piano.vlx
```

`verify` **plays game 1's secret code by injecting timed key presses** into the
headless emulator and asserts the firmware logs `WIN`, lights the LEDs, sings
through the buzzer, and then flips to `Game 2` (the auto-advance). It passes in
~6 s of wall time with no browser and no hardware — the regression test for the
game logic. Last run: **pass**, serial `Game 1 → OK 1/10 … 10/10 → WIN → Game 2`.

## Why the input layer is shimmed (`VELXIO_EMULATION`)

The real piano senses lemons **analogically**: each key floats near 0 and rises
above `baseline + 100` when the player closes the 5 V circuit
([../HARDWARE.md](../HARDWARE.md)). That divider cannot be reproduced in
the Velxio canvas, and with **ten LEDs every usable pin is spoken for** — the
browser AVR (avr8js) exposes only `A0`–`A5` (ADC injection) and `D2`–`D13`
(digital injection); `A6`/`A7` are absent and `D0`/`D1` are the UART. That's 18
lines, and 7 keys + 10 LEDs + buzzer already uses all 18.

So the emulation build swaps ONLY the input layer and drops the two panel
controls (`../firmware/src/main.cpp`, guarded by `-DVELXIO_EMULATION`):

- **Keys 1–6**: pushbuttons + 10 kΩ pull-ups on **A0–A5**, read with
  `analogRead` — idle ≈ 1023 (5 V through the pull-up), pressed ≈ 0 (button
  closes to GND). Active-low analog, threshold at 512.
- **Key 7**: pushbutton + pull-up on **D12** (A6 has no digital pin),
  `digitalRead`.
- **No GAME SELECT / RESTART**: no pins left. The browser starts at game 1 and
  each win auto-advances to the other theme, so both games are reachable with
  no switch and no restart.
- **Buzzer on D11 (not D8), all notes via `emuTone()`**: Velxio's buzzer part
  starts a WebAudio note when Timer2 duty goes >0 and stops it ONLY on a
  duty→0 event; duty is polled only on the PWM pins (3/5/6/9/10/11), and
  `noTone()` leaves OCR2A set. On D8 (or without clearing OCR2A after each
  note) the first tone beeps forever — even after the simulation stops.
  `emuTone()` = `tone()` → `delay()` → `noTone()` → `OCR2A = 0`.
- **Boot guard**: `calibrate()` waits until the inputs read idle-high before
  starting — Velxio's first SPICE solve takes a moment, and until it lands
  every input reads 0 (= "everything pressed"), which used to spam notes.

The pull-ups are mandatory: an active-low button with no pull-up floats near
0 V in the solver and reads permanently pressed. They're the emulation's
counterpart of the real build's per-key resistors — they define each key's idle
level. **Game logic, melodies and the LED bar are the same code as hardware**;
`pio run` on the two hardware Nano envs is unaffected (the shim path is
compile-checked by the `emulation` env).

### Emulation pin map (differences vs. [../HARDWARE.md](../HARDWARE.md) only)

| Real V5 (Nano) | Emulation |
|---|---|
| Keys 1–7 = analog touch on A0–A6 (reading *rises* above baseline) | Keys 1–6 = pull-up + button on **A0–A5** (reading *drops*); key 7 = pull-up + button on **D12** |
| 10 green LEDs on D2,D3,D4,D5,D6,**D9,D10,D11,D12,D13** | 10 green LEDs on D2,D3,D4,D5,D6,**D7,D8,D9,D10,D13** (D9–D13 freed differently since D7/D8 aren't restart/buzzer here) |
| GAME SELECT on A7 (switch picks starting game) | none — starts at game 1, wins auto-advance |
| RESTART on D7 (active-HIGH) | none — the bar auto-resets each round |
| Buzzer on D8 (`tone()` + bit-banged `buzz()`) | Buzzer on **D11** via `emuTone()` |

## Audio timing note

Stock Velxio's AVR frame loop ran the simulated clock ~1.3× faster than wall
time, which made buzzer notes lag and overlap. The harness deploy carries an
isolated upstream patch that makes pacing cycle-accurate (measured 1.00× after,
1.30× before) — see
`velxio-multi-board-emulator/patches/0001-avr-cycle-accurate-frame-pacing.patch`.
On an unpatched Velxio the game still works, just with laggy sound.

## Files

| File | What |
|---|---|
| `lemon-piano.yaml` | Circuit spec (source of truth): components, wiring, secret-code input script, assertions |
| `lemon-piano.vlx` | Generated project — import this into Velxio and play |
| `free-play.yaml` | Regression: five non-first-note keys sound but never start the sequence |
| `hold-and-repeat.yaml` | Regression: a held key sustains + counts once; a repeat is ignored, not `WRONG` |
| `all-levels-win.yaml` | The scripted "virtual button" — plays all **four** levels' secret codes back to back and asserts the auto-advance chain through `ALL LEVELS CLEAR` and the wrap to level 1, headlessly (`--mode verify`). Useful for testing any level (including 3/4, added 2026-07-29) without clicking 40 notes by hand — but it's a fixed script, not something you press live. See the timing note in its own header comment: each level's inputs start only once the previous level's full victory sequence has had time to finish, or the presses land while the AVR is mid-`delay()` and are silently lost. |
| `autoplayer.yaml` / `autoplayer.vlx` | The **interactive** version of the same idea: a second Arduino Uno (`player`) with a PLAY button and a LEVEL SELECT button, wired onto the same key nodes as the real lemons. Press LEVEL SELECT to arm 1-4 (its own 4 LEDs show which), then PLAY to fire that level's code — a real button you click live, as many times as you like, in any order, while the lemons stay clickable for manual free play. Multi-board specs are **interactive-mode only** in this harness (no headless `verify`/`document` — confirmed by trying both), so this one is validated by compiling each board's sketch standalone (`arduino-cli compile --fqbn arduino:avr:nano` / `...:uno`) and generating the `.vlx` (`routing: loose` — 3-way junctions at each key node overlap the strict lane router, harmlessly). See the file's own header comment for the two boards' full pin maps. |
| `runs/` | Evidence bundles from verify/document runs (gitignored) |
