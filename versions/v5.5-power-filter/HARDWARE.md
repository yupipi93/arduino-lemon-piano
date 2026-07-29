# V5.5 hardware — the power-entry filter

Everything on the board — pin map, keyboard, LED bar, buttons, buzzer — is
**V5's, unchanged**: see [../v5-led-bar/HARDWARE.md](../v5-led-bar/HARDWARE.md)
for all of it. This file documents only what V5.5 adds: the filtered 5 V supply.

> **Build status:** designed and DRC-validated on the diagram, **not yet
> measured on the real board** (the V5 board itself is also still to be wired —
> V5 TODO #13). The design numbers below are computed, not measured; the bench
> validation recipe is at the end.

## The problem, quantified

The V5 keyboard reads seven analog pins pulled up to the rail through 220 Ω,
and detects a touch as a dip of **3–4 ADC counts ≈ 15–20 mV** (skin ≈ 1 MΩ
against 220 Ω — the [V5 measurements](../v5-led-bar/HARDWARE.md#why-the-keyboard-is-pulled-up-again-measured-2026-07-27--28)).
The +5 V rail is simultaneously:

- the top of all seven key pull-ups,
- **AVcc — the ADC's reference**,
- whatever the USB source delivers, spikes included.

A slow rail sag cancels out (the ADC is ratiometric: baseline and reading move
together), but a **fast edge does not** — the pin node (behind 220 Ω and wire
capacitance) and the reference (straight into AVcc) move at different speeds,
and for a few milliseconds the ADC sees a differential far bigger than 4 counts.
A relay or light-switch arc puts hundreds of mV of ringing on a domestic 5 V
wall wart; a PC's 5 V shared with twenty other loads is a permanent storm.
Every such event **is** a key press as far as `strongestKey()` can tell.

## The filter

```
 5 V IN ──┬──[1N5817 >]──┬────────([100 µH])────────┬─── +5 V rail (≈ 4.7 V)
          │   D_S        │                          │
        [TVS]          [470 µF] ‖ [100 nF]      [470 µF] ‖ [100 nF]
    P6KE6.8A D_TVS       │  C1        C2            │  C3        C4
          │              │                          │
 GND IN ──┴──────────────┴──────────────────────────┴─── GND rail
```

Left to right — each element by role:

| Part | Value / type | Role |
|---|---|---|
| D_TVS | **P6KE6.8A** (axial, unidirectional, 600 W) | Standoff 5.8 V, clamps ~10 V: eats the microsecond switch/relay spikes before anything else sees them. Reversed input? It conducts at −0.7 V and (with D_S) nothing downstream notices. |
| D_S | **1N5817** Schottky, series | Reverse-polarity protection, and it **decouples the mini-USB**: with a PC attached for flashing, the Nano's own USB 5 V cannot back-feed the filter (no diode fight, no loop current). Costs ~0.25 V at this load. |
| C1 ‖ C2 | **470 µF/16 V** electrolytic ‖ **100 nF** ceramic | Input reservoir: absorbs what the TVS lets through and gives the choke a stiff source. The ceramic covers the MHz range the electrolytic's ESL misses. |
| L1 | **100 µH** power choke, **≥ 1 A**, DCR ≤ 0.3 Ω | The series element of the pi. Not a signal inductor — a drum/toroid power choke; it must not saturate at the 200 mA worst-case board draw. |
| C3 ‖ C4 | **470 µF/16 V** ‖ **100 nF**, at the rail | Output reservoir, physically at the breadboard's rail entry. |

**Numbers.** fc = 1/(2π√(L·C3)) = 1/(2π√(100 µH · 470 µF)) ≈ **730 Hz**,
second-order (−40 dB/dec): conducted noise at 50 kHz (typical wall-wart
switcher) is attenuated by ~70 dB ideal, realistically 40–50 dB against ESR —
either way, orders of magnitude below the 15–20 mV that matters. The LC
resonance at fc has Q ≈ √(L/C)/R_ESR ≈ 1.5 with ordinary electrolytics — mildly
underdamped, harmless here (the transients we care about are far above fc; if a
scope ever shows ringing at ~700 Hz, a 1 Ω resistor in series with C3 kills it).

**Voltage budget.** 1N5817 (~0.25 V) + choke DCR (~0.06 V at 200 mA) puts the
rail at **≈ 4.7 V**. Everything on the board is ratiometric or has headroom:
thresholds scale with AVcc (calibration measures them *on this rail*), LEDs
lose ~6 % brightness, the buzzer doesn't care. Nothing to compensate.

**What it deliberately does not do:** common-mode rejection. A series filter
only works on the differential path. For the common-mode path (house wiring ↔
supply ↔ board), **loop the input lead 3–4 turns through a clip-on ferrite
core** — it costs nothing and it is the one thing that also helps when the
transient arrives through the *air* rather than the wire.

## Bill of materials (delta vs V5)

| Component | Qty | Notes |
|---|---|---|
| P6KE6.8A TVS (axial) | 1 | any 600 W axial TVS with 5.8 V standoff works (e.g. SA5.0A) |
| 1N5817 Schottky | 1 | 1N5819 fine too (slightly higher Vf) |
| 470 µF / 16 V electrolytic | 2 | low-ESR preferred, not critical |
| 100 nF ceramic | 2 | X7R |
| 100 µH power choke, ≥ 1 A | 1 | drum or toroid; a "100 µH" signal inductor rated 100 mA will saturate — don't |
| 5 V input pigtail / barrel jack | 1 | chop a USB-A cable: red = V+, black = GND |
| clip-on ferrite core | 1 | input lead through it, 3–4 turns |

## Powering rules

1. Feed the filter from a **USB wall charger or a bench supply** — a source
   with nothing else hanging off it. The PC's 5 V is the worst source in the
   house.
2. The Nano's **mini-USB stays for flashing only**. It bypasses the filter by
   construction (it lands on the Nano's own 5 V node); the 1N5817 keeps it from
   back-feeding C1–C3. Don't leave the PC attached while playing — USB ground
   noise walks straight past any series filter.
3. Keep the filter **at the board end** of the input lead, not the charger end:
   the lead between filter and rail is the part that must stay short.
4. One ground: the filter's GND, the rails, and the player's clip meet at the
   breadboard rail — no second path back to the source.

## Still ghosting?

If phantom notes survive filtered power (they'd now come via the radiated /
body path, not the supply), the next escalation — **not** part of the V5.5
board, by design, one change per version — is **10 nF ceramic from each key pin
to GND**, at the pin. With the 220 Ω pull-up that is a ~2 µs pole: invisible to
a 70 ms note floor, deadly to a microsecond spike arriving on the lemon wire.
That would be V5.6, and it should come with before/after sampler evidence.

## Bench validation recipe

The claim to verify is "a light switch no longer plays the piano", measured, not
felt:

1. Run the [V5 bench sampler](../v5-led-bar/HARDWARE.md#why-the-keyboard-is-pulled-up-again-measured-2026-07-27--28)
   (7 channels, 20 Hz) on **raw USB**, flip a room light 20 times, save the log.
2. Same session, same switch, powered **through the filter**: 20 flips, save.
3. Compare worst-case single-sample deviation per channel. Raw should show the
   phantom-press events; filtered should stay within the calibrated noise
   (±1–2 counts). If filtered still shows events, they are common-mode or
   radiated — ferrite turns first, then the V5.6 pin capacitors.

## Diagrams

| File | What it shows |
|---|---|
| [images/wiring-v5.5.png](images/wiring-v5.5.png) | **full V5.5 wiring** — the V5 board plus the power-entry filter across the top |

`wiring-v5.5.png` is generated by
[../../tools/wiring_diagrams.py](../../tools/wiring_diagrams.py) (`build_v5_5`)
on the wirewright engine — `python3 tools/wiring_diagrams.py v5.5`,
DRC-validated, never hand-drawn. The filter parts (`capacitor`, `inductor`,
`diode`, `power_jack`) were added to the engine's library for this version.
