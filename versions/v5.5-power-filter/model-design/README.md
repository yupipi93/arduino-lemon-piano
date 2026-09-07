# Enclosure design (model-design)

`reference-images/` are Sergio's caliper-annotated photos of the parts that
must fit in the box besides the PCB: the speaker (27.8 × ~24.5 × 15.1 mm,
mounting-ear holes 30.7 mm apart), the LM386 amplifier module (40.7 × 13.8,
13.6 tall), the optional USB-C keep-alive gadget (27.7 × 10.9, 13.6 tall),
the wanted left-to-right layout (`comp distribution.png`) and the openings
(`holes.png`).

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
