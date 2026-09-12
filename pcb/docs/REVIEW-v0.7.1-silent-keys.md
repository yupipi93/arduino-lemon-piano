# Review — v0.7.1 assembled, the keys make no sound (2026-09-12)

Sergio received and assembled the real v0.7.1 board. The Nano boots and runs,
but touching a lemon — or touching an A-pin directly — plays nothing. The same
circuit "looks like" the breadboard rig, which works. He suspected a design
fault, because on the board every key resistor appears joined to every other.

This is the review. It is written against the **copper**, not against the prose
in `NETLIST.md` / `HARDWARE.md`, because those two documents disagree with each
other (see "The document that lied", below).

## 1. The board is a faithful build of its own netlist

Independently re-derived from `pcb/kicad/lemon-piano.kicad_pcb` — pads parsed
out of the footprints, tracks joined geometrically, the B.Cu pour treated as
the GND node:

| Check | Result |
|---|---|
| Nets that are one connected copper island | **34 / 34** |
| Nets split into 2+ islands (open) | **0** |
| Copper islands carrying two different nets (short) | **0** |
| Non-GND pad centres falling inside the GND pour | **0** |
| Non-GND B.Cu track points inside the GND pour | **0** |
| KiCad 9.0.9 DRC (`validation/drc-v0.7.1.json`, 2026-09-06) | 0 violations, 0 unconnected |

Nano socket rows re-checked pin by pin against the real Nano pinout: U1 pad 4 =
A0 = `/KEY1` … pad 10 = A6 = `/KEY7`, pad 11 = A7 = `/SENS_MINUS`, pad 12 = 5V,
pad 14 = GND; U2 pad 5 = D2 = `/LED1` … pad 14 = D11 = `/LED10`, pad 15 = D12 =
`/SENS_PLUS`. All correct.

**So there is no routing mistake and no fabrication mistake.** Which also means
DRC proved nothing about the question being asked: DRC checks the copper against
the netlist, never the netlist against the intent. The fault is one level up.

### The resistors that look joined are supposed to be joined

`R1…R7` pad 2 all sit on `/+5V`; `R8…R17` pad 1 all sit on the `/GND` pour.
That is what a pull-up bank and a bank of LED ballasts look like from the bottom
side. **It is correct — do not "fix" it.**

But the observation is a sharp one, because it is the visible fingerprint of the
real problem: **this board is not the circuit on the working breadboard.**

- Board / V5 topology: `+5V ──[220 Ω]──┬── A(n)` with the fruit hanging off the
  same node and **the player holding a GND clip** (J2 pin 8). One common +5 V
  rail, seven resistors joined at one end. A touch drags the pin **down**.
- V4 / V4.5 topology: `lemon ──[220 Ω]── A(n)`, pin otherwise floating, and
  **the player holding a +5 V clip**. Seven resistors with *nothing* in common.
  A touch drags the pin **up**.

If the working breadboard is the second one, the board cannot behave like it,
and no amount of soldering will make it. On the v0.7.1 board there is no way to
play without a galvanic path to board GND.

## 2. The real design fault: 220 Ω is three orders of magnitude too small

The whole touch signal on this board is **4 ADC counts out of 1023** — 0.4 %.
`main.cpp` says so in as many words (`touchMargin = 4`, "a fruit touch dips to
1018"), and `autoCalibrate()` will not go below `AUTO_MARGIN_MIN = 4`.

Touch dip in ADC counts = `1023 · R_pullup / (R_pullup + R_body)`:

| pull-up | wet firm grip ≈ 50 kΩ | normal ≈ 300 kΩ | dry finger ≈ 1 MΩ | nail / poor clip ≈ 3 MΩ |
|---|---|---|---|---|
| **220 Ω (v0.7.1)** | **4.5** | **0.7** | **0.2** | **0.1** |
| 100 kΩ | 682 | 256 | 93 | 33 |
| 220 kΩ | 834 | 433 | 185 | 70 |
| 1 MΩ | 974 | 787 | 512 | 256 |

With 220 Ω the key fires only if the **entire** path — hand skin → clip → body →
fingertip → lemon flesh → nail → clip → board — stays under **56 kΩ**. Dry skin
contact alone is routinely 100 kΩ–1 MΩ *per contact point*. The bench rig that
produced "baseline 1022, touch 1018" was sitting exactly on that edge.

Two more consequences of living in a 4-count window:

- `touchMargin = max(4, 2 × worst measured noise)`. One extra count of noise on
  the assembled board raises the bar to 6 counts — **above the entire signal**.
  The board then behaves exactly as reported: boots, calibrates, never triggers.
- `trackBaselines()` chases the idle level every 100 ms. A touch that builds
  slowly gets absorbed into the baseline instead of firing.

`v5-led-bar/HARDWARE.md` already names the fix in its last paragraph and then
does not take it: *"a much weaker pull-up (~1 MΩ) would turn those 4 counts into
hundreds."* That is the change this board needs.

### The catch that comes with it

At 1 MΩ the ADC source impedance is ~100× the ATmega328P's recommended 10 kΩ,
so the sample-and-hold never charges within one conversion and each reading
carries residue from the previous mux channel. That is *precisely* the failure
already measured on the floating V4 keyboard ("readings came in gradient ramps").
So the resistor swap is only half a fix. The other half is in firmware:

- **read each channel twice and throw the first read away** (or `delayMicroseconds(200)`
  after switching channels), and
- **220 kΩ, not 1 MΩ**, as the sweet spot: still 185–433 counts of signal on
  ordinary skin, and a settling time a double-read absorbs comfortably.

## 3. The other candidate, and it is cheaper to test first: the buzzer branch

"The notes do not sound" and "the keys do not detect" are not the same fault,
and this firmware distinguishes them for free at every boot:

`setup()` → `autoCalibrate()` plays `soundCalStart()` (fireball), **one coin per
key, seven of them**, then `soundCalDone()` (power-up sweep), then
`playLevelIntro()`. All of that happens **before any lemon is touched**.

- **Silent at boot too** → the fault is BUZ1 / D13, not the keys.
- **Beeps at boot, silent on touch** → the fault is the sensing front end, i.e. §2.

And a second free indicator: on this board the buzzer is on **D13**, which is
also the Nano's on-board `L` LED. Any note lights it. If `L` flickers while the
board is silent, the firmware is playing and the transducer is the problem.

Three things that would kill the buzzer branch on the board but not on the
breadboard:

1. **A magnetic buzzer instead of a piezo.** The footprint is
   `Buzzer_12x9.5RM7.6`. A piezo element is kΩ-level and D13 drives it happily; an
   electromagnetic transducer is 16–42 Ω, i.e. 120–300 mA from a pin rated 40 mA.
   It will be silent or near-silent, and the pin may already be damaged.
2. **Anything low-impedance on J5.** J5 is wired straight across BUZ1 and carries
   raw D13. ADR-034 says it in the file: *a bare 4–8 Ω voice coil must not be
   connected directly.* The enclosure from v4 onwards has a 30.7 × 27.8 speaker
   and a mounting rule that says "J5 with a soldered or right-angled cable" — so
   a speaker on J5 is a likely thing to have done during assembly, and it would
   short D13 to ~5 Ω and silence everything. J5 needs an **amplified** module.
3. An **active** buzzer where a passive one belongs (it would beep, but wrongly),
   or reversed polarity on an active one (pad 1 = +).

Do not reach for `versions/v0-buzzer/` to test this: its buzzer is on **D8**,
which on this board is LED7's pin. V0 is a breadboard diagnostic, not a board one.

## 4. What to measure, in order

1. **Power on and listen.** Boot chirps? That splits §3 from §2 in five seconds.
2. **Serial monitor, 9600 baud** (`serialEnabled` is `true`; the log is
   unconditional). Read the calibration block:
   - `baseline ≈ 1022, noise 0–1` → rail and pull-ups are fine; go to §2.
   - `baseline ≈ 0` → the +5 V rail or R1–R7 are not there. Check continuity from
     R1 pad 2 to U1 pad 12.
   - `noise ≥ 3` → `auto margin` will be ≥ 6, above the whole signal. §2 again.
3. **Unplug everything from J5** and retry.
4. **Continuity, board unpowered**, on SW1: between the two pads on the **same
   side** (4.5 mm apart) versus the two on the **same row** (6.5 mm apart). The
   generated footprint groups pad 1 = north row and pad 2 = south row, i.e. it
   assumes the 6.5 mm pairs are the internally-shorted ones. On a standard 6 mm
   tact switch the shorted pairs are the **4.5 mm** ones. If the meter beeps on
   the 4.5 mm pair with the button up, `/SENS_PLUS` and `/SENS_MINUS` are hard
   shorted to GND and both buttons read permanently held. **Unverified — this is
   a measurement to take, not a confirmed defect**, but it costs thirty seconds
   and it would explain a second set of symptoms.
5. `firmware/probe/` — flash it and watch all seven channels live. §5.

## 5. `firmware/probe/` — the bench sampler this board never had

`versions/v5.5-power-filter/firmware/probe/` is a standalone PlatformIO project
that shares the board and nothing else: no game, no LEDs, no buttons. It prints
all seven channels at 10 Hz with each channel's boot baseline and live delta, so
a touch is either visible as a number or it is not there. `-DDEBUG_TOUCH` on the
game build cannot do this — it only prints on an **accepted** press, so it prints
nothing at all when the fault is that no press is ever accepted.

## 6. Proposed v0.8.0

One value change and one firmware change, both small:

- **R1–R7: 220 Ω → 220 kΩ.** Same 0805 footprint, same pads, same board. It is a
  hand-swap on the bottom side; no re-fabrication is needed to test the theory.
- **`readKey()`: double-read each channel, discard the first.** Required by the
  higher source impedance, and harmless at any impedance.
- R18 (10 kΩ, SENS−) stays. R8–R17 (LED ballast) stay.

Per `docs/VERSIONING.md` a resistor value change is a hardware change, so it
lands as **V5.6**, not as a patch to V5.5. Not written yet — it waits on the
measurements in §4, because §3 may turn out to be the whole story.

## The document that lied

`versions/v5-led-bar/HARDWARE.md` contains both topologies at once. Its pin map,
its BOM and its measured section all say **pull-up to +5 V, player on GND, touch
drags the pin down**. Its own "Touch sensing" section, three paragraphs later,
says **"pins float near 0, the +5 V clip through the body raises the reading,
threshold = baseline + max(40, 3 × noise)"** — copied forward from V4.5 and never
updated. `docs/HARDWARE.md` repeats the stale version, listing V5 under "2026
boards — the clip is on +5 V".

The firmware is unambiguous and agrees with the board:
`thresholdFor() = baseline − touchMargin`, `keyTouched() = readKey ≤ threshold`.

Fixed in this commit. Ground truth for the front end is **`main.cpp` + the copper**,
in that order.
