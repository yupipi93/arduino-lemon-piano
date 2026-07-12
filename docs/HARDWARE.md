# Hardware — deduced schematic

No schematic for the final V4 build survived, so this document reconstructs it
from the code ([firmware/src/main.cpp](../firmware/src/main.cpp)), the Fritzing
diagram of the keyboard stage, and the original banana-piano tutorial it
evolved from. Confidence notes are marked ⚠️ where the code alone can't settle
the wiring.

## Pin map (Arduino Nano, V4)

| Pin | Direction | Role |
|---|---|---|
| A0–A6 | analog in | 7 lemon keys, left → right |
| D2 | out | **Red LED** — wrong note |
| D3 | out | **Green LED** — correct note |
| D4 | in | **Game select** button — HIGH at boot ⇒ game 1 (Mario Main), else game 2 (Underworld) |
| D5 | out | **Relay 1** — water pump pair |
| D6 | out | **Relay 2** — water pump pair |
| D7 | in | **Restart** button — re-runs game selection |
| D8 | out | **Passive buzzer** (`tone()` + bit-banged `buzz()`) |
| D13 | out | Onboard LED — lit while a melody note is bit-banged |
| 5V | — | hand-held alligator clip (the player closes the circuit) |

⚠️ `setup()` also does `pinMode(0..7, INPUT)` on the *digital* pins 0–7 — a
2019 confusion between analog and digital numbering (D0/D1 are the UART).
Harmless for the analog reads, but it's on the fix list.

## How the touch sensing works

<div align="center">
<img src="images/keyboard-breadboard-nano.png" alt="V4 keyboard stage" width="80%"/>
</div>

Each key is a voltage divider closed by the player's body:

```
 5V ──[hand-held clip]── player ── 🍋 lemon ──[clip]──[220 Ω]── A0..A6
                                                            (pin otherwise floating)
```

- **Idle:** the analog pin floats and reads near 0 (just mains-coupled noise).
- **Touched:** 5 V → hand → body → lemon → pin. The lemon + skin resistance
  drops most of the voltage, but the reading rises well above the threshold.

```cpp
const int SENSITIVITY = 100;   // ~0.5 V. Raise if it sounds without touching
                               // (170 for a laptop running on its charger)
if (analogRead(i) > SENSITIVITY) { /* key i touched */ }
```

That comment about laptop chargers is the tell that the pins float: switching
power supplies raise the baseline noise on a floating input. ⚠️ There are no
pull-down resistors in the code's model; if ghost notes appear, add ~1 MΩ from
each analog pin to GND and raise `SENSITIVITY`.

### Threshold inversion vs. the original banana piano

The 2019 evolution flipped the sensing logic — useful to know when reading the
archive:

| | Original banana piano / v3 | V4 (current) |
|---|---|---|
| Idle reading | ~1023 (pin biased toward 5 V) | ~0 (pin floating) |
| Touch detected | averaged reading `<= 1019` (drops) | reading `> 100` (rises) |
| Player's clip | GND | 5 V |
| Sampling | 2–4× averaged `analogRead` | single `analogRead` |

## The water-pump penalty circuit

Failing at note index ≥ 7 runs, for 1 second:

```cpp
digitalWrite(RELAY_1, HIGH);   digitalWrite(RELAY_2, LOW);    // pump ON
tone(BUZZER, NOTE_D1); delay(1000); noTone(BUZZER);
digitalWrite(RELAY_1, LOW);    digitalWrite(RELAY_2, HIGH);   // pump OFF
```

⚠️ Deductions, to verify against the physical build:

- Two channels driven complementarily suggests a **2-channel relay module
  wired as an H-bridge-style reversal, or one active-HIGH + one active-LOW
  channel in series with the pump** — the code alone can't distinguish.
  Most likely: a standard active-LOW 2-relay module where `RELAY_2 = LOW`
  energizes the pump.
- At boot the relays are "initialized" with `analogWrite(RELAY_1, HIGH);
  analogWrite(RELAY_2, HIGH)` — `HIGH` is 1, so this is a 1/255 PWM duty on
  pins 5/6, i.e. *almost* LOW, not a clean off-state. On an active-LOW module
  this would actually chatter the pump at boot. On the fix list (TODO.md #2).

## Buzzer

Two sound paths coexist:

- `tone()` — key notes, the penalty groan, and the death melody.
- `buzz()` — a bit-banged square wave (with the D13 LED as note indicator),
  used by `sing()` for the Mario/Underworld melodies. Blocking (`delay()`
  between notes), so the game is unresponsive while a melody plays.

## Reference diagrams

| File | What it shows |
|---|---|
| [images/keyboard-breadboard-nano.png](images/keyboard-breadboard-nano.png) | V4 keyboard stage: Nano, buzzer, 7×220 Ω, 7 "fruit" buttons on A0–A6 |
| [keyboard-schematic.fzz](keyboard-schematic.fzz) | Editable Fritzing source of the above |
| [images/banana-piano-original.png](images/banana-piano-original.png) | untitled.es original: Uno + speaker, clips as keys, hand-held clip note |

Missing from both diagrams (deduced from code only): LEDs (D2/D3), buttons
(D4/D7), relay module (D5/D6), water pump. A complete Fritzing/KiCad redraw is
on the roadmap (TODO.md #12).
