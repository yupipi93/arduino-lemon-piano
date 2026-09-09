# V5.5 — filtered 5 V supply + amplified speaker (2026) — newest board

The V5 board plays exactly as before — this version exists because of how it was
**powered**. Fed raw USB from a PC or a wall wart, flipping a light switch
anywhere in the house made the piano play by itself, and on a socket shared with
many other devices it went haywire. That is not a software bug: the V5 keyboard's
touch margin is **3–4 ADC counts ≈ 15–20 mV** ([why](../v5-led-bar/HARDWARE.md)),
so any conducted transient bigger than that *is indistinguishable from a key
press*. V5.5 puts a **transient clamp + LC pi filter** between the 5 V source and
the board, so what reaches the rail — which is also the ADC's reference — is
clean.

**Hardware delta vs [V5](../v5-led-bar/):**

- **on** the board: a **power-entry filter** feeding the +5 V rail —
  **P6KE6.8A** TVS across the input, **1N5817** Schottky in series, then a
  **CLC pi filter**: 470 µF ‖ 100 nF → **100 µH** power choke → 470 µF ‖ 100 nF
  (fc ≈ 700 Hz, 2nd order);
- also on the board: an **amplified speaker** on the D13 sound line — a
  10 kΩ / 1 kΩ divider (≈ ÷11) → 1 µF DC block → **LM386** module → **4 Ω 3 W**
  speaker, with the on-board piezo left in parallel on D13. The amp is fed from
  the **unfiltered** 5 V (tapped before the TVS): its audio-rate current must not
  cross the filter, which is also AVcc, the ADC reference the 3–4-count touch
  margin rides on. That is why the `+5 V` rail in the diagram stops short of the
  amp block;
- the keyboard, LED bar and SENS ± buttons are byte-for-byte V5. **The firmware
  was too, until 2026-09-09**, when V5.5 gained FREE PLAY and the MODE WHEEL
  (below). V5 itself is unchanged and still builds — the two have simply
  diverged, which is what separate version directories are for;
- powering rule that comes with it: feed the filter from a **USB wall charger or
  bench supply**, not from a PC loaded with other devices, and don't power the
  board over its mini-USB in normal play (that connector bypasses the filter —
  it stays for flashing only; the Schottky keeps it from back-feeding the filter).

<div align="center">
<img src="images/wiring-v5.5.png" alt="V5.5 wiring diagram" width="92%"/>
</div>

## Why a light switch played the piano

Three coupling paths deliver a mains transient to the keyboard, in order of
strength when powered from a shared/noisy source:

1. **Conducted** — the spike rides the 5 V supply straight onto the rail. The
   rail tops the seven 220 Ω pull-ups *and* is AVcc, the ADC reference. The DC
   levels are ratiometric (a slow sag cancels out of the comparison), but a fast
   edge is not: the pin node and the reference don't move together, and a
   >20 mV difference for a few milliseconds is a phantom note. **This is the
   path V5.5 kills.**
2. **Common-mode** — house wiring ↔ supply ↔ board ground bounce. A series
   filter can't touch this one alone; the note on the diagram says it: loop the
   input lead **3–4 turns through a clip-on ferrite**.
3. **Radiated** — into the lemon wires and the player's body itself. Much
   weaker, but if ghosting persists, [HARDWARE.md](HARDWARE.md#still-ghosting)
   documents the next step (10 nF per key pin).

## How it plays

The game is [V5's](../v5-led-bar/README.md#how-it-plays), unchanged —
auto-calibration on boot with the bar as progress meter, noodling until the
first scored note, ten LEDs to a win, auto-advance through the four levels,
ending loop after level 4, SENS ± buttons and the smart-adjust gesture. The
filter is invisible to it; what changes is that calibration's measured noise
stays honest on a noisy mains, so the auto-margin stays tight instead of
widening to swallow the hum.

**Playing it, rather than reading about it: [the printed instruction sheet](../../docs/USER-GUIDE.md)**
([en español](../../docs/GUIA-DE-USO.es.md)).

### FREE PLAY, and the MODE WHEEL (2026-09-09)

Two additions, on the two buttons that were already there. No hardware changed.

**FREE PLAY** is the piano with the game taken out. The seven lemons become
**do re mi fa sol la si** — C5 D5 E5 F5 G5 A5 B5, one octave of plain white
keys — and that is all they are: no secret code, no wrong note, no penalty, and
crucially **the same lemon may be played over and over**. That last point is the
whole feature. The game deliberately swallows a repeated key (flaky fruit
contact used to machine-gun both the buzzer and the guesses), and that rule is
exactly backwards for an instrument, so free play does not have it.

The LED bar changes job with it: while a note sounds it is a **pitch meter**
(key 1 lights one LED, key 7 lights all ten), and when nothing is sounding it
shows **only its two ends lit** — a shape the game's left-filling bar can never
produce, so one glance says "this is the instrument, there is nothing to win".

Free play is **not a level**. Winning never advances into it; the only way in is
to choose it.

**The MODE WHEEL** is how you choose — the game-select switch that V5 removed,
brought back as a gesture:

| | |
|---|---|
| **Open** | hold **−** for 3 s. From 1 s the bar becomes a charge meter and a chirp climbs with it, so the gesture is never a mystery; let go early and a bump says "cancelled" |
| **Turn** | tap **+** / **−**. Five items — level 1, 2, 3, 4, FREE PLAY — and it **wraps** both ways |
| **Preview** | each stop plays the opening of its own theme (free play plays the scale) and shows itself on the bar: **level n = n LEDs, blinking**; free play = **one LED running** back and forth. Blinking is a question, steady is a score, and motion is neither |
| **Accept** | **both buttons at once**, in any order, any overlap |
| **Leave** | hold either button 3 s, or wait 20 s. Nothing changes — the game keeps its level and its progress bar |

A preview is cut off the instant the wheel turns again, so browsing runs at the
speed of the hand, not of the tunes.

### The overlaps, and why they do not collide

Three gestures now share two buttons, which is where this sort of thing normally
goes wrong. The four collisions and their resolutions are documented at the top
of [`firmware/include/ui_gestures.h`](firmware/include/ui_gestures.h) and each
one is a named test case. The two worth knowing at the keyboard:

- **Holding − ramps the sensitivity for the first second, then stops** and starts
  charging the menu instead. If the hold turns into a menu-open, the margin is
  **put back to where it was when the button went down** — reaching for the menu
  must not quietly desensitise the keyboard on the way in.
- **Both buttons for 1 s is smart adjust; − alone for 3 s is the wheel.** They
  live in different states and cannot be reached from each other by accident: if
  you press **+** while **−** is charging, the charge cancels and hands over to
  smart adjust, out loud.

## Build, test & flash

```bash
cd firmware
pio run -e nanoatmega328        # old-bootloader Nano clones
pio run -e nanoatmega328new     # 2018+ bootloader
pio run -e nanoatmega328-debug  # + raw ADC logging per accepted press
pio run -e emulation            # Velxio build
pio run -e emulation-freeplay   # Velxio build that boots into free play
test/run.sh                     # the host tests — run these before committing
```

`test/run.sh` is the deterministic check for everything above, and it needs
neither hardware nor the emulator: plain `g++`, two binaries, exit 0.
`ui_gestures_test` drives the button state machine through every overlapping
timeline; `piano_sim_test` compiles **the real `src/main.cpp`** against a fake
board (`test/arduino/Arduino.h`) and plays the piano — note mapping, free-play
repeats, the wheel, winning, cancelling. It is the only regression test that
reaches the wheel at all: the browser emulation has no pins left for the two
buttons.

Flash over the mini-USB as always. While *playing*, though, don't leave the PC
attached: its ground noise re-enters through the USB common ground and sidesteps
the whole filter.

## Verification status

| Check | Status |
|---|---|
| Diagram renders, 0 DRC violations | ✅ `python3 tools/wiring_diagrams.py v5.5` — 64 nets, 0 hard violations (2026-09-06) |
| Firmware builds (all five envs) | ✅ `pio run` per env, 2026-09-09 — 537 B RAM (26.2 %), 15 450 B flash (50.3 %) |
| Host tests: gestures + whole firmware | ✅ `firmware/test/run.sh`, 2026-09-09 — **97 + 56 checks, 0 failed**. Mutation-checked: breaking the free-play scale, dropping the margin restore, or moving the arming line each turns it red |
| Emulation, the game | ✅ V5's circuit and specs, verified there ([emulation/README.md](emulation/README.md)); the filter itself is analog supply hardware and **not emulatable** |
| Emulation, free play (`emulation/piano-mode.yaml`) | ⬜ **written, not yet run** — the Velxio harness is not installed on the machine this was written on. Compile-checked (`pio run -e emulation-freeplay`) and the spec parses, but no `--mode verify` has passed on it |
| The mode wheel in the browser | ⛔ **impossible on purpose** — ten LEDs + buzzer + key 7 use every digital line, so the two buttons have no pins. Covered by the host tests instead |
| Free play / the wheel on real hardware | ⬜ pending — written and validated with the board unplugged, at the owner's request; flash it and play it |
| Filter measured on the real board | ⬜ pending — build it and re-run the [V5 bench sampler](../v5-led-bar/HARDWARE.md#why-the-keyboard-is-pulled-up-again-measured-2026-07-27--28) during a switch-flipping session |
