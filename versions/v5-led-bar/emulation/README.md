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

## Multi-board autoplayer (interactive, live)

`autoplayer.yaml` / `autoplayer.vlx` puts a **second Arduino** on the same
canvas as the real V5 board, purely as a testing aid: type a level number
into it and it plays that level's code onto the real lemons on demand,
while the lemons themselves stay fully clickable the whole time. This is
the only way to drive the game live without clicking all 40 notes by hand
across the four levels (`all-levels-win.yaml` does the same thing but as a
fixed headless script — see the Files table below).

### Architecture

```
┌─────────────────────────┐         ┌──────────────────────────────┐
│  arduino-nano  ("piano") │         │   arduino-uno  ("player")     │
│  = firmware/src/main.cpp │         │  = inline sketch in this spec │
│  (VELXIO_EMULATION)      │         │  (autoplayer-only, not the    │
│                          │         │   real game)                  │
│  A0-A5, D12  7 keys ─────┼──220Ω───┼── D6-D12  7 "finger" outputs  │
│  D2-D10,D13  10 LEDs     │  (fr1-  │  D2-D5    4 level-armed LEDs  │
│  D11         buzzer      │   fr7)  │  Serial   level select + play │
└─────────────────────────┘         └──────────────────────────────┘
```

- **piano** is the unmodified V5 game — identical circuit to
  `lemon-piano.yaml`, same firmware, same secret codes.
- **player** is a small standalone sketch (inline in `autoplayer.yaml`,
  nowhere near `firmware/`) whose only job is to pulse piano's own key pins
  LOW in the right sequence, exactly like a finger would.
- The two boards share a **GND reference** (`arduino-uno:GND.1` ↔
  `arduino-nano:GND.1`) and nothing else — no shared power rail needed since
  both run off the simulator's own 5 V.

### Using it

```bash
cd versions/v5-led-bar
PIPE=../../../velxio-multi-board-emulator/harness/.venv/bin/velxio-pipeline
$PIPE run --mode interactive --spec emulation/autoplayer.yaml --out emulation/runs --open
```

Click Run. On the **`arduino-uno` Serial Monitor tab** (not the `arduino-nano`
one — each board has its own tab), type a digit **1-4** + Enter to arm that
level (its 4 blue LEDs show which), then **`p`** + Enter to play it onto the
lemons. Watch the `arduino-nano` tab for the real game's own log (`Level N`,
`OK X/10`, `WIN`). The player board has no idea what level piano is actually
on — match the number yourself. Your own lemon clicks work at the same time,
in the same session, with no need to stop or reset anything.

### Why a plain "wire it up" attempt doesn't work here

Five real, Velxio-version-specific behaviors had to be worked around, each
one confirmed by actually driving the browser (headless Playwright: import
the `.vlx`, click Run, dispatch button events, read each board's own Serial
Monitor, `window.__spiceDebug()` for node voltages) rather than assumed from
reading source:

1. **Board ids must start with a recognised kind string**
   (`arduino-nano`, `arduino-uno`, …) — `isBoardComponent()` checks
   kind-prefixes, not whatever id a spec assigns. Custom ids (this file used
   to say `piano`/`player`) silently break pin resolution for every
   pushbutton on **every** board, piano's 7 keys included.
2. **Cross-board wires must land directly on a board pin**, never on a
   passive in between — Velxio's cross-board bridge (`Interconnect.ts`)
   only routes a wire when both ends are literal board components.
3. **Pushbuttons on the second (non-primary) board don't work at all** —
   click handlers are bound to one global "active board" simulator, not one
   per board. There's no spec-level fix, which is why level-select/play are
   **typed serial commands**, not physical buttons — serial I/O is
   genuinely per-board here.
4. **A finger pin wired bare (no resistor) to piano's key pin is a hard
   short** that always beats a real lemon click — both are on the same
   node, and the digital fast-path both AVR boards use has no room for a
   press to win against a driven output. Fixed with a 220Ω resistor per
   finger wire (`fr1`-`fr7`), turning it into an ordinary voltage divider.
5. **A cross-board pin's `pinMode` can't be toggled at runtime and still
   propagate** — it has to be set to `OUTPUT` once in `setup()` and stay
   there; toggling to `INPUT` and back (tried, to avoid #4 a different way)
   silently stopped the connection from working in either direction.

Full write-up, with the two dead-end attempts that came before the working
one: `CHANGELOG.md`, entries "V5 autoplayer v2-v4" and "v5". The general
version of all five (useful on any future multi-board project, not just
this one) is documented in
`velxio-multi-board-emulator/AGENTS.md` under "Multi-board interactive
circuits (will bite you)".

## Files

| File | What |
|---|---|
| `lemon-piano.yaml` | Circuit spec (source of truth): components, wiring, secret-code input script, assertions |
| `lemon-piano.vlx` | Generated project — import this into Velxio and play |
| `free-play.yaml` | Regression: five non-first-note keys sound but never start the sequence |
| `hold-and-repeat.yaml` | Regression: a held key sustains + counts once; a repeat is ignored, not `WRONG` |
| `all-levels-win.yaml` | The scripted "virtual button" — plays all **four** levels' secret codes back to back and asserts the auto-advance chain through `ALL LEVELS CLEAR` and the wrap to level 1, headlessly (`--mode verify`). Useful for testing any level (including 3/4) without clicking 40 notes by hand — but it's a fixed script, not something you press live. |
| `autoplayer.yaml` / `autoplayer.vlx` | The **interactive**, live-pressable version — see "Multi-board autoplayer" above. |
| `runs/` | Evidence bundles from verify/document runs (gitignored) |
