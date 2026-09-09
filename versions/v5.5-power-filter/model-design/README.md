# Enclosure design (model-design)

`reference-images/` are Sergio's caliper-annotated photos of the parts that
must fit in the box besides the PCB: the speaker (27.8 × ~24.5 × 15.1 mm,
mounting-ear holes 30.7 mm apart), the LM386 amplifier module (40.7 × 13.8,
13.6 tall), the optional USB-C keep-alive gadget (27.7 × 10.9, 13.6 tall),
the wanted left-to-right layout (`comp distribution.png`) and the openings
(`holes.png`).

`enclosure-v4/` and `fruit-arc-v4/` are the **current** design. `enclosure-v2/`
is what was physically printed on 2026-09-07/08 and is kept as the record.
Everything is generated from the parametric recipes in the 3D-design repo
(`3d-modeling-agent`, `recetas/piano-limones-caja-v4` and
`recetas/piano-limones-luna-v4` — that repo is the source of truth; the STLs
here are copies of its gate output).

## v4 of the box (2026-09-09): the PCB rises to the lid

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

## v4 of the fruit arc (2026-09-08): oranges, puzzle joints, numbers

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
