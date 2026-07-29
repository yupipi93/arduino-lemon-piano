# Hardware fundamentals (shared across versions)

The parts of the build that do **not** change from board to board: how a piece of
fruit becomes a key, the one polarity flip in the project's history, and the
shared reference images. Per-board pin maps and BOMs live with their version:

| Version | Hardware doc |
|---|---|
| V0 — buzzer rig | [../versions/v0-buzzer/HARDWARE.md](../versions/v0-buzzer/HARDWARE.md) |
| V1 — banana piano | [../versions/v1-banana-piano/HARDWARE.md](../versions/v1-banana-piano/HARDWARE.md) |
| V2 — keyboard test | [../versions/v2-keyboard-test/HARDWARE.md](../versions/v2-keyboard-test/HARDWARE.md) |
| V2.5 — live threshold | [../versions/v2.5-threshold-buttons/HARDWARE.md](../versions/v2.5-threshold-buttons/HARDWARE.md) |
| V3 — game prototype | [../versions/v3-game-prototype/HARDWARE.md](../versions/v3-game-prototype/HARDWARE.md) |
| V4 — water pump | [../versions/v4-water-pump/HARDWARE.md](../versions/v4-water-pump/HARDWARE.md) |
| V4.5 — margin buttons | [../versions/v4.5-margin-buttons/HARDWARE.md](../versions/v4.5-margin-buttons/HARDWARE.md) |
| V5 — LED bar | [../versions/v5-led-bar/HARDWARE.md](../versions/v5-led-bar/HARDWARE.md) |

## How the touch sensing works

<div align="center">
<img src="images/keyboard-breadboard-nano.png" alt="Keyboard stage" width="80%"/>
</div>

Each key is a voltage divider that the player's body closes. The fruit is not a
sensor — it is a wet, conductive lump that couples your skin to an analog pin. The
body's resistance (hundreds of kΩ) sits in series with a 220 Ω resistor, and the
firmware just watches the pin move.

**2026 boards (V4, V4.5, V5) — the clip is on +5 V:**

```
 5V ──[hand-held clip]── player ── 🍋 lemon ──[clip]──[220 Ω]── A0..A6
                                                            (pin otherwise floating)
```

- **Idle:** the pin floats and reads near 0 (just mains-coupled noise).
- **Touched:** 5 V → hand → body → lemon → pin; the reading **rises** above a
  calibrated threshold.

```cpp
if (analogRead(i) > keyThreshold[i]) { /* key i touched */ }
```

**2019 boards (V1, V2, V3) — the clip is on GND:**

```
 +5V ──[220 Ω]──┬── A(n)                  ← pull-up holds the pin at ≈1023
                └── clip ── 🍌 fruit ── player ──[hand-held clip]── GND
```

- **Idle:** ≈1023.
- **Touched:** the body drags the pin **down** — but only by a few counts, because
  220 Ω is tiny next to skin resistance. Hence the famously odd threshold:

```cpp
if (((analogRead(n) + analogRead(n)) / 2) <= 1019) { /* key n touched */ }
```

### The polarity flip (V3 → V4)

The single biggest electrical change in the project's history — useful to keep
straight when reading the older sketches:

| | V1 / V2 / V3 (2019) | V4 / V4.5 / V5 (2026) |
|---|---|---|
| Player's clip | **GND** | **+5 V** |
| 220 Ω resistor | pull-up, pin → +5 V | in series, lemon → pin |
| Idle reading | ≈1023 (pin biased high) | ≈0 (pin floating) |
| Touch detected | averaged reading drops `<= 1019` | reading rises above the calibrated baseline |
| Sampling | 2–4× averaged `analogRead` | single `analogRead`, edge-triggered |
| Threshold | hardcoded constant | auto-calibrated (V4) → noise-adaptive (V4.5, V5) |

### Calibration (2026 boards)

- **V4:** `threshold = baseline + 100`, sampled once at boot.
- **V4.5 / V5:** `threshold = baseline + max(40, 3 × measured noise)`, capped at
  900, re-sampled at boot **and on every RESTART**. V4.5 adds two buttons that
  nudge a manual offset on top (±10 per press).

The relay-driven water pump exists on **V3** (wired, never fired) and **V4** only;
V4.5 removed it, so from V4.5 on a late miss is punished with a low groan instead
of a spray.

Keep hands off the fruit while calibration runs — it is measuring the *idle*
level. ⚠️ No pull-downs exist in any version's model; if ghost notes appear, add
~1 MΩ from each analog pin to GND so calibration sees a cleaner baseline.

## Parts common to every version

| Component | Notes |
|---|---|
| ATmega328P board | Uno on the 2019 rigs; Nano from V4 (key 7 needs A6, V5 also needs A7) |
| Fruit + alligator clips | 7 keys + 1 hand-held clip. Lemons from V4; bananas before that |
| 220 Ω resistors | one per key in every version (pull-up in 2019, series from V4) |
| Passive buzzer / speaker | always on **D8** on hardware (**D11** in the browser builds — see any version's `emulation/README.md`) |
| Breadboard + jumpers | |

**A6/A7 caveat.** Every version's sketch reads `analogRead(6)` for key 7, and A6
exists only on the TQFP ATmega328P (Nano / Pro Mini) — the Uno's DIP package has
no A6/A7 pins at all. So the 2019 rigs, which were built on an Uno, could not have
had a working key 7 as written; V5 additionally needs A7 for game select.

## Buzzer, both code paths

Every version drives the buzzer two ways, and both matter when reading the code:

- `tone()` — key notes and short feedback tones, non-blocking.
- `buzz()` — a bit-banged square wave used by the melody player, blocking
  (`delay()` between notes) and blinking the on-board LED (D13) per note. The
  game is *meant* to pause while a tune plays.

In the Velxio browser builds both paths route through `emuTone()` on D11, because
Velxio's buzzer part only polls Timer2 duty on PWM pins and only stops a note on a
duty→0 event. On D8 the first note would beep forever.

**Debugging the buzzer:** flash [V0](../versions/v0-buzzer/) — one buzzer on D8,
a scale forever, both playback paths available as separate builds, and the on-board
LED lit per note so you can see the firmware running even when you hear nothing.
Its [HARDWARE.md](../versions/v0-buzzer/HARDWARE.md) has the ordered checklist
(passive vs active buzzer, polarity, `tone()` vs `buzz()`, accidental emulation
build on D11).

## The music

Every melody and sound effect the project plays — the four level themes, the coin,
power-up, 1-up, death, flagpole fanfare and ending melody — is catalogued with its
note data and its **provenance** in [MARIO-SOUNDS.md](MARIO-SOUNDS.md). Sounds are
tagged there as sourced verbatim, transcribed from a cited tab, or reconstructed
from a description, because the accuracy genuinely differs between them.

## Shared reference images

| File | What it shows |
|---|---|
| [images/keyboard-breadboard-nano.png](images/keyboard-breadboard-nano.png) | the surviving Fritzing keyboard stage: Nano, buzzer, 7 × 220 Ω, 7 "fruit" buttons on A0–A6 — unchanged since 2019 |
| [keyboard-schematic.fzz](keyboard-schematic.fzz) | editable Fritzing source of the above |
| [../versions/v1-banana-piano/images/banana-piano-original.png](../versions/v1-banana-piano/images/banana-piano-original.png) | the untitled.es tutorial's own breadboard picture (V1) |

Per-version wiring diagrams (`versions/*/images/wiring-*.png`) are all generated
by [../tools/wiring_diagrams.py](../tools/wiring_diagrams.py) on the wirewright
engine — one contract per board, DRC-validated, never hand-drawn. A formal
Fritzing/KiCad redraw of a full board is still optional ([../TODO.md](../TODO.md)
#14).
