# V1 hardware — banana piano rig

Reconstructed from the sketch
([firmware/banana-piano/banana-piano.ino](firmware/banana-piano/banana-piano.ino))
and the tutorial's breadboard picture
([images/banana-piano-original.png](images/banana-piano-original.png)).
⚠️ marks what the code alone cannot settle.

## Pin map

| Pin | Direction | Role |
|---|---|---|
| A0–A6 | analog in | 7 fruit touch keys, each with a **220 Ω pull-up to +5 V** |
| D8 | out | speaker / passive buzzer (`tone()`, one note per key) |
| D11 | in | HC-SR04 **ECHO** — `pulseIn()` in the commented-out demo block |
| D12 | out | HC-SR04 **TRIGGER** — `digitalWrite()` in the same commented block |
| D13 | out | `pinMode(OUTPUT)` at boot; drives the board's **on-board LED** only |
| 5V / GND | — | rails; the player holds a clip on **GND** |

### Board note

The sketch reads `analogRead(6)`, and **A6 exists only on the TQFP ATmega328P**
(Nano / Pro Mini / Micro) — the Uno's DIP package has no A6 pin. The tutorial
rig in the picture is an Uno, so on the original board key 7 could not have
worked as written. Both are kept as PlatformIO envs (`nanoatmega328` builds the
complete keyboard, `uno` builds the historical board). The diagram uses the
Nano-style pinout so all six revisions can be compared pin-for-pin.

## Bill of materials

| Component | Qty | Notes |
|---|---|---|
| Arduino Uno (2019) / Nano | 1 | Nano needed for key 7 (A6) |
| Fruit (bananas) + alligator clips | 7 + 8 | 7 keys + 1 hand-held GND clip |
| 220 Ω resistors | 7 | one per key, **pin → +5 V** (pull-up, not in series) |
| Speaker or passive buzzer | 1 | D8 |
| HC-SR04 ultrasonic module | 1 | mounted; its code is commented out |
| Breadboard + jumpers | 1 | |

## Key wiring (the 2019 polarity)

```
 +5V ──[220 Ω]──┬── A(n)                     ← pin idles ≈ 1023
                └── clip ── 🍌 fruit ── player ── [hand-held clip] ── GND
                                                  ↑ touching drags A(n) DOWN
```

- **Idle:** the pull-up holds the pin at ~1023.
- **Touched:** the body (≈ hundreds of kΩ) forms a divider with the 220 Ω, so
  the pin drops only a few counts — hence the `<= 1019` threshold and the
  2-sample average.
- ⚠️ The tutorial picture shows the resistor row going to the top rail and the
  black "clip" wires landing on the pin columns; which rail the hand-held clip
  used is inferred from the code's inverted logic (it must be GND for a touch to
  pull the pin down).

V4 flipped this: the clip moved to **+5 V**, pins float near 0, and a touch
*raises* the reading. That inversion is the single biggest electrical difference
between the 2019 and 2026 boards —
[../../docs/HARDWARE.md](../../docs/HARDWARE.md) has the comparison table.

## Diagram

| File | What it shows |
|---|---|
| [images/wiring-v1.png](images/wiring-v1.png) | full V1 wiring: 7 keys + pull-up comb, speaker, HC-SR04, GND clip |
| [images/banana-piano-original.png](images/banana-piano-original.png) | the untitled.es original breadboard (Uno + speaker, clips as keys) |

`wiring-v1.png` is generated — never hand-drawn — by
[../../tools/wiring_diagrams.py](../../tools/wiring_diagrams.py) (`build_v1`) on
the wirewright engine: `python3 tools/wiring_diagrams.py v1`.
