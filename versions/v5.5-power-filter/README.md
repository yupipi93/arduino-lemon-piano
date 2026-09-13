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

### FREE PLAY, and the MODE WHEEL (2026-09-09, reshaped 2026-09-13)

Two additions, on the two buttons that were already there. No hardware changed.

> **What changed on 2026-09-13, after the first real session on the v0.7.1
> board.** Free play came off the wheel and onto its own button; neither button
> ramps the sensitivity any more; the LED bar counts sensitivity instead of
> margin; and a lemon held down sounds once instead of machine-gunning. Each is
> Sergio's own report from playing it, and each has a named test.

**FREE PLAY** is the piano with the game taken out. The seven lemons become
**do re mi fa sol la si** — C5 D5 E5 F5 G5 A5 B5, one octave of plain white
keys — and that is all they are: no secret code, no wrong note, no penalty, and
crucially **the same lemon may be played over and over**. That last point is the
whole feature. The game deliberately swallows a repeated key (flaky fruit
contact used to machine-gun both the buzzer and the guesses), and that rule is
exactly backwards for an instrument, so free play does not have it.

**Over and over means one note per touch, not one per flicker** (2026-09-13).
Resting a finger on a lemon is a single note that lasts as long as the finger
does. The contact underneath is not a switch — through the 1 MΩ pull-ups it
breaks for a few milliseconds at a time without the finger moving — so a release
is only believed after `RELEASE_CONFIRM_MS` (90 ms) of silence. Let go properly
and touch again and it sounds again; four deliberate taps are still four notes.

The LED bar changes job with it: while a note sounds it is a **pitch meter**
(key 1 lights one LED, key 7 lights all ten), and when nothing is sounding it
shows **only its two ends lit** — a shape the game's left-filling bar can never
produce, so one glance says "this is the instrument, there is nothing to win".

Free play is **not a level**. Winning never advances into it.

**The way in and out is to hold + for 3 s** (2026-09-13). It is a switch, not a
menu entry: the first hold swaps the game for the instrument, the next one puts
the game back — **restarting the level it interrupted**, which for anyone who
never touched the wheel is level 1. The arming meter and the rising chirp are
the same as −'s, so the two holds feel like one gesture with two destinations.

**The MODE WHEEL** is how you choose — the game-select switch that V5 removed,
brought back as a gesture:

| | |
|---|---|
| **Open** | hold **−** for 3 s (from free play it opens on the level free play interrupted). From 1 s the bar becomes a charge meter and a chirp climbs with it, so the gesture is never a mystery; let go early and a bump says "cancelled" |
| **Turn** | tap **+** / **−**. Four items — level 1, 2, 3, 4 — and it **wraps** both ways. Free play is **not** on it: it is a hold on **+** |
| **Preview** | each stop plays the opening of its own theme and shows itself on the bar: **level n = n LEDs, blinking**. Blinking is a question, steady is a score |
| **Accept** | **both buttons at once**, in any order, any overlap |
| **Leave** | hold either button 3 s, or wait 20 s. Nothing changes — the game keeps its level and its progress bar |

A preview is cut off the instant the wheel turns again, so browsing runs at the
speed of the hand, not of the tunes.

### The overlaps, and why they do not collide

Four gestures now share two buttons, which is where this sort of thing normally
goes wrong. The four collisions and their resolutions are documented at the top
of [`firmware/include/ui_gestures.h`](firmware/include/ui_gestures.h) and each
one is a named test case. The whole map, at the keyboard:

| gesture | while playing | in the wheel |
|---|---|---|
| **tap +** | one step **more** sensitive — one LED **on** | next level, wraps |
| **tap −** | one step **less** sensitive — one LED **off** | previous level, wraps |
| **hold + 3 s** | **free play ⇄ the level** | (3 s leaves the wheel) |
| **hold − 3 s** | **the level wheel** | (3 s leaves the wheel) |
| **both, 1 s** | smart adjust: learn the margin from a real touch | accept, at any overlap |

The two worth knowing:

- **A tap is a step. A hold is a mode.** Holding a button no longer ramps the
  knob — it used to fire a step every 120 ms, which meant every mode change also
  re-tuned the keyboard by thirty counts. The nudge still fires on press, because
  a knob that waits for the release feels broken, so the gesture that fires
  **puts that one step back**: reaching for a mode must not leave the keyboard
  somewhere else.
- **Both buttons for 1 s is smart adjust; one button alone for 3 s is a mode.**
  They live in different states and cannot be reached from each other by
  accident: press the other button while one is charging and the charge cancels
  and hands over to smart adjust, out loud.

**The bar counts sensitivity, not margin.** More LEDs lit = more sensitive = a
smaller margin, so **+ adds light and − takes it away**. It was the other way
round until 2026-09-13, and it read as a lie: a button labelled "more" that puts
lights out is a button that says "less" from across the room.

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
