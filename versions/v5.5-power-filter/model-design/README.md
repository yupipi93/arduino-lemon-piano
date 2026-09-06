# Enclosure design (model-design)

`reference-images/` are Sergio's caliper-annotated photos of the parts that
must fit in the box besides the PCB: the speaker (27.8 × ~24.5 × 15.1 mm,
mounting-ear holes 30.7 mm apart), the LM386 amplifier module (40.7 × 13.8,
13.6 tall), the optional USB-C keep-alive gadget (27.7 × 10.9, 13.6 tall),
the wanted left-to-right layout (`comp distribution.png`) and the openings
(`holes.png`).

`enclosure-v2/` is the resulting design, generated from the parametric
recipes in the 3D-design repo (`3d-modeling-agent`, `recetas/piano-limones-caja-v2`
and `recetas/piano-limones-luna-v2` — that repo is the source of truth; the
STLs here are copies of its gate output, 2026-09-06, third revision of the day):

| File | What | Print |
|---|---|---|
| `base.stl` | box body, 208.0 × 62.4 × 29.6 mm | upright, 66.46 g, 1 h 51 m |
| `tapa.stl` | lid, modelled face-down as printed | 30.76 g, 1 h 05 m |
| `luna.stl` | the fruit arc, 188 × 94 × 16 mm, wire channels + connector pocket underneath | 36.85 g, 1 h 10 m |

Fit is against **pcb v0.7.1**: 4 × M2 standoffs at 110 × 30 mm, Nano window
+ mini-USB cable channel in the lid, 10-LED slot, buzzer hole, trimmer hole
over the amp, slotted speaker grille (frame 30.7 × 27.8, ear holes 36 apart,
43.7 tip to tip), **the two threaded 5 mm panel buttons in the lid** over the
east block of the board (J3/J4 are not populated; 25.2 mm free under the lid
face for their 20 mm), a notch for the key ribbon over J2, **power inlet on the
west (short) wall**: the inventory's 4-pin USB-C panel jack (`con-usbc-jack-4p`)
plus a Ø8 hole for a direct cable, side by side; and the **Multitec logo**
(hexagon + MULTITEC, raised inside a recessed plaque) across the lid's front
strip. Every hole carries a safety margin (+0.5 vertical, +0.8 in walls,
0.3/side for modules). Renders in `renders/`.

Dimensions to confirm on the physical parts before printing (all parameters
of the recipe): the USB-C jack's screw spacing and opening (its listed
"datasheet" is a Garmin cable manual — 16 mm / 9.6 × 3.8 assumed), the
speaker ear hole diameter (M3 self-tapping assumed), and the height of the
socketed Nano's ICSP header (sets the 22 mm clearance over the PCB).
