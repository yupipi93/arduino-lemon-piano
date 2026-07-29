# V5 hardware — ten-LED progress bar

The keyboard stage is unchanged since 2019 (7 keys, 220 Ω, +5 V clip since V4);
the feedback stage was rebuilt: V4.5's red LED and MARGIN buttons came off (the
relays and pump had already gone with V4.5), and ten green LEDs went on. Reconstructed from
[firmware/src/main.cpp](firmware/src/main.cpp) and verified end-to-end against the
[Velxio spec](emulation/lemon-piano.yaml). ⚠️ marks what the code alone cannot
settle.

> **Build status:** this board is **not physically wired yet** (TODO #13). The
> firmware builds, and the emulation is verified green — that emulation is
> currently the most trustworthy reference for wiring it.

## Pin map (Arduino Nano)

Rewired on 2026-07-28 for buildability: **the bar is one unbroken ascending run,
LED n on pin n+1**, so you can wire it left to right without looking anything up.

| Pin | Direction | Role |
|---|---|---|
| A0–A6 | analog in | 7 lemon keys, each with a **220 Ω pull-up to +5 V**; a touch drags the pin **down** |
| **D2 … D11** | out | **green LEDs 1 … 10** — LED n on pin n+1, in order |
| **D12** | in | **SENS +** button (more sensitive) — to GND on the internal pull-up |
| **D13** | out | **passive buzzer** — key notes, victory themes and UI chirps |
| A7 | analog in | **SENS −** button (less sensitive) — to GND with an **external 10 kΩ pull-up** (A7 has no internal one); read as `analogRead(A7) < 512` |
| GND | — | hand-held alligator clip (**the player holds GND**) |

### Why the buzzer is on D13 and the button on D12

D13 carries the Nano's **on-board LED through ~1 kΩ**. That load fights a ~30 kΩ
internal pull-up, so a button there can read as *permanently pressed*. A buzzer
does not care about the extra load — it just gets a free blink-with-the-audio
indicator — so D13 is the buzzer's pin and D12 takes the button.

Do not put a button on D0/D1 either: those are the UART, i.e. the serial monitor
this rig is tuned with and the pins avrdude uses to upload. The spare I/O on this
board is the analog side (A7), not the UART.

This uses **every I/O line** on the Nano: A0–A7 (8) + D2–D13 (12) = 20, with
D0/D1 left for the UART.

### Why one button had to be an analog pin

Ten LEDs plus the buzzer take eleven of the twelve usable digital lines (D2–D13),
leaving exactly **one** — so the second button had to go on **A7**. A6 and A7 are
analog-input-only on the Nano (no digital buffer, **no internal pull-up**), so:

- **A6** is fine as key 7 — it is an analog read anyway;
- **A7** works as a button only with an **external 10 kΩ pull-up** to +5 V and the
  button pulling it to GND, read with `analogRead(A7) < 512`. It is the one part on
  this board beyond keys, LEDs, resistors and buttons.
- **D12** needs nothing: `INPUT_PULLUP` plus a button to GND.

A classic **Uno has no A6/A7**, which is why V5 has no `uno` build env.

## Bill of materials

| Component | Qty | Notes |
|---|---|---|
| **Arduino Nano** (ATmega328P) | 1 | needs A6 **and** A7 → an Uno won't do |
| Lemons 🍋 + alligator clips | 7 + 8 | 7 keys on A0–A6 + 1 hand-held **GND** clip |
| **Green LEDs** | **10** | the progress bar |
| 220 Ω resistors | 7 + 10 | 7 key **pull-ups** (pin → +5 V) + one per LED |
| 10 kΩ resistor | 1 | pull-up for the SENS − button on A7 |
| Passive buzzer | 1 | D8 |
| Push buttons | **2** | SENS + on D7 (to GND) · SENS − on A7 (to GND) |
| Breadboard + jumpers | 1 | |

## The ten-LED progress bar

`LED_PINS[0..9] = {D2,D3,D4,D5,D6,D7,D8,D9,D10,D11}` — LED n on pin n+1 — each
with its own 220 Ω series resistor to a common GND:

```
 Dn ──[220 Ω]──▶|── GND        (LED n, anode to the pin)
```

- While the bar is empty (`currentStep == 0`) the game is in **free play**: keys
  sound but nothing is scored or punished.
- A touch **sustains**: `startKeyTone()` calls `tone()` with no duration, so the
  note runs until the lemon is released (`stopKeyTone()`), with `NOTE_DURATION`
  70 ms as a floor for quick taps. Measured on the emulator: a 600 ms hold gives
  a 584 ms tone.
- **One key at a time, strongest wins.** `strongestKey()` picks the channel with
  the largest margin over its own threshold rather than the first one above it, and
  while that key is down every other channel is ignored until it is released
  (`keyStillDown()`, with `TOUCH_HYSTERESIS` 60 counts of Schmitt hysteresis so a
  borderline reading cannot chop the note up or re-trigger guesses). This is what
  stops coupled channels from turning one finger into two guesses — see the
  measured evidence above for why coupling happens at all.
- Only the **first press of a key** reaches the game (`lastCountedKey`): holding
  or re-tapping the same lemon sounds but never scores or punishes again until a
  different key is played. This is what makes flaky fruit contact harmless — and
  it means a secret code must never repeat a note back-to-back.
- Each **correct** note lights `LED_PINS[currentStep]` and advances the step.
- Once the sequence has started, a **wrong** note calls `allLedsOff()` (blanks all
  ten) and plays a short low tone — **after** the pressed note has finished
  (`NOTE_DURATION` 70 ms, then a `WRONG_TONE_GAP_MS` 60 ms silence, then
  `WRONG_TONE_MS` 200 ms of C2). Measured on the emulator: 69.9 ms note →
  60.5 ms silence → 200.0 ms tone, no overlap. With sustain, "finished" means the
  lemon was released — capped by `SUSTAIN_CAP_MS` (2 s) so a stuck or ghosting
  key cannot freeze the game.
- **All ten lit = win:** the victory theme plays with the bar flashing per note,
  then the game auto-advances to the other theme and the bar blanks.

Standard indicator LEDs (Vf ≈ 2 V): 220 Ω gives ~15 mA at 5 V, within the
ATmega's per-pin limit. All ten lit ≈ 150 mA — fine on USB, but note the
ATmega328P's 200 mA total-I/O ceiling leaves little headroom for anything else.

## Why the keyboard is pulled UP again (measured 2026-07-27 / 28)

This board went back to the 2019 arrangement — **220 Ω pull-up per key, player on
a GND clip** — because the floating +5 V-clip keyboard could not be read at all.
Not "read badly": measured on the real board with a bench sampler (7 channels,
20 Hz, four held touches of 1.9–32.5 s):

| Measure | Result |
|---|---|
| Samples with all 7 channels above 400 | 50 % |
| Samples with zero channels above 400 | 38 % |
| Samples pinned at 1023 / at 0 | 27 % / 23 % |
| **Highest vs second-highest channel** | **median 5 counts** |
| Channel reading highest, whichever lemon was touched | A0 or A6 in 92 % of samples |

Five counts of spread across seven channels means the ADC is not resolving seven
different voltages, and "A0 or A6 wins" is just the first/last position in the mux
scan. Readings came in gradient ramps (`148 194 258 332 400 469 545`): the
sample-and-hold capacitor never charges to the pin's voltage because the source
impedance is effectively open, so each reading is mostly the previous channel's
residue plus mains pickup through the player's body. The idle level also drifts
~170 counts on a ~25 s cycle, which is ~75 % of the touch margin `calibrate()`
computes at boot — so boot-time thresholds go stale within seconds.

A 220 Ω pull-up per pin fixes all of it: the ADC gets a source impedance it can
sample, the idle level is anchored near the rail, the mains pickup collapses, and
each key becomes a real divider. Measured after the rewire, on this board:

```
  key 1 baseline=1022 noise=1 -> threshold=1018
  key 7 baseline=1022 noise=1 -> threshold=1018
auto margin=4  (worst noise 1 x 2, floor 4)
```

**Idle 1022 with 1 count of noise, on every channel** — against ~250 with 76–104
counts of noise and 170 counts of drift before. The trade-off is that the signal is
small: a touch dips the reading only **3–4 counts** (skin ≈ 1 MΩ against 220 Ω),
which is exactly where the 2019 sketch's `<= 1019` came from. Hence a margin
measured in single counts, and the sensitivity buttons.

If you want a *large* signal instead of a merely reliable one, a much weaker
pull-up (~1 MΩ) would turn those 4 counts into hundreds — at the cost of a slower,
higher-impedance node.

## Touch sensing

Unchanged from [V4.5](../v4.5-margin-buttons/HARDWARE.md): pins float near 0,
the +5 V clip through the body raises the reading,
`threshold = baseline + max(40, 3 × noise)` re-measured at boot and on every
RESTART. There are no MARGIN buttons on this board — those pins are LEDs. The
physics and the 2019-vs-2026 polarity table are in
[../../docs/HARDWARE.md](../../docs/HARDWARE.md).

## Buzzer

- `tone()` — key notes and the short low "wrong" tone (non-blocking).
- `buzz()` — bit-banged square wave used by `playSong()` for the victory themes.
  Blocking (`delay()` between notes), so the game pauses (bar lit) while the
  winning theme plays — intentional.
- In the Velxio build both paths route through `emuTone()` on **D11**; see
  [emulation/README.md](emulation/README.md).

## Diagrams

| File | What it shows |
|---|---|
| [images/wiring-v5.png](images/wiring-v5.png) | **full V5 wiring** — 7 pulled-up keys + GND clip, the D2–D11 LED run, both sensitivity buttons, buzzer on D13 |
| [emulation/lemon-piano.yaml](emulation/lemon-piano.yaml) | the verified browser circuit (10 LEDs + 7 keys + buzzer) |
| [../../docs/images/keyboard-breadboard-nano.png](../../docs/images/keyboard-breadboard-nano.png) | shared Fritzing keyboard stage (still current) |

`wiring-v5.png` is generated by
[../../tools/wiring_diagrams.py](../../tools/wiring_diagrams.py) (`build_v5`) on
the wirewright engine — `python3 tools/wiring_diagrams.py v5`, DRC-validated,
never hand-drawn. A formal Fritzing/KiCad redraw of the whole board is still
optional ([../../TODO.md](../../TODO.md) #14).
