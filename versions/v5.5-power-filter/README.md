# V5.5 — filtered 5 V supply (2026) — newest board

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
- **off** the board: nothing. The keyboard, LED bar, buttons and buzzer are
  byte-for-byte V5 — and so is the firmware (only the serial banner changed);
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

Identical to [V5](../v5-led-bar/README.md#how-it-plays) — auto-calibration on
boot with the bar as progress meter, free play until the first scored note, ten
LEDs to a win, auto-advance through the four levels, ending loop after level 4,
SENS ± buttons and the smart-adjust gesture. The filter is invisible to the
game; what changes is that calibration's measured noise stays honest on a noisy
mains, so the auto-margin stays tight instead of widening to swallow the hum.

## Build & flash

Same as V5 — the firmware is the V5 code with a `V5.5` banner:

```bash
cd firmware
pio run -e nanoatmega328        # old-bootloader Nano clones
pio run -e nanoatmega328new     # 2018+ bootloader
pio run -e emulation            # Velxio build
```

Flash over the mini-USB as always. While *playing*, though, don't leave the PC
attached: its ground noise re-enters through the USB common ground and sidesteps
the whole filter.

## Verification status

| Check | Status |
|---|---|
| Diagram renders, 0 DRC violations | ✅ `python3 tools/wiring_diagrams.py v5.5` — 58 nets, 0 hard violations (2026-07-29) |
| Firmware builds (all three envs) | ✅ `pio run` per env, 2026-07-29 (see [CHANGELOG.md](../../CHANGELOG.md)) |
| Emulation | the filter is analog supply hardware — **not emulatable**; the game circuit is V5's, verified there ([emulation/README.md](emulation/README.md)) |
| Filter measured on the real board | ⬜ pending — build it and re-run the [V5 bench sampler](../v5-led-bar/HARDWARE.md#why-the-keyboard-is-pulled-up-again-measured-2026-07-27--28) during a switch-flipping session |
