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

## 2b. What the bench said back (2026-09-12, same day)

Sergio, with the board in front of him, added three facts. They delete two whole
branches of this review and sharpen the third.

1. **Every boot sound plays, and both SENS buttons beep when pressed.** So §3 is
   closed: BUZ1, D13 and J5 are fine. And it closes §4 as a bonus — the buttons
   respond, so SW1/SW2 are not shorted and the footprint's pad grouping is right.
2. **SENS− is on A7 and only works through R18, a 10 kΩ pull-up to `/+5V`** — the
   same rail, the same bottom-side row as R1…R7. That one beep proves the 5 V
   rail reaches the key pull-ups, with no meter. So "R1–R7 or the rail are
   missing" is closed too.
3. **He never touches a clip.** On the breadboard, with 220 Ω, he touches only
   the fruit — no ground clip, no second hand, nothing — calibrates, and it
   plays.

Buzzer, buttons, LEDs and rail are alive. **Exactly one block is dead: the seven
key channels.**

### Fact 3 is the one that matters, and it does not fit this board

With a 220 Ω pull-up the key node is a *stiff* node: 220 Ω to the rail. A body
that is not galvanically tied to circuit GND couples through maybe 100–200 pF to
earth — microamps at 50 Hz, which across 220 Ω is nanovolts. **Nothing a
10-bit ADC can see.** A no-clip touch cannot work on the v0.7.1 front end. It is
not a tolerance, a threshold or a margin; it is four orders of magnitude.

So the working rig must have a **high-impedance** key node, and there are exactly
two ways to get one:

- **The V4 / V4.5 front end**: pin floating, the 220 Ω in *series* between fruit
  and pin, player's clip on +5 V. Same resistor value — which is precisely why
  "the same values work on the breadboard" is true and still tells us nothing.
  A floating pin is swung hundreds of counts by a hand, with no clip needed, and
  the touch reads **UP**. The V5 measurements describe that node exactly ("idle
  ~250 with 76–104 counts of noise… readings came in gradient ramps"), call it
  unreadable, and go back to pull-ups — trading a large messy signal for a tiny
  clean one that needs a clip.
- **An earth reference through the supply.** A breadboard fed from an earthed PC
  sits at earth potential, so a body on a chair has a real return path and no
  clip is needed. A phone charger or a power bank is double-insulated and floats;
  its GND has no relation to the body. This matters because V5.5's own powering
  rules say *"Feed the filter from a USB wall charger or a bench supply — the
  PC's 5 V is the worst source in the house"* and *"don't leave the PC attached
  while playing"*. **Following the filter's rule removes the earth path that made
  clipless play work.** It also explains why several different supplies all
  failed: if they were all wall-warts, they all float.

Both explanations point the same way, and the probe separates them by the sign
of the movement. Do not pick one from the armchair.

### Why "just press both buttons and recalibrate" cannot rescue it

Sergio's objection is fair: `MARGIN_MAX` is 600, so the smart-adjust gesture has
plenty of range for any resistor value. The range is not the problem. The
*direction* is. `learnFromTouch()` tracks one array:

```cpp
int lo[KEY_COUNT];                      // ...only the minimum
if (v < lo[i]) lo[i] = v;
dropped[i] = baseline[i] - lo[i];
if (dropped[i] < 0) dropped[i] = 0;     // an upward touch scores exactly zero
```

A touch that *raises* a pin scores `dropped = 0` on every channel, so `best`
stays −1 and the gesture prints "touch not separable from noise — margin
unchanged", forever, however wide the range is. **A one-directional search
cannot be saved by making it wider.** Same for `autoCalibrate()`, `keyTouched()`
and `strongestKey()`: all three only ever look down.

That is also why the probe in §5 reports both directions and names the winner.

## 2c. MEASURED, 2026-09-13: zero. Not small — zero.

The Nano was plugged into quantumpc and the board read over SSH by the agent
directly. Raw log: `pcb/validation/probe-v0.7.1-board-2026-09-13.txt`.

```
  key 1..7  baseline=1023  noise=0        (all seven identical)
-- since boot   DOWN: 0 0 0 0 0 0 0   UP: 0 0 0 0 0 0 0
   verdict: NOTHING MOVES. No touch signal to threshold.
```

**156.5 s, 1345 samples, seven channels, and the set of distinct values observed
on every single one of them is `{1023}`.** Not a weak signal. Not a signal under
the margin. No signal, at the resolution of the instrument, for two and a half
minutes.

### That is the predicted number, not an anomaly

§2's table predicted **0.2–0.7 counts** for a normal-to-dry touch through a 220 Ω
pull-up. The ADC cannot show a fraction of a count, so the prediction *was* zero.
Working back from the observation: a reading of exactly 1023 needs the total
hand → body → fruit → clip path to stay **above ~250 kΩ**, and ordinary dry skin
is 100 kΩ–1 MΩ *per contact point*. The measurement and the arithmetic agree.

Note the honest gap: the operator was asked to touch during the window but that
cannot be verified from the log. It does not change the conclusion — a touch
through this front end could not have shown up either way — and step 1 below
settles it without needing anyone's word for it.

### Three things this closes for good

- **R1–R7 are populated and soldered.** A floating analog pin does not sit at a
  rock-steady 1023; it wanders and drags the previous mux channel's residue
  behind it, which is exactly what the V4 keyboard measurements look like. Seven
  immovable 1023s are the signature of a working pull-up.
- **Nothing is mis-calibrated.** `noise=0` puts the auto-margin at its floor of
  4, the most sensitive the firmware can be. There is no threshold left to tune.
- **No firmware change can rescue this board.** A 220 Ω path to the rail defeats
  every sensing technique available on an AVR: the resistive divider (the signal
  is sub-count), and RC timing / `CapacitiveSensor` too, because the node
  recharges through 220 Ω in nanoseconds. **The resistor has to physically go.**

## 2d. The plan, in the order that costs least

**Step 1 — remove R1…R7 and measure again. No new parts.** Seven 0805s on the
bottom side; they lift with an iron and tweezers. Leave the pads empty and re-run
the probe. If the channels start swinging while touching the fruit **with no
clip**, then the breadboard's key node is a floating one and the PCB's pull-up
bank *is* the entire difference between the two rigs — established by
measurement rather than by argument, and without needing to know what the
breadboard is wired like.

**Step 2 — fit the value that matches what step 1 shows.** Sergio's parts drawer
(top unit, ascending value order) already has **1 MΩ** and **220 kΩ**. They are
through-hole, but a THT resistor solders onto an 0805 pad pair with bent legs
perfectly well for seven parts.

| | signal with a GND clip | clipless | firmware |
|---|---|---|---|
| empty pads (floating) | large, messy | works — this is the V4 front end | polarity **flips**: touch reads UP |
| **1 MΩ** | ~500 counts | 50 Hz swing, board must be earth-referenced | unchanged |
| 220 kΩ | ~430 counts | weak | unchanged |

1 MΩ is the recommendation: it keeps the firmware's polarity, so nothing has to
change in the game except the double-read below.

**Step 3 — `readKey()` must discard its first conversion.** Above ~10 kΩ source
impedance the ATmega's sample-and-hold has not settled when the conversion
starts, so a single `analogRead` after a mux switch carries the previous
channel's residue. The probe already does this; the game build does not. It is
two lines.

That trio is V5.6. Nothing about it re-fabricates the board.

## 2e. THE BOARD IS GOOD — proven 2026-09-13 with a piece of wire

A bare wire from J2 pin 1 (and then pin 2) to J2 pin 8 (GND), probe running.
Log: `pcb/validation/probe-v0.7.1-jumper-2026-09-13.txt`.

| channel | range over 176 s | bridged? |
|---|---|---|
| 1 | **1023 → 0** (220 samples below 900) | yes |
| 2 | **1023 → 0** (31 samples below 900) | yes |
| 3, 6 | 1023 → 1023 | no |
| 4, 5, 7 | 1023 → 1022 | no |

**Full-scale swing on exactly the two channels that were bridged, and nothing on
the other five.** A cleaner result is not available. It proves, end to end:

- `J2.1 → R1 → A0` and `J2.2 → R2 → A1` are continuous and functional;
- `J2.8 → GND pour` is continuous and functional;
- the ADC resolves the whole 1023-count range on these nets;
- there is no crosstalk — the five untouched channels stayed put.

**So the PCB is not broken, and it is not mis-designed in any way that stops a
key working.** Every hypothesis in §2, §2b, §2c and §2d that blamed the board is
now dead:

| Hypothesis | Status |
|---|---|
| Copper routing / netlist error | dead (§1, independent re-derivation) |
| Buzzer / J5 / stuck SENS buttons | dead (§2b — they all work) |
| R1–R7 missing or unpopulated | dead (§2c — pinned 1023 is a working pull-up) |
| R1–R7 bridged to +5 V | dead (meter: 220 Ω on all seven) |
| KEY nets shorted to each other | dead (meter: ~440 Ω key-to-key, which is R+R through the rail — the *expected* value) |
| J2 not soldered through | dead (continuity on all 8 pins, and this wire test) |
| **220 Ω too stiff for skin** | **still standing, but it must then be true of the breadboard too** |

Two incidental faults found on the way, both confirmed irrelevant to this: **R18
fitted as 220 Ω instead of 10 kΩ** (it only touches `/SENS_MINUS` and `/+5V`; with
the button up it behaves identically, and the button demonstrably works), and
**LED4 dead** (its anode is D5, a digital output sharing nothing with A0–A6).

### What is left, and it is not on the board

The fault is in the path from the header pin to the player's hand: the fruit, the
clips, the wire, or the return to GND. And the decisive comparison has still
never been made — **the probe has never been run on the breadboard.** That is now
the entire remaining question, because everything the board can be blamed for has
been measured and cleared.

> A note on method, for next time. This wire test costs thirty seconds and it
> should have been the FIRST thing done, before any theory about pull-up values,
> body impedance or earth references. It substitutes a known resistance for the
> human and turns an argument into a number. Instead the review spent two days
> reasoning from a table of skin resistances — correct arithmetic, aimed at the
> wrong question, because "is the board able to see a key at all?" was never
> asked. See [[la-resistencia-conocida-antes-que-la-teoria]].

## 2f. The breadboard, measured at last — and it works by 2 counts

Same probe, same Nano socket, same body, same supply, same USB cable into
quantumpc. Log: `pcb/validation/probe-breadboard-2026-09-13.txt`. The three
phases the operator described are visible in the timestamps without being told:
touch, a 20 s pause, then touch again while gripping the Nano's USB shell.

| phase | ch1 | ch2 | ch3–7 (never touched) |
|---|---|---|---|
| touching, **no ground** (4–24 s) | 1015 → **8 counts** | 1017 → **6 counts** | 1021–1023 |
| pause (24–36 s) | 1023 | 1023 | 1023 |
| touching, **holding GND** (36–60 s) | 994 → **29 counts** | 983 → **40 counts** | 1023 |

### The comparison, finally

| | breadboard | v0.7.1 board |
|---|---|---|
| touching, no ground | **6–8 counts** | 0 |
| touching, holding GND | **29–40 counts** | 0 |
| game's auto-margin | 4 | 4 |

**The working rig fires without a ground clip by a margin of two to four
counts.** That is the headline, and it reframes the whole investigation: this was
never a robust design that the PCB broke. It is a design with ~2 counts of
headroom, and on the PCB it fell off the edge.

The arithmetic from §2 was right and its conclusion was wrong in one specific
way: 220 Ω does give a usable signal — but only 6 counts of it, from a **28 kΩ**
contact (bare fingertip pressed on a bare pin). Holding ground drops the contact
to **5.4 kΩ** and buys 40 counts. Neither number is a dry finger's megohm,
because a fingertip pressed hard on metal is not a dry finger.

### Why this does not yet convict the PCB

The same finger cannot be 5 kΩ on one rig and >250 kΩ on the other. So either
the boards differ in some way not yet found, or **the gestures were not the
same** — on the breadboard it was a fingertip pressed on bare pin metal while
gripping the USB shell; the PCB runs were "fruit, the pin, before and after the
resistor" over 156 s, and may never have included that exact grip on bare J2
metal.

So the last experiment is a **back-to-back with one variable**: the identical
gesture on the v0.7.1 board, same finger, same USB shell, probe running.

- **~40 counts** → the board was always fine; the piano plays as soon as the
  player is grounded, and nothing needs desoldering.
- **0 counts** → there is something, and it is now isolated to a single
  repeatable difference with everything else held constant.

### Either way, one thing is now justified by data instead of by theory

**R1–R7 at 220 Ω leave 2 counts of headroom on the rig that works.** Raising them
turns 6–8 counts into hundreds and removes the ground clip from the equation. §2d
proposed that from arithmetic; the breadboard's own numbers now support it. It
stays queued behind the back-to-back, because a fix applied before the cause is
known is a coincidence, not a repair.

## 2g. R1 → 10 kΩ, measured twice — and the difference is finally a number

R1 was lifted and a 10 kΩ through-hole resistor hung externally from J2 pin 1.
Two runs, because the first one taught something on its own.

### Run 1: 10 kΩ to J1 ("5V IN") — a floating node, not a pull-up

Capture of 13:02 — **not preserved**: `~/sonda-pcb.txt` was overwritten by the next two runs before it was copied into `pcb/validation/`. The numbers below are from the analysis made at the time (766 samples, 86.4 s); the CHANGELOG entry of that hour carries them too. Every later run IS archived. Channel 1
swung **512 counts** (383…895); channels 2–7 flat. It looked like a triumph. It
was not a pull-up working — it was a pin left floating, and the file says so
three times:

1. **baseline 735** (≈ 3.6 V), not 1023 — nothing held the node at the rail;
2. the channel went **160 counts ABOVE its own baseline** — a pull-up can never
   push a pin above its rail, so the node was not held at all;
3. **noise 10 and a slow drift 732…744** — diode leakage moving with temperature.

J1 is the power *input*. Current flows `J1 → D2 (1N5817) → filter → rail`, and
when the board is powered from the Nano's USB with nothing in J1, the Schottky
isolates J1 from the rail — which is precisely what the 1N5817 is there for (`HARDWARE.md`, powering rule 2). So
the 10 kΩ was tied to a node held only by the reverse leakage of D1 and D2: a
10 kΩ antenna on an open pin. A floating node swings hundreds of counts when a
hand comes *near* it, through capacitive coupling, without any good galvanic
contact. That is the V4 "floating keyboard" the repo measured in July and called
unreadable. *It moved* is not *it worked*: the game needs a stable baseline and a
downward dip, and this went both ways.

### Run 2: 10 kΩ to the Nano's 5V pin — a real pull-up, and the real number

Log: `probe-v0.7.1-R1-10k-on-rail-2026-09-13.txt`, header `label: pcb`.
Baseline **1023, noise 0** on all seven — the resistor is on the rail now.
Channel 1's best dip: **14 counts** (1009); typical touches 1–2 counts. Channels
2–7 flat.

Solving `reading = 1023·Rc/(Rc+Rpu)` for the contact resistance, per capture:

| rig / condition | pull-up | reading | dip | **contact** |
|---|---|---|---|---|
| breadboard, "no ground" | 220 Ω | 1015 | 8 | **28 kΩ** |
| breadboard, gripping USB shell | 220 Ω | 983 | 40 | **5.4 kΩ** |
| PCB, 10 kΩ on rail, best touch | 10 kΩ | 1009 | 14 | **721 kΩ** |
| PCB, 10 kΩ on rail, typical | 10 kΩ | 1021 | 2 | **5.1 MΩ** |
| PCB, 10 kΩ on rail, weak | 10 kΩ | 1022 | 1 | **10 MΩ** |

**Same finger, same USB-shell grip: 5 kΩ on the breadboard, 0.7–10 MΩ on the
PCB.** Twenty-five to four hundred times worse. With that contact a 220 Ω
pull-up gives 0.02–0.3 counts, i.e. the exact 0 measured for three days — and
even 10 kΩ only buys 14. **The circuit and the value are no longer the question.
The contact at the header pin is.**

Two things the same table settles:

- The breadboard's "clipless" 28 kΩ is not physically possible through
  capacitance at DC; there was a real return path — most likely a neighbouring
  finger or the palm on an exposed GND jumper tip, of which a breadboard has
  dozens. The PCB, with solder mask everywhere and eight isolated pins, offers
  no accidental ground, and its "no ground" reading (1–2 counts, 5–10 MΩ) is
  what genuinely clipless touch looks like. **The finished piano, in a box, will
  behave like the PCB, not like the breadboard.**
- With the finger gripping the USB shell on both rigs, the return path is the
  same; the remaining 100× gap is **at the pin surface**: J2 was hand-soldered
  from below and rosin flux wicks up header pins as a hard insulating varnish;
  the breadboard's pins and jumper tips are bare tinned metal.

### The decisive test is also how the piano is actually played

Nobody plays this instrument with a fingertip on a 0.64 mm header pin. The
fruit reaches the board through an **alligator clip**, which bites through any
film and offers a large contact area. So:

1. Alligator clip on **J2 pin 1**, another on **J2 pin 8** (GND). Hold the pin-8
   clip's metal in one hand; touch the pin-1 clip's metal — or a lemon clipped
   to it — with the other. Probe running. Expected with the 10 kΩ and a 5.4 kΩ
   body: **~660 counts**. With 28 kΩ: **~270**.
2. Optionally, wipe the J2 pins with isopropyl alcohol and repeat the fingertip
   test — separates "flux" from "contact area".

If (1) gives hundreds of counts, the case is closed: board correct, 10 kΩ
correct, ground clip required, pins were varnished. If it still gives ~10, the
remaining suspect is the clip/wire path itself, and it is measured with the
meter in ohms in ten seconds.

### What "clipless" would actually take

The floating-node run accidentally built the clipless piano the owner wants — and
showed why it is hard: no baseline, drift, movement both ways. The controlled
version is the MaKey-MaKey arrangement: pull-ups in the **1–5 MΩ** range (so a
5–10 MΩ body-to-earth path still yields 100–500 counts), `readKey()` discarding
the first conversion, and the adaptive `trackBaselines()` the firmware already
has doing real work. That is a design change (V5.6+), not a repair, and it trades
sensitivity for ghost notes. With the 10 kΩ + a GND clip the instrument is
rock-solid today; clipless is the next version's question.

## 2h. SOLVED — R1 at 1 MΩ, 15 clean presses, clipless

Log: `pcb/validation/probe-v0.7.1-R1-1M-SOLVED-2026-09-13.txt`, header
`label: pcb`. R1 replaced by an external 1 MΩ from J2 pin 1 to the Nano's 5V
pin. Baseline **1022, noise 2** → the game's auto-margin would be **4**.

Event detection run at that real threshold finds **15 presses on channel 1**,
and the operator's own account of the session falls out of the timestamps:

| phase | presses | dip |
|---|---|---|
| 0–20 s, no ground, shod | 5 | **132–200** |
| 20–36 s, gripping ground | 5 | **255–291** |
| 44–56 s, barefoot on the floor | 5 | **166–258** |

Worst press of the fifteen: **132 counts against a margin of 4 — 33× headroom.**
Best: 291, i.e. 72×. Presses last 300–1500 ms and return cleanly to 1023 between
them. **Channels 2–7 sat at exactly 1023 for the whole 90 s**: no crosstalk, no
phantom notes, no stuck keys.

### Barefoot matters, and it does not matter

The operator noticed mid-session that he was barefoot on the floor and asked
whether it counted. It does, and the three phases measure it: standing on the
building's floor gives a **partial** return to earth — better than isolated
(132–200), worse than a hand on circuit ground (255–291), landing in between
(166–258). Exactly what the physics predicts, measured by accident.

But every one of the three conditions clears the threshold by more than thirty
times over, so in practice it is irrelevant: shoes, bare feet or a rug all play.

### What this closes

The three-day zero was never a broken board and never a wrong netlist. It was a
**220 Ω pull-up used as a body-resistance sensor**: it needs the whole hand →
body → fruit path under ~56 kΩ, and a clipless touch on this board measures
**5–10 MΩ**. That is four orders of magnitude, which is why every intermediate
theory — routing, bridges, flux, the fruit header, the buzzer — was chasing a
number that no assembly fault could have produced. §2f's breadboard reading (6–8
counts clipless) was the same design one bad contact away from the same silence;
it worked on 2 counts of headroom and the PCB fell off the edge. **1 MΩ replaces
2 counts of headroom with 33×.**

### Remaining work, in order

1. **Swap R2…R7 to 1 MΩ** (R1 is already done, externally). Keep R18 out of it —
   it is the SENS− pull-up and 220 Ω there is harmless (§2c).
2. **Re-run the probe with all seven high-impedance.** This is the one thing the
   current capture cannot predict: channels 2–7 are still 220 Ω, which makes them
   immune to the coupling a 1 MΩ neighbour could induce. If ghosting appears,
   **470 kΩ** is the fallback — it still yields ~65–145 counts on the measured
   contact, with a quarter of the impedance.
3. Flash the game firmware, which already discards its first ADC conversion (that
   change is what makes a MΩ front end readable at all) and play it on fruit.
4. Then, and only then, decide whether V5.6 exists as a board revision.

## 2i. The quiet-rest control: zero ghosting at 1 MΩ

The one thing §2h could not answer was whether a 1 MΩ node ghosts when nobody is
near it — the earlier chatter (11 retriggers in 0.6 s, and the margin walking
from 4 to 76) was suspected to be the operator's own hands, but suspicion is not
evidence. He then left the rig untouched and said so, which is the control this
needed. Unattended batch, nobody in the room:

**The game, 4 minutes at rest** (`game-v0.7.1-quiet-rest-4min-2026-09-13.txt`):
after `Level 1`, **zero lines**. No phantom notes, no margin nudges, nothing.

**The probe, 4 minutes at rest**, 2087 samples
(`probe-v0.7.1-quiet-rest-4min-2026-09-13.txt`):

| channel | min | max | excursion | pull-up |
|---|---|---|---|---|
| 1 | 1023 | 1023 | **0** | **1 MΩ** |
| 2–7 | 1023 | 1023 | 0 | 220 Ω |

**100.00 % of the 1 MΩ channel's 2087 samples are exactly 1023.** Not one count
of movement in four minutes; zero samples below the game's 1019 threshold.

So the earlier chatter was hands, confirmed from both ends, and the decision
falls out:

| | |
|---|---|
| press signal (§2h) | **132–291 counts** |
| rest noise at 1 MΩ | **0 counts / 4 min** |
| game threshold | **4 counts** |

**Do not raise `TOUCH_HYSTERESIS`. Do not drop to 470 kΩ.** 1 MΩ is the answer,
with room on both sides. The hysteresis worry in §2h was real arithmetic aimed at
a symptom that turned out to be a human — worth recording precisely because the
temptation was to fix it before measuring it.

One caveat survives: six of the seven channels are still 220 Ω, so this cannot
show coupling *between* MΩ neighbours. After the swap, repeat this same
quiet-rest run — as a confirmation now, not a gate.

## 3. The buzzer branch — CLOSED 2026-09-12, it works (kept for the reasoning)

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

## 4. What to measure, in order — revised 2026-09-12

Steps 1–4 of the original list are **done and passed** (§2b): it beeps at boot,
the buttons beep, the rail reaches the pull-ups. What is left is one experiment
with two runs, and the second run is the one that matters.

1. **Flash `firmware/probe/` to the assembled board.** Touch the fruit exactly as
   you play — no clip, as you always do. Let it run a minute and read the
   `verdict:` line.
2. **Flash the same probe to the working breadboard.** Same sketch, same Nano,
   same baud. Touch it the same way.

Then compare the two `-- since boot   DOWN: … UP: …` lines:

| Board | Breadboard | Conclusion |
|---|---|---|
| nothing moves | moves **UP** | Different front ends. The breadboard is V4/V4.5 (floating pin, series resistor); the PCB is V5 (pull-up, needs a GND reference). §2b. |
| nothing moves | moves **DOWN** | Same topology, and something on the board is killing the signal. That is the only outcome that puts the fault back in the PCB itself, and it is the one to escalate. |
| moves **DOWN** but small | moves **DOWN** bigger | Same topology, signal attenuated. Tune the margin down with SENS− and measure how far you have to go. |

3. **Free and worth doing in the same session:** power the board from an
   **earthed PC's USB** rather than a charger, and retry. If clipless play starts
   working, the missing ingredient was the earth reference, not the resistor —
   and V5.5's own powering rule is what removed it (§2b).

Nothing needs desoldering until those two runs disagree.

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

---

## §3. The SENS − button played a note (2026-09-13)

All seven pull-ups are now 1 MΩ and R18 is a real 10 kΩ. Every key sounds; the
dead LED 4 came back with it. Sergio then reported a new symptom:

> pressing the **−** button beeps like a key, and holding it on gives the error
> sound, as if a lemon had been touched. At the bottom of the LED bar every
> press sounds a key; past the fifth LED it stops and the button behaves.

### Why the − button and not the +

| | pin | how it is read | on the ADC mux |
|---|---|---|---|
| SENS **+** | D12 | `digitalRead` | no |
| SENS **−** | **A7** | **`analogRead(A7) < 512`** | **yes** |

A7 is analog-in only — it has no internal pull-up, which is why R18 exists at
all — and it is **channel 7 of the same multiplexer that carries the seven
keys**. `serviceButtons()` reads it at the top of every `loop()`, immediately
before the key scan starts at A0.

While the button is held, **A7 sits at 0 V**. The ATmega's sample-and-hold cap is
therefore handed to key 1 holding **zero**, not 5 V.

### Why 1 MΩ turned that into a note

The cap refills through whatever drives the pin:

| pull-up | τ = R × 14 pF | against the 12 µs sampling window | one conversion closes |
|---|---|---|---|
| 220 Ω | 3 ns | 4000 τ | **all of the gap** |
| 1 MΩ | 14 µs | 0.86 τ | **≈ 57 % of the gap** |

At 220 Ω the residue was gone before the first conversion finished, so this bug
could not exist on the board as fabricated. At 1 MΩ a single conversion is not a
reading, it is a step towards one — and `readKey()`'s single discarded
conversion, the fix from §2h, is only the first step.

Reproduced on the host, no hardware involved
(`firmware/test/piano_sim_test.cpp`, section 14, the ADC model in
`test/arduino/Arduino.h` now settling exponentially at 0.575 per conversion):

```
readKey(0) = 944   threshold = 1016
```

**72 counts below the trigger point.** A phantom key 1, every loop, for as long
as the button is down. Which also explains the LED bar: the droop is finite, so
a margin wide enough — about halfway up the bar — simply hides it. That was
never the button behaving, it was the margin covering it.

### The fix

`readKey()` converts the channel **until two readings agree** (≤ 1 count apart,
hard stop at 12), then averages four. Not a bigger constant: a settled 220 Ω
channel breaks out on the second conversion and costs 112 µs, the 0 V → 5 V
worst case on 1 MΩ takes eight and costs 0.9 ms, and **nothing in it is tuned to
a particular pull-up value** — which matters, because these resistors have now
changed twice.

Proof, in both directions:

- Tests 13 + 14 green, 68 checks, 0 failed.
- **Mutation control:** put the single discard back and test 14 fails 3 of its
  6 checks, including `readKey(0)` against its threshold.
- `pio run` clean on `nanoatmega328`, `nanoatmega328new` and `emulation`;
  15492 B.
- Flashed and verified on the board; it boots and calibrates all seven at
  baseline 1023 / noise 0 / margin 4.

### Verified on the board (2026-09-13)

Sergio pressed the **−** button on the real rig, repeatedly, with the fixed
firmware on it: **no key sounds.** Asked and answered through
`qpc-approvals piano/boton-menos-a7` → *approved: "Ya NO suena"*.

That also settles the competing hypothesis. If the note had been his body
capacitively coupling into a 1 MΩ node while he held the button, a firmware
change to how many times a channel is converted could not have removed it. It
was the multiplexer.
