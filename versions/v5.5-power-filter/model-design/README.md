# Enclosure design (model-design)

`reference-images/` are Sergio's caliper-annotated photos of the parts that
must fit in the box besides the PCB: the speaker (27.8 × ~24.5 × 15.1 mm,
mounting-ear holes 30.7 mm apart), the LM386 amplifier module (40.7 × 13.8,
13.6 tall), the optional USB-C keep-alive gadget (27.7 × 10.9, 13.6 tall),
the wanted left-to-right layout (`comp distribution.png`) and the openings
(`holes.png`).

`enclosure-v6/` and `fruit-arc-v5/` are the **current** design. `enclosure-v4/`
is superseded — kept only because its README section explains how the board got
raised in the first place. `enclosure-v2/`
is what was physically printed on 2026-09-07/08 and is kept as the record.
Everything is generated from the parametric recipes in the 3D-design repo
(`3d-modeling-agent`, `recetas/piano-limones-caja-v4` and
`recetas/piano-limones-luna-v4` — that repo is the source of truth; the STLs
here are copies of its gate output).

## v6 of the box (2026-09-10): three fixes found with the parts in the box

Three corrections Sergio found once the v5 box was printed and populated. All
three are the same lesson: **a component modelled as a plain brick hides its
real features.**

| File | What | Print |
|---|---|---|
| `enclosure-v6/tapa.stl` | lid — **needs reprinting** | 33.34 g, 1 h 09 m |
| `enclosure-v6/base.stl` | base — changed, but see below | 63.79 cm³ |

1. **LED slot 5.0 → 5.5 mm.** *"A touch wider, barely half a millimetre, so the
   LEDs go in properly — right now they rub a little and that could cause
   friction opening and closing the lid."* They rubbed because of v5 itself: with
   the board raised the LEDs are no longer soldered proud, so the slot stopped
   being a peephole and became the **socket** the LED head enters. 5.5 leaves
   2.0 mm of real air, 1.0 per side.
2. **The volume trimmer is not on the module's centreline.** *"You worked the
   hole out as if the pot were in the middle of the board."* It was an
   assumption of mine, not a measurement. Measured **from his photo** — blue
   trimmer and board edges segmented by colour, scaled by the **14.4 mm between
   the cradle rails** (a model dimension, not a guess): the trimmer axis sits
   **3.0 mm west** of the module axis. The access changes from a Ø6.4 circle to
   a **10.0 × 6.4 slot** shifted west — a slot rather than a circle because a
   handheld photo is easily ±0.5 mm out and looking down on a 13.6 mm-tall part
   makes it worse, so the screwdriver still reaches if I'm 2 mm off.
3. **The amp's screw terminal fouled the cradle stop.** *"On the side opposite
   the pins it has the speaker input, and right now it hits the wall a bit —
   you need to take that wall down."* The wall is the cradle's 8 mm south stop;
   the terminal sits **on top of** the board (top face at z 6.1), so the stop
   covered the 3.9 mm where the wires enter. Both amp stops now come up only to
   **the board's top face** — they stop the board's **edge** and leave anything
   soldered above it clear — and they are set back **1.5 mm** instead of 0.3 so a
   terminal overhanging the edge passes over them.

The screw terminal now has its **own envelope** in `comprobar_encaje.py` (10.2 ×
7.5 × 9.0 on the board's top face, overhanging 1.5 mm — the worst case the design
accepts), and the check reports what the wire has to the south: **1.4 mm worst
case** before the x=150 screw column, 2.9 mm if the terminal is flush. Moving
that column would fix it but would force reprinting the base too, because the
lid's countersinks would no longer line up with the already-printed base.

**The base does not need reprinting**: the only change is 4 mm of height off two
1.35 mm walls, which comes off the printed part with a file in a minute.

## v5 of the box (2026-09-09): the lid becomes a mask, the box drops 6.2 mm

Sergio's second pass the same day: *"a hole with plenty of clearance in the lid
for the two capacitors, the choke and the diodes, and for the 7-pin connector.
Also holes for the internal buttons, in case the panel ones fail. The point is
to redesign the box lower, with the PCB as close to the lid as you possibly
can, so its components show. Adjust the speaker and the amplifier so they don't
collide and you can cut height."*

| File | What | Print |
|---|---|---|
| `enclosure-v5/base.stl` | box body, 219.2 × 66.9 × **23.4 mm** | upright, **68.78 g, 2 h 09 m** |
| `enclosure-v5/tapa.stl` | lid, modelled face-down as printed | **33.43 g, 1 h 09 m** |

The conceptual shift: once the tall parts have holes, the ceiling is no longer
set by the tallest component but by whatever has **no** hole — which is the
**M2 screw head** on the standoffs, 1.5 mm.

- **Standoffs 9 → 17.3 mm**: the PCB's top face rises to **z 20.9**, i.e.
  **2.5 mm** below the lid's inner face. `base_alto` 29.6 → **23.4**; assembled
  height 32.8 → **26.6 mm**.
- **Six openings** over the board: the two capacitors in **one** hole (as
  asked — they are 10 mm apart and Ø8, so two holes would leave a 2 mm rib),
  the filter block (choke + both diodes + the 5 V input) in another, the key
  connector (the "7-pin" one is really a 1×8: 7 keys plus the player's ground
  clip), a slot serving **both internal tact switches**, which merges with the
  Nano window, and the buzzer — whose Ø12 body now passes through instead of
  speaking through a 3.4 sound hole. Clearance 1.2 mm per side on top of
  process compensation.
- **The mini-USB channel is gone**: with the board up there, the Nano's
  connector sits *above* the visible face, so the channel only weakened the lid.
- **The LEDs no longer need to be soldered raised** — the v4's one awkward
  assembly step. A short-legged Ø3 LED now ends inside the lid's 3.2 mm and
  shines through its slot.
- **Panel buttons go back side by side** (− left, + right): two Ø11 nuts in a
  column need 24 mm of wall and the wall is 23.4. Their bodies pass *under* the
  board now.
- **The collision Sergio predicted was real and measured**: 11.5 mm³ between
  the speaker's cable and the amplifier module. In v4 the cable exited above the
  amp (z 22.2 vs its 18.1 top); lowering the box drops it to z 16.0, level with
  it. The amp-to-collar gap goes 1.5 → 5.0 mm and the east gap 7.0 → 5.0, so the
  box grows just 1.5 mm in length, with an assert per speaker orientation.

What now limits the height, in order: the amp's **straight Dupont header**
(z 22.6, 0.8 mm under the lid) and the **speaker's back** (z 4.9, 2.9 mm off
the floor).

**One assembly condition:** the speaker output J5 must use a soldered wire or a
**right-angle** connector, never a straight Dupont (16.5 mm won't fit). It is
the only header without a hole, because opening it would merge three openings
into one 70 mm gap and leave the lid's north edge as an unsupported 5 mm rib.
It gets a 2 mm relief pocket in the inner face instead.

Components that deliberately stand proud of the visible face: the socketed Nano
with its ICSP header (+12.9 mm), the key connector with its Dupont (+10.8) and
the two capacitors (+9.3).

## v4 of the box (2026-09-09, superseded): the PCB rises to the lid

Sergio found the flaw with the printed box in his hands: with the board sitting
at the bottom, the Nano window and the LED slot in the lid looked at 22 mm of
air. So the board goes up.

| File | What | Print |
|---|---|---|
| `enclosure-v4/base.stl` | box body, 217.7 × 66.9 × 29.6 mm | upright, **74.44 g, 2 h 13 m** |
| `enclosure-v4/tapa.stl` | lid, modelled face-down as printed | **34.40 g, 1 h 10 m** |

- **Standoffs 4 → 9 mm**: the top face of the PCB moves from z 7.6 to **z 12.6**.
  The ceiling is set by the **Dupont plugged into the pin strips (16.5 mm)**,
  not by the Nano: they end 0.5 mm below the lid. The Nano reaches 31.2, i.e.
  **1.6 mm below the visible face** — visible through its window and swappable
  without opening the box. The 10 LEDs must be **soldered 11.6 mm raised** to
  reach the slot, which is what Sergio planned. The tallest filter part, the
  capacitor, is 15 mm (his measurement) and clears by 2 mm.
- **The two panel buttons leave the lid** (a 20 mm body would hit the board)
  and move to the **front wall in a column**: **+ at z 21, − at z 8**, hole
  Ø6.3 = 5.0 thread + 0.8 process compensation + 0.5 assembly clearance, each
  with its sign in relief beside it.
- **18 mm front strip** (interior depth 62.5, +4.5 mm overall): it takes the
  button bodies (14.5 mm with pins) and the **16.3 × 30 ferrite** of the power
  lead, lying on the floor between two 4 mm stops.
- **LED slot narrowed 6.4 → 5.0**: it now guides the raised LEDs.
- **The lid stays single-level.** Sergio asked whether a stepped lid was worth
  it; with the board raised there is no clearance left to recover, and a step
  would add two overhangs and a visible seam to a part that today prints with
  no supports at all (1.59% overhang).

Everything else is as in v3: engraved logo with a checked 2 mm keep-out, DIN 562
square nuts in the six columns, centred key-ribbon slot, west power inlet
separated from the USB-C jack, speaker-cable notches on **both** sides of the
collar, 7-slot modern grille, cradles for the USB gadget and the LM386 with
trimmer access.

## v5 of the fruit arc (2026-09-09): the joint actually locks now

Sergio, with the v4 arc **assembled** in his hands: *"the puzzle pieces go in
and out without having to be fitted from above, and that means I'd need glue.
Make the puzzle tab more pronounced so it has to go in top-down and can't be
pulled apart once joined."*

**The neck size was never the problem, and that was the trap.** v4 had a Ø7.0
head against a 5.6 neck, and the check passed it — because it compared a
*formula* (`2·radius` vs `2·sqrt(radius² − offset²)`) and ignored the **relief
holes**. Those two holes sat **on the joint line itself**, and with their radius
plus clearance they opened the female's mouth to **7.70 mm** — wider than the
male's 7.10 head. The head simply walked out. Simulating extraction on the real
profile: **0.00 mm of deformation needed**.

The new profile is a **keyhole**: a **straight 4.4 mm neck** (a rectangle — the
only thing that sets the mouth) and a **Ø8.4 head** pushed 5.2 mm past the
joint line. Mouth 4.70 vs head 8.10 → **3.40 mm of deformation** would be needed
to pull it apart, so it can only be assembled and disassembled by **lowering one
piece onto the other**. No glue.

The relief holes are gone on purpose. They existed because the v4 circle crossed
the joint line at a very acute angle; in the keyhole that corner meets at **122°**
measured in the material. And here they did harm twice over: they opened the
mouth (the bug) and, at 0.9 mm, they poked past the head's arc leaving a
**0.63 mm** fin — 13.4 mm², caught by the gate on this very v5's first attempt.

And the part that actually mattered: `comprobar_puzzle.py` no longer compares a
formula. It **simulates the extraction** on the profile the recipe draws,
sweeping the tenon along the arc axis and measuring the deformation needed at
each position, requiring at least 2.0 mm — with a **new positive control**: the
same computation on the v4 profile must return 0.00 mm, i.e. reproduce the
failure Sergio is holding. Without that control, "3.40 mm" would prove nothing.

Nothing else changed: cups, numbers, wire channel and the rest are as in v4. The
longer tenon grows the west piece from 123.44 to 126.76 mm and the centre from
183.93 to 187.58.

| File | What | Print |
|---|---|---|
| `fruit-arc-v5/oeste.stl`, `este.stl` | end segments (2 cups each) | 35.50 / 34.36 g |
| `fruit-arc-v5/centro.stl` | middle segment (3 cups) | 57.90 g |

## v4 of the fruit arc (2026-09-08, superseded): oranges, puzzle joints, numbers

| File | What | Print |
|---|---|---|
| `fruit-arc-v4/oeste.stl`, `este.stl` | end segments (2 cups each) | 35.05 / 34.31 g |
| `fruit-arc-v4/centro.stl` | middle segment (3 cups), rotated to fit the bed | 57.35 g |

Open 140° arc, radius 226.7, seven cups **85 mm apart** with a Ø34 mouth so a
fat orange sits without touching its neighbour; **puzzle joints** (peg and
socket, 0.30 mm diametral clearance) instead of dowels, so no loose parts;
**numbers 1–7 on inclined wedges** on the outside, reading upright from outside
the arc; and the wire channel **extended past the end nail holes** so the nail
goes fully in — the four corrections Sergio asked for with the v3 arc printed.

## The printed record (v2 box + v3 arc)

`enclosure-v2/` is the resulting design, generated from the parametric
recipes in the 3D-design repo (`3d-modeling-agent`, `recetas/piano-limones-caja-v2`
and `recetas/piano-limones-luna-v3` — that repo is the source of truth; the
STLs here are copies of its gate output, 2026-09-07):

| File | What | Print |
|---|---|---|
| `base.stl` | box body, 208.0 × 62.4 × 29.6 mm | upright, 66.46 g, 1 h 51 m |
| `tapa.stl` | lid, modelled face-down as printed | 30.7 g, 1 h 05 m |
| `oeste.stl`, `este.stl` | fruit arc, end segments (2 cups each), 116 × 133 mm | 23.7 g / 42 m each |
| `centro.stl` | fruit arc, middle segment (3 cups), rotated 40° to fit the bed | 39.5 g, 1 h 11 m |
| `espiga.stl` | Ø5 × 16 dowel joining the segments — print **4** | 0.4 g each |

**The fruit arc** is now an open 140° arc of radius 226.7 mm — seven cups
**85 mm apart along the arc** so that fat oranges (Ø ≤ 80) never touch — 456 ×
171 mm assembled, hence the three segments and dowels. The wiring runs
underneath: an arc channel under the seven nails, a 26 × 18 pocket for the two
ribbon connectors (5 + 3 pins) under the middle cup, and a 12 mm entry channel
that exits on the **inner** (concave) side, so the box sits inside the arc and
the ribbon enters behind the fruit.

**The box** fits **pcb v0.7.1**: 4 × M2 standoffs at 110 × 30 mm, Nano window +
mini-USB cable channel in the lid, 10-LED slot, buzzer hole, trimmer hole over
the amp, slotted speaker grille (frame 30.7 × 27.8, ear holes 36 apart, 43.7
tip to tip), the two threaded 5 mm panel buttons in the lid over the east block
of the board (J3/J4 unpopulated; 25.2 mm free under the lid face for their
20 mm), a notch for the key ribbon over J2, power inlet on the west (short)
wall (the inventory's 4-pin USB-C panel jack `con-usbc-jack-4p` plus a Ø8 hole
for a direct cable), the Multitec logo raised inside a recessed plaque across
the lid, and "SUPER" / "LEMON PIANO" in Press Start 2P on the front wall. Every
hole carries a safety margin (+0.5 vertical, +0.8 in walls, 0.3/side for
modules), and `comprobar_encaje.py` in the recipe folder proves by boolean
intersection that none of the 40 component envelopes touches the box or each
other (with a positive control). Renders in `renders/`.

Dimensions to confirm on the physical parts before printing (all parameters
of the recipe): the USB-C jack's screw spacing and opening (its listed
"datasheet" is a Garmin cable manual — 16 mm / 9.6 × 3.8 assumed), the
speaker ear hole diameter (M3 self-tapping assumed), the height of the
socketed Nano's ICSP header (sets the 22 mm clearance over the PCB), and the
cup mouth (32) against a real orange.
