# Hardware — deduced schematic

This documents the **active V5** build (ten-LED progress bar). The keyboard
stage is unchanged from 2019; the feedback/actuator stage was reworked (V4's
relays + red LED gone, ten green LEDs in). The touch keyboard is reconstructed
from the code ([firmware/src/main.cpp](../firmware/src/main.cpp)), the surviving
Fritzing diagram, and the banana-piano tutorial it evolved from. Confidence
notes are marked ⚠️ where the code alone can't settle the wiring.

> V4's pin map (single red/green LED + relay-driven water pump) is preserved in
> [`../archive/lemon-piano-v4/`](../archive/lemon-piano-v4/).

## Pin map (Arduino Nano, V5)

| Pin | Direction | Role |
|---|---|---|
| A0–A6 | analog in | 7 lemon keys, left → right (touch sensing) |
| A7 | analog in | **Game select** switch — 5 V ⇒ game 1 (Mario Main), GND ⇒ game 2 (Underworld). Read at boot/restart; sets the *starting* game |
| D2, D3, D4, D5, D6 | out | **Green LEDs 1–5** of the progress bar |
| D7 | in | **Restart** button (active-HIGH) — re-reads game select, blanks the bar |
| D8 | out | **Passive buzzer** (`tone()` + bit-banged `buzz()`) |
| D9, D10, D11, D12, D13 | out | **Green LEDs 6–10** of the progress bar |
| 5V | — | hand-held alligator clip (the player closes the circuit) |

Ten LEDs need ten output pins, which is why game-select moved off a digital pin
to **A7**. A6 and A7 are analog-input-only on the Nano (no digital buffer, no
internal pull-up), so:

- **A6** works fine as key 7 (it's an analog read anyway).
- **A7** must be actively driven — wire an SPDT switch between 5 V and GND with
  the wiper to A7, or a switch to 5 V plus a 10 kΩ pulldown to GND. The
  firmware reads `analogRead(A7) > 512`.

This uses every I/O line on the Nano: A0–A7 (8) + D2–D13 (12) = 20, with D0/D1
left for the UART. A classic **Uno has no A6/A7**, so V5 needs a Nano/Mini.

## How the touch sensing works

<div align="center">
<img src="images/keyboard-breadboard-nano.png" alt="Keyboard stage" width="80%"/>
</div>

Each key is a voltage divider closed by the player's body:

```
 5V ──[hand-held clip]── player ── 🍋 lemon ──[clip]──[220 Ω]── A0..A6
                                                            (pin otherwise floating)
```

- **Idle:** the analog pin floats and reads near 0 (just mains-coupled noise).
- **Touched:** 5 V → hand → body → lemon → pin. The lemon + skin resistance
  drops most of the voltage, but the reading rises well above the threshold.

V5 (like V4 since the fixes) **auto-calibrates**: at boot `calibrate()` samples
each key's floating baseline and sets `threshold = baseline + TOUCH_MARGIN`
(100), so the old "raise SENSITIVITY to 170 on a laptop charger" tweak is
automatic. Keep hands off the lemons during boot.

```cpp
if (analogRead(i) > keyThreshold[i]) { /* key i touched */ }
```

⚠️ There are no pull-down resistors in the code's model; if ghost notes appear,
add ~1 MΩ from each analog pin to GND (calibration then measures a cleaner
baseline).

### Threshold inversion vs. the original banana piano

The 2019 evolution flipped the sensing logic — useful to know when reading the
archive:

| | Original banana piano / v3 | V4 / V5 (current) |
|---|---|---|
| Idle reading | ~1023 (pin biased toward 5 V) | ~0 (pin floating) |
| Touch detected | averaged reading `<= 1019` (drops) | reading rises above the calibrated baseline |
| Player's clip | GND | 5 V |
| Sampling | 2–4× averaged `analogRead` | single `analogRead` (edge-triggered) |

## The ten-LED progress bar (V5)

Ten green LEDs, `LED_PINS[0..9] = {D2,D3,D4,D5,D6,D9,D10,D11,D12,D13}`, each with
its own 220 Ω series resistor to a common GND:

```
 Dn ──[220 Ω]──▶|── GND        (LED n, anode to the pin)
```

- Each **correct** note lights `LED_PINS[currentStep]` and advances the step.
- A **wrong** note calls `allLedsOff()` (blanks all ten) + a short low tone, and
  resets to the first note.
- **All ten lit = win**: the victory theme plays with the bar fully lit, then
  the game auto-advances to the other theme and the bar blanks for the next
  round.

Standard indicator LEDs (Vf ≈ 2 V): 220 Ω gives ~15 mA at 5 V, within the
ATmega's per-pin limit. Total draw if all ten are lit ≈ 150 mA — fine on USB.

> **V4 only (archived):** V4 had a single red + green LED and a **relay-driven
> water pump** that fired on a late miss (`RELAY_1`/`RELAY_2` on D5/D6, 1 s
> spray, 10 penalties → death tune). All of that is gone in V5. The wiring and
> ⚠️ polarity deductions live in
> [`../archive/lemon-piano-v4/`](../archive/lemon-piano-v4/).

## Buzzer

- `tone()` — key notes and the short low "wrong" tone (non-blocking).
- `buzz()` — a bit-banged square wave used by `playSong()` for the
  Mario/Underworld victory themes. Blocking (`delay()` between notes), so the
  game pauses (with the bar fully lit) while the winning theme plays — this is
  intentional. In the Velxio build both paths route through `emuTone()` on D11
  (see [../emulation/README.md](../emulation/README.md)).

## Reference diagrams

| File | What it shows |
|---|---|
| [images/keyboard-breadboard-nano.png](images/keyboard-breadboard-nano.png) | Keyboard stage (still current): Nano, buzzer, 7×220 Ω, 7 "fruit" buttons on A0–A6 |
| [keyboard-schematic.fzz](keyboard-schematic.fzz) | Editable Fritzing source of the above |
| [images/banana-piano-original.png](images/banana-piano-original.png) | untitled.es original: Uno + speaker, clips as keys, hand-held clip note |

The surviving diagrams cover only the keyboard stage. The **ten-LED bar**,
game-select on A7, and restart button are documented here from the code; a
complete Fritzing/KiCad redraw is on the roadmap ([../TODO.md](../TODO.md)).
For a working, fully-wired reference of V5 today, the
[Velxio emulation](../emulation/lemon-piano.yaml) spec is the closest thing to a
complete schematic (10 LEDs + 7 keys + buzzer, verified end-to-end).
