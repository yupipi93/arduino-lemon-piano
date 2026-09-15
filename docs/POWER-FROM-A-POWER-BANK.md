# Powering the piano from a USB power bank

> **Applies to the board that exists today** — the [V5.5](../versions/v5.5-power-filter/)
> PCB v0.7.1, unchanged. Nothing here modifies the piano; it all happens on the
> far side of `J1` (`5V IN`). The LiPo + IP5356 build is a different, still
> unbuilt, proposal — [V6](../versions/v6-battery-amp/).

The piano runs fine off a wall charger. Off a power bank it **dies after a few
seconds**, which is the classic low-load auto-shutdown: a bank is built to stop
burning its own cell into a cable nobody is using, so it opens the output when
the draw stays under a threshold.

## Why the piano falls under the threshold

| State | Draw | Above a 45-75 mA cut-off? |
|---|---|---|
| Nano + buzzer idle, **bar dark** (free play, or a level with no LED lit yet) | ≈ 25-35 mA | **no** |
| One bar LED lit | + ≈ 13.6 mA ((5 − 2.0) / 220 Ω) | borderline |
| All ten lit (victory animation) | ≈ 160 mA | yes |

So the piano is *visibly* fine while the bar is counting and dies exactly when
the player is doing well enough to have cleared it, or is noodling in free play.
The numbers above are computed from the schematic, **not measured** — measuring
them is step 1 below.

## The three things that can cut the output — they are not the same fault

Distinguish them by **when** it dies, before changing anything:

| Symptom | Cause | Fix |
|---|---|---|
| Runs a few seconds, then off | **low-load timeout** | raise the draw (bleeder / keep-alive LED), or the bank's own low-current mode |
| **Bank shows a protection code (`ICP`) and drops the instant the board is attached** | **reversed polarity** — `D1`'s anode is on `/GND` and its cathode on `/VIN`, so a swapped pigtail forward-biases the TVS: a 0.7 V diode straight across the bank | swap the two wires; `J1` pad 1 = `+` | 
| Never comes up; bank blinks and drops instantly | **inrush** — `C1 ‖ C3` = 940 µF looks like a short to a boost whose protection reacts in < 50 µs | drop C1 to 220 µF (`fc` is set by C3, not C1) |
| Comes up at 5 V but the bank never registers a device | **no PD contract** — a two-wire USB-A pigtail has no CC lines, so nothing negotiates | a USB-C **to USB-C** path with a trigger in it |

> **`ICP` is a protection trip, not a shutdown.** UGREEN does not publish the
> expansion (searched 2026-09-15; their own FAQ only says an unusual code means
> the bank has entered protection mode). Read it as a *current* protection: the
> bank saw something it judged an overload or a short and opened the output. The
> distinction that matters is that a low-load timeout is silent and a trip is
> not — a code on the screen means too much current, never too little.

## The trigger module: `mod-pd-trigger-dip100w`

Inventory: *"PD/QC 100W trigger module with DIP selector (screw terminal
output)"*, qty 3. Small blue PCB, USB-C in at one end, a **red 3-position DIP**
in the middle (silk `1 2 3` and `ON`), a 2-way screw terminal out.

### DIP positions (1 = ON, 0 = OFF; OFF = slider away from the `ON` silk)

| 1 | 2 | 3 | Requested voltage |
|:-:|:-:|:-:|---|
| **0** | **0** | **0** | **5 V ← this one** |
| 1 | 1 | 1 | 9 V |
| 1 | 1 | 0 | 12 V |
| 1 | 0 | 0 | 15 V |
| 1 | 0 | 1 | 20 V |

**For 5 V: all three sliders OFF**, away from the `ON` legend. The module does
not step up — if the source cannot serve the requested voltage it falls back to
5 V, so a wrong code fails safe *downwards*. The DIP must be set **before**
plugging the source in; never move it with the piano connected.

### The 9 V mistake is not survivable by the board

`D1` is a **P6KE6.8A**: 5.8 V standoff, avalanche from ≈ 6.45 V. It is a 600 W
part *for 1 ms*, not a shunt regulator. On a 9 V contract from a 100 W bank it
conducts continuously, cooks, and then the ATmega sees 9 V on AVcc.

**So: set the DIP, plug the trigger into the bank, and measure the screw
terminal with the Fluke 79 III — before the wires ever reach `J1`.** Polarity at
the header is ADR-025's: `J1` pad 1 = `+`, pad 2 = `GND`, silk `5V IN + −`.

### The 5 V setting: the doubt is resolved, the trigger DOES hold the port open

The worry was that a decoy asked for 5 V might simply take vSafe5V and never
send an explicit Request, leaving the bank as unaware of the load as a bare
two-wire pigtail does.

**Observed 2026-09-15 (Natalia, UGREEN Nexode 100 W): with the trigger alone on
the bank — a few mA of load, far under any low-load threshold — the output stays
up.** So whatever the trigger does on CC is enough to keep the port open, and
the low-load timeout is off the list of faults for this piano. (Duration of the
observation not recorded; if the bank ever drops after a long idle, re-open
this.)

## Reversed polarity is the first thing to rule out, and it is free

`D1` is wired cathode to `/VIN`, anode to `/GND` (`pcb/docs/NETLIST.md`, nets 2
and 3). At the correct polarity it sits under its 5.8 V standoff and does
nothing. **Swap the two wires and it is forward-biased: a 0.7 V silicon diode
directly across the power bank**, which is a short by any protection circuit's
definition, and the reason the trip is instantaneous and repeatable rather than
delayed. `D2` blocks the reverse path, so the Nano and everything behind it never
sees it — the design fails exactly as intended, and the bank's code is the alarm.

Three checks, none of which need the board powered:

1. **Which screw is which.** Trigger on the bank, nothing on the terminal, Fluke
   on DC volts across the two screws: the red probe on the screw that reads
   **+5 V** is the wire that goes to `J1` **pad 1** (silk `5V IN + −`).
2. **Stray strand.** A chopped pigtail frays; one whisker bridging the two screws
   is the same trip with none of the theory. Look at it under the microscope.
3. **Short on the board.** Everything disconnected, Fluke on Ω, **red on `J1`
   pad 1, black on pad 2**: the reading starts low and climbs as the meter charges
   `C1`/`C3`, settling in the kΩ. Near 0 Ω that does not move = a real short.
   **With the probes the other way round a low reading is normal** — that is `D1`
   conducting, not a fault. Do not diagnose from the reversed reading.

If the polarity *was* reversed, check `D1` survived before trusting it again:
diode test should read ~0.7 V one way and open the other. Shorted both ways means
it took the hit and needs replacing (`semi-p6ke6-8a`, 40 in stock) — the board is
unprotected without it.

## If polarity is fine: it is the inrush

Then the 940 µF is what the bank is objecting to, and the order of operations is
the free test: **tighten the wires into the screw terminal first, with the
trigger unplugged, and only then plug the USB-C into the bank.** A cold start
lets the bank ramp VBUS into the capacitance on its own soft-start; hot-plugging
a discharged 940 µF onto an already-established 5 V rail is the worst case there
is, and it is what happens when the terminal is landed last.

If it still trips, drop **C1** from 470 µF to 220 µF (the electrolytic
assortment kit has it). C1 is the one that matters here: it sits right behind the
Schottky with nothing but `D2` between it and the source, while `L1` limits how
fast `C3` can be asked for current. The filter does not suffer — `fc` is set by
C3.

## Measure it — KWS-X1 in line

`dev-kws-x1-usbc-meter` reads V/A/W *and* the negotiated protocol, which is
exactly the unknown above.

1. Bank → **KWS-X1** → USB-C **to USB-C** cable → trigger → screw terminal.
   (A USB-A port on the bank, or a USB-A→C cable, means no CC lines and no
   negotiation — that path cannot test the hypothesis.)
2. Protocol page: does it show a **PD contract at 5 V**, or nothing?
3. Measurement page, with the piano attached: read the **actual idle current**
   and watch what it reads at the instant the output drops. That number is the
   threshold to beat, and it turns the table at the top of this file from
   computed into measured.

## If it still shuts down: the keep-alive

Raise the idle draw above the measured threshold. A plain resistor across the
trigger's screw terminal — i.e. on the **unfiltered** side, in parallel with the
piano — costs nothing on the rail: it is a constant DC load, it adds no drop
across `L1`, and it cannot move AVcc.

| Bleeder on 5 V | Extra draw | Dissipation | In stock |
|---|---|---|---|
| 220 Ω | 23 mA | 0.11 W | `res-kit-220r` ×140 |
| 150 Ω | 33 mA | 0.17 W | `res-kit-150r` ×40 |
| 2 × 220 Ω in parallel (110 Ω) | 45 mA | 0.11 W each | `res-kit-220r` |
| 100 Ω | 50 mA | **0.25 W — at the limit of a ¼ W part** | `res-kit-100r` ×40 |

Start at 150 Ω and step down only if the meter says so. Prefer the 2 × 220 Ω
pair over a single 100 Ω: same current, half the heat per part.

A 5 mm LED in series with 100 Ω is the same 30 mA and doubles as a power-on
indicator — and unlike the firmware keep-alive sketched in
[V6](../versions/v6-battery-amp/#open-risks--measure-before-building), it costs
no LED of the progress bar's semantics.

**Check the bank's own low-current mode first**, though — UGREEN's Nexode line
generally has one (a double press of the power button), and a mode that costs
nothing beats a resistor that burns 165 mW forever. Unverified for this exact
unit; the manual is the authority.

## Why battery power is worth the trouble at all

Not portability. The V5.5 filter is a **series** filter, so by construction it
cannot touch the common-mode path: mains-referenced through a charger, the
player's body and the board sit at different potentials and the difference lands
across the body → lemon → pin loop that the 3-4 count (15-20 mV) margin is
measured on. On battery the board floats *with* the player and it cancels — and
the charger's Y-cap leakage (0.1-0.35 mA at 50 Hz, today flowing through the
player's hand) goes away with it. Expect battery mode to be **quieter than
mains**, not merely as quiet. That claim is also unmeasured; the 20-light-switch-flip
comparison in [V5.5's bench recipe](../versions/v5.5-power-filter/HARDWARE.md#bench-validation-recipe)
is how to settle it.
