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

| Pin | Direction | Role |
|---|---|---|
| A0–A6 | analog in | 7 lemon keys, left → right, 220 Ω in series (touch sensing) |
| A7 | analog in | **GAME SELECT** — 5 V ⇒ game 1 (Mario Main), GND ⇒ game 2 (Underworld). Read at boot/restart; sets the *starting* game |
| D2, D3, D4, D5, D6 | out | **green LEDs 1–5** of the progress bar |
| D7 | in | **RESTART** button (active-HIGH) — re-reads game select, recalibrates, blanks the bar |
| D8 | out | **passive buzzer** (`tone()` + bit-banged `buzz()`) |
| D9, D10, D11, D12, D13 | out | **green LEDs 6–10** of the progress bar |
| 5V | — | hand-held alligator clip (the player closes the circuit) |

This uses **every I/O line** on the Nano: A0–A7 (8) + D2–D13 (12) = 20, with
D0/D1 left for the UART.

### Why game select moved to A7

Ten LEDs need ten output pins, so game select had to leave the digital block. A6
and A7 are **analog-input-only** on the Nano (no digital buffer, no internal
pull-up), so:

- **A6** is fine as key 7 — it is an analog read anyway.
- **A7** must be actively driven: an SPDT switch between 5 V and GND with the
  wiper on A7, or a switch to 5 V plus a 10 kΩ pulldown. The firmware reads
  `analogRead(A7) > 512`. ⚠️ Unconfirmed on real hardware (TODO #15).

A classic **Uno has no A6/A7**, which is why V5 dropped the `uno` build env.

## Bill of materials

| Component | Qty | Notes |
|---|---|---|
| **Arduino Nano** (ATmega328P) | 1 | needs A6 **and** A7 → an Uno won't do |
| Lemons 🍋 + alligator clips | 7 + 8 | 7 keys on A0–A6 + 1 hand-held 5 V clip |
| **Green LEDs** | **10** | the progress bar |
| 220 Ω resistors | 7 + 10 | 7 key series resistors + one per LED |
| Passive buzzer | 1 | D8 |
| SPDT switch | 1 | GAME SELECT on A7 (or a switch to 5 V + 10 kΩ pulldown) |
| Push button | 1 | RESTART, D7, to +5 V with a 10 kΩ pulldown |
| Breadboard + jumpers | 1 | |

## The ten-LED progress bar

`LED_PINS[0..9] = {D2,D3,D4,D5,D6,D9,D10,D11,D12,D13}`, each with its own 220 Ω
series resistor to a common GND:

```
 Dn ──[220 Ω]──▶|── GND        (LED n, anode to the pin)
```

- While the bar is empty (`currentStep == 0`) the game is in **free play**: keys
  sound but nothing is scored or punished.
- A touch **sustains**: `startKeyTone()` calls `tone()` with no duration, so the
  note runs until the lemon is released (`stopKeyTone()`), with `NOTE_DURATION`
  70 ms as a floor for quick taps. Measured on the emulator: a 600 ms hold gives
  a 584 ms tone.
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
| [images/wiring-v5.png](images/wiring-v5.png) | **full V5 wiring** — 7 keys, ten-LED bar, A7 game-select, restart, buzzer |
| [emulation/lemon-piano.yaml](emulation/lemon-piano.yaml) | the verified browser circuit (10 LEDs + 7 keys + buzzer) |
| [../../docs/images/keyboard-breadboard-nano.png](../../docs/images/keyboard-breadboard-nano.png) | shared Fritzing keyboard stage (still current) |

`wiring-v5.png` is generated by
[../../tools/wiring_diagrams.py](../../tools/wiring_diagrams.py) (`build_v5`) on
the wirewright engine — `python3 tools/wiring_diagrams.py v5`, DRC-validated,
never hand-drawn. A formal Fritzing/KiCad redraw of the whole board is still
optional ([../../TODO.md](../../TODO.md) #14).
