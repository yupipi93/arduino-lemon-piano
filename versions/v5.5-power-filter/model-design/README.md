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
STLs here are copies of its gate output, 2026-09-06):

| File | What | Print |
|---|---|---|
| `base.stl` | box body, 205.1 × 62.4 × 29.6 mm | upright, 66.00 g, 1 h 51 m |
| `tapa.stl` | lid, modelled face-down as printed | 29.88 g, 59 m |
| `luna.stl` | the fruit arc, 188 × 94 × 16 mm, wire channels underneath | 37.57 g, 1 h 09 m |

Fit is against **pcb v0.7.1**: 4 × M2 standoffs at 110 × 30 mm, Nano window
+ mini-USB cable channel in the lid, 10-LED slot, buzzer hole, trimmer hole
over the amp, slotted speaker grille, two threaded 6 mm panel buttons **centred
and aligned** on the south (player's) wall (16 mm apart, the PCB sits 13.5 mm
back from that wall to leave room for their bodies) wired to `EXT+`/`EXT−` (J3/J4), a notch for the key
ribbon over J2, and the USB-C port of the gadget through the north wall as
the power inlet. Renders in `renders/`.

Dimensions to confirm on the physical parts before printing (all parameters
of the recipe): speaker frame height (24.5, read off the photo, not a caliper
value), speaker ear hole diameter (M3 self-tapping assumed), and the height
of the socketed Nano's ICSP header (sets the 22 mm clearance over the PCB).
