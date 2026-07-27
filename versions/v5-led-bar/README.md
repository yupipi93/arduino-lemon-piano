# V5 — ten-LED progress bar (2026) — newest board

No relays, no pump, no red LED, no fail counter: a row of **ten green LEDs** is
the entire feedback surface. Each correct note lights the next LED, a wrong note
blanks all ten, and lighting the tenth wins — the theme plays with the bar
flashing to the beat, then the game **auto-advances** to the other tune.

**Hardware delta vs [V4.5](../v4.5-margin-buttons/):**

- **off** the board: the **red LED** and the two **MARGIN buttons** (the relay pair
  and the water pump already left with V4.5);
- **on** the board: **ten green LEDs** with 220 Ω each on D2,D3,D4,D5,D6,D9,D10,
  D11,D12,D13;
- **game select moves to A7** (analog-in only) to free the tenth LED's pin;
- board: **Arduino Nano only** — V5 needs A6 *and* A7, which a classic Uno lacks.

<div align="center">
<img src="images/wiring-v5.png" alt="V5 wiring diagram" width="92%"/>
</div>

## How it plays

1. **Power on.** The A7 game-select switch picks the **starting** game: tied HIGH
   (5 V) = game 1 (Mario Main Theme), LOW (GND) = game 2 (Underworld).
2. **Touch lemons.** Hold the 5 V clip in one hand and touch a lemon with the
   other — your body closes the circuit and the note plays on the buzzer.
3. **Play freely.** While the bar is empty, every key just sounds its note —
   no wrong tone, no penalty. It is a piano; noodle as long as you like.
4. **The puzzle starts when you hit the code's first note** (LED 1 lights). From
   there each correct note lights the next green LED (the bar climbs 1 → 10),
   and a wrong note **blanks all ten** and drops you back to free play. The low
   "wrong" tone plays **after** the note you pressed has finished — you always
   hear which key was wrong, then the buzz.
5. **Victory + auto-advance.** Light all ten → the theme plays with the bar
   flashing per note, then the game flips to the **other** theme automatically
   (game 1 → 2 → 1 …), so both games cycle from one starting point.
6. **Restart** anytime with the button on D7 (re-reads game select, recalibrates,
   blanks the bar).

### Secret codes (spoilers)

| Game | Melody | Code |
|---|---|---|
| 1 | Super Mario Bros — Main Theme | `6, 5, 6, 7, 2, 5, 2, 1, 3, 4` |
| 2 | Super Mario Bros — Underworld | `3, 6, 1, 4, 2, 5, 3, 6, 1, 4` |

| Key | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|---|---|---|---|---|---|---|---|
| Game 1 note | E6 | G6 | A6 | B6 | C7 | E7 | G7 |
| Game 2 note | A3 | A#3 | C4 | A4 | A#4 | C5 | D5 |

Touch sensing keeps V4.5's noise-adaptive calibration (`baseline + max(40, 3 ×
noise)`, re-run on every RESTART) — it just has no MARGIN buttons to nudge it,
because those pins are LEDs now.

## Firmware

```bash
cd firmware
pio run                          # nanoatmega328 (default, old bootloader)
pio run -e nanoatmega328new      # Nano with the new bootloader
pio run -e emulation             # compile-check the VELXIO_EMULATION shim
pio run -t upload
pio device monitor               # 9600 baud — logs Game/OK n/10/WIN
```

There is no `uno` env: V5 needs A6 (key 7) + A7 (game select).

## Emulation — ✅ verify green

```bash
PIPE=../../../velxio-multi-board-emulator/harness/.venv/bin/velxio-pipeline
$PIPE stack up                    # then import emulation/lemon-piano.vlx at http://localhost:3080/editor
$PIPE run --mode verify --spec emulation/lemon-piano.yaml --out emulation/runs
```

Clickable lemons (or press `1`–`7` on your keyboard), audible buzzer, the ten-LED
bar. Because ten LEDs use every free browser pin there is **no game-select switch
and no restart button**: it starts at game 1 and auto-advances on each win.
There are **two** headless specs, both green (2026-07-27):

| Spec | What it proves |
|---|---|
| `emulation/lemon-piano.yaml` | free play → first note → a miss (`WRONG`) → the full code → `WIN` → auto-advance to `Game 2`, plus the victory light show |
| `emulation/free-play.yaml` | five keys that are *not* the code's first note: the buzzer sings, but `WRONG` and `OK` never appear and LED 1 never lights |

```bash
$PIPE run --mode verify --spec emulation/free-play.yaml --out emulation/runs
```

Full details, key mapping and the pin-map diff:
[emulation/README.md](emulation/README.md).

## Files

| Path | What |
|---|---|
| [firmware/src/main.cpp](firmware/src/main.cpp) | the V5 game (10-LED bar, auto-advance) + Velxio shim |
| [firmware/include/notes.h](firmware/include/notes.h) | note frequency table |
| [emulation/lemon-piano.yaml](emulation/lemon-piano.yaml) | circuit spec (source of truth) |
| [emulation/lemon-piano.vlx](emulation/lemon-piano.vlx) | generated project — import into Velxio and Run |
| [HARDWARE.md](HARDWARE.md) | pin map, BOM, wiring detail |
| [images/wiring-v5.png](images/wiring-v5.png) | wirewright-rendered wiring |

**Next revision:** none yet — this is the newest board. Modify the hardware and
you start V6: the recipe is in
[../../docs/VERSIONING.md](../../docs/VERSIONING.md).
