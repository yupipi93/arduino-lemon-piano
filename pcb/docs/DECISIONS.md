# DECISIONS.md — Lemon Piano V5.5 Board

Every design choice made autonomously (no user in the loop), with rationale.
Circuit facts were **never** decided here — they come from
this repo's `versions/` docs (see [NETLIST.md](NETLIST.md)).

## ADR-001 — Pin map source: builder contract over stale BOM rows

The V5 `HARDWARE.md` BOM table says "Passive buzzer — D8" and "SENS + on
D7"; the 2026-07-28 pin map in the same file and the DRC-validated
`build_v5_5()` wirewright contract both say **buzzer = D13, SENS+ = D12,
SENS− = A7**. The contract + pin map win (they agree with each other and
with the firmware pin constants); the BOM rows are stale leftovers.

## ADR-002 — Nano socket orientation: mini-USB faces the WEST edge

The 2×15 socket rows run along X (a 43 mm Nano cannot sit across a 30 mm
board). USB-west puts D2..D12 on the SOUTH row → LED bar directly south
with short fanning traces, and A0..A7 on the NORTH row → key header
directly above the analog pins with straight 4 mm drops. USB-east was
evaluated and rejected: it mirrors the rows and makes the LED-bar and
key-header traces cross the board. H1 sits under the USB overhang
airspace (connector is 8.5 mm above board on the socket; M2 screw head
~2 mm — no interference).

## ADR-003 — Power entry on a screw terminal (not a barrel jack)

The v5.5 BOM says "5 V input pigtail / barrel jack — chop a USB-A cable:
red = V+, black = GND". A 2-pos 5.08 mm screw terminal (Phoenix MKDS
1,5/2) accepts exactly that pigtail without a crimped plug, matches the
board-spec wording "jack/terminal", and is the sturdier choice for a
board that gets handled by piano players. Wire entry faces north (off
the board edge).

## ADR-004 — Lemon-key interface: 1×8 pin header on the NORTH edge

Spec allowed "screw-terminal blocks or pin headers". An 8-position screw
block (5.08 mm: 41 mm wide, ~10 mm deep; 3.5 mm: ~8 mm deep) cannot fit
in the 6.5 mm strip between the north edge and the Nano's top socket row
without colliding with it — and moving the Nano south starves the LED
bar. A 1×8 2.54 mm pin header fits directly above A0..A6 (zero-crossing
straight drops), and the lemon leads already terminate in jumper-style
ends in the breadboard build. Pin 1 = GND (clip line, west), then
KEY7..KEY1 — each key pin sits exactly above its Nano analog pin. Every
position is silk-labelled (G, 7..1) per the "labelled" requirement.
Also: the mission text says "8 lemon-key lines + GND clip line"; the
circuit truth is **7** key lines (A0–A6) + 1 GND clip = 8 terminals.

## ADR-005 — LEDs: 3 mm THT green (LED_D3.0mm)

The source BOM says only "Green LEDs ×10". CONVENTIONS §5 default for
status LEDs is 0805 SMD; the breadboard build uses THT. 3 mm THT is the
middle ground: hand-solderable, bright, and 10 of them fit the south
strip at 4.6 mm pitch (5 mm LEDs need ≥5.5 mm pitch — they did not fit
alongside the SENS buttons). Lead pitch is 2.54 mm either way, so the
breadboard's existing LEDs still fit the holes if they are 3 mm parts.

## ADR-006 — Filter parts: vertical axials, radial electrolytics, B.Cu ceramics

- P6KE6.8A / 1N5817 mounted **vertical** (D_DO-15/D_DO-41 P5.08 vertical):
  a 30 mm-tall board is area-limited, not height-limited.
- 470 µF/16 V as 8 mm radial (CP_Radial_D8.0mm_P3.50mm) per "electrolytics
  THT radial" in the board spec.
- L1 = Fastron 07HCP footprint (8.7 mm radial drum, the 07HCP-101K is a
  real 100 µH / 1.3 A part satisfying "≥1 A, low DCR").
- C2/C4 (100 nF ceramics) on **B.Cu** under their electrolytics: the top
  east block is full; a 0805 with two short vias adds ~1 nH, irrelevant
  for a 700 Hz pi filter whose job is conducted-transient suppression.

## ADR-007 — Key pull-ups + SENS− pull-up on B.Cu between the socket rows

R1..R7 + R18 sit on the bottom, each at its analog pin's x, 2.2 mm south
of the row — the region under the socketed Nano is unused on the bottom.
Traces are 2 mm B.Cu stubs to the pin annuli; the +5 V side feeds a
single bus. The LED series resistors (R8..R17) similarly go on B.Cu
under the LED bar, offset +2.3 mm east of each LED.

## ADR-008 — Routing widths: cloud routes at 0.2, post-pass widens

The stateless `/route` endpoint exports the DSN from a bare board: the
default netclass (0.2 mm track / 0.2 mm clearance) applies — netclasses
live in the project file the API never sees. Enforcing the YAML widths
(0.25 signal / 0.5 power ≥ the 0.4 spec minimum) is therefore a local
post-pass (`tools/post_route.py`): clearance-aware widening from the
freerouting centerlines (one-shot, idempotent), capped so 0.2 mm copper
clearance and 0.3 mm edge clearance are never violated, floor 0.2 mm
(= CONVENTIONS §7 minimum). Result on v0.1.0: 105 segments at full
target, 10 short pad-entry stubs capped (all ≤ 0.35 mm, all ≥ 0.2 mm,
worst-case current ≈ 200 mA — a 0.2 mm/1 oz trace is good for ~0.5 A+).
The zone is refilled afterwards (fills are stored in the file; `/drc`
does not refill).

## ADR-009 — GND strategy: B.Cu zone, solid connect on EVERY GND pad

Zone per LESSONS_LEARNED §1/§3/§16: clearance 0.2, min_thickness 0.2,
thermal 0.5/0.5, no `min_resolved_spokes`. v0.0.2 DRC showed the SMD GND
pads of R8..R17 starving on thermal reliefs (1 spoke < 2) in the packed
LED strip → **all** GND pads get `zone_connect 2` (solid). Slightly
harder hand-soldering on those pads, always-connected fill.

## ADR-010 — Automatic zone-island healing (LESSONS_LEARNED §12)

Freerouting output varies run to run; one v0.1.0 route attempt fenced
off a B.Cu fill region (DRC: zone↔zone unconnected). `post_route.py` now
detects fill islands and lays a 0.3 mm B.Cu GND stitch track across the
narrowest pinch (a track needs only clearance where the fill needs
clearance + min_thickness), refills, and repeats — aborting loudly if it
cannot converge. The DRC gate in `cloud_pipeline.sh` additionally blocks
`/fab` whenever errors or unconnected items are non-zero.

## ADR-011 — Silk policy

References hidden for the dense small parts (LEDs, resistors, bottom
ceramics, holes) — the F.Fab layer keeps them all for assembly docs; the
load-bearing labels are explicit: `KEYS G 7..1`, `5V IN + −`, `SENS+`,
`SENS−`, LED `1`/`10`, title + version. All silk text ≥ 0.8 mm (KiCad
text_height DRC minimum). Iterated until `/drc` reports **zero** silk
warnings, so the final DRC gate is 0 errors / 0 warnings with no
justification list needed.

## ADR-012 — Hole-verification vision pass structurally skipped

`pcb_designer.verify.holes`' vision method calibrates a 6-DOF affine
from detected hole centres and needs ≥3 holes; this board has exactly 2
by spec. The geometric check (position, Ø, spacing pattern) runs and
passes; the vision pass is reported as SKIP with the reason, and the
renders are inspected visually in the pipeline instead (gate 7e).

## ADR-013 — Toolkit bugfix: `verify.holes._pad_subblock` main-pad selection

KiCad 9's `MountingHole_*_Pad_Via` footprints serialise the eight Ø0.5
stitching vias BEFORE the Ø2.5 anchor pad; the parser took "the first
pad" and reported drill Ø0.5/pad Ø0.8 for every such hole (false FAIL
against any ground truth). Fixed in `src/pcb_designer/verify/holes.py`
to select the largest-drill pad. `ruff` + full `pytest` (53) stay green.
Own commit per the toolkit-change policy.

## ADR-014 — Deterministic serialisation for idempotency (LESSONS_LEARNED §7)

pcbnew and kicad-sch-api serialise objects in random-UUID order. The
builders therefore (a) canonically reorder top-level and footprint-child
blocks (pads keep library order — the verify parser and human diffs
expect it), and (b) rewrite every UUID with a deterministic uuid5
sequence, mapping old→new so sheet-instance `(path "/…")` references
stay consistent. Each object keeps a unique UUID; none are reused.
Verified: 3 consecutive builds → identical SHA256 for both files.

## ADR-015 — v0.1.0 ERRATUM: Nano socket rows were 180°-rotated (fixed v0.2.0)

v0.1.0's pin map assumed TX1/VIN at the mini-USB end. The REAL Arduino
Nano — verified on the official 2008 V2.2 board photo (Wikimedia
`File:ArduinoNanoTop.jpg`) and independently on a clone
(`File:Arduino Nano.jpg`) — has **D12/D13 flanking the USB and TX1/VIN
at the ICSP end**. Consequence of the erratum: v0.1.0 was electrically
correct ONLY with the Nano inserted USB-east, where the buzzer physically
blocks the plug — i.e. unusable for flashing. Discovered while preparing
the photo-overlay render: exactly the failure class the overlay gate
exists for (MT1 POST-MORTEM-001 all over again).

v0.2.0 fix, keeping the mechanical design (USB corridor WEST):
- socket net maps swapped to the real orientation (U1 south row =
  D13,3V3,AREF,A0..A7,5V,RST,GND,VIN west→east; U2 north row pin1 east =
  TX1,RX0,RST,GND,D2..D12);
- analogs now sit on the SOUTH row → keys header moved to the SOUTH edge
  (J2.1..7 = KEY1..7 straight drops under A0..A6, J2.8 = GND clip);
- digitals on the NORTH row → LED bar moved to the NORTH edge. The
  D2..D11 pins DESCEND west→east, so a non-crossing fan needs LED1 at
  the EAST end: the bar ascends east→west (labels `10` west, `1` east);
- pull-ups follow their pins (B.Cu, y=119.4, KEY pad south);
- chirality triad and all verify tables re-derived; all gates re-run
  green (DRC 0/0/0, ERC 0/0, verify_placement/holes/geometry PASS).

## ADR-016 — Overlay rotation follows the engine's MT1-calibrated convention

`render_overlay` composes the pasted photo's rotation as
`PIL_rot = −pcb_rotation + image_rotation` (module_overlay.py). MT1's
anchors are 180°/bottom-side so the sign was never exercised at 90° —
our first overlay attempt (image_rotation=180, derived from the
pcbnew-verified +90 convention) landed the Nano photo USB-EAST. Caught
immediately by the overlay itself; fixed by calibrating the data to the
engine (image_rotation_deg: 0 for the native USB-at-bottom crop), the
same per-module calibration approach MT1 used. Engine left untouched:
changing the sign would silently break MT1's tuned overlay data.

## ADR-017 — Render suite via the upgraded cloud service (v0.3.0)

The board now ships five render styles per side, all produced by the
hosted API (`/render?style=`): `bare` (copper/silk raytrace), `dim`
(MT1-style 2D DIM plots), `realistic` and `realistic-dim` (raytrace with
the kicad-packages3d component bodies now installed in the service
image), and `overlay` (transparent-bg raytrace + the real Nano photo
composited server-side from client-uploaded assets;
calibration=green_bbox because the board has 2 anchor holes < the 4 the
hole-affine needs). Nano photo: Wikimedia Commons `File:Arduino
Nano.jpg`, CC BY 2.0, tight-cropped; provenance + measured scale
documented in overlays/modules.yaml.

## ADR-018 — post_route robustness upgrades (v0.2.0 pipeline work)

Four self-healing behaviours added while stabilising v0.2.0, all
deterministic and clearance-checked with exact geometry:
1. `pad.GetLayer()` returns F_Cu even for pads on flipped B.Cu
   footprints — every layer test now uses `IsOnLayer()` (a 0.1777 mm
   clearance violation slipped through the widener because of this);
2. zone-island healing is now judged by CONNECTIVITY (ratsnest count),
   not fill-outline count, and gained two bridge mechanisms beyond the
   B.Cu pinch stitch: an F.Cu track from a THT GND pad inside the island,
   or a new GND via + F.Cu track (straight/L) to the nearest main-fill
   GND pad/via;
3. split non-GND nets are repaired generically (union-find over the
   net's copper; bridge the closest pad pair across fragments) —
   freerouting reproducibly dropped two 2.54 mm links of the +5V
   pull-up daisy-chain;
4. tiny-segment cleanup only removes segments fully inside a same-net
   via barrel — the blanket ≤0.1 mm text pass once deleted a
   load-bearing 0.05 mm jog and split /+5V.

## ADR-019 — v0.2.1: silk/label pass + render suite naming (user request)

1. Version silk prefixed with the word `pcb` (`pcb v0.2.1`), front and
   back.
2. MT1-style per-pin legends (h0.8/w0.65, rotated) on both socket rows;
   U1/U2 refs moved west of their rows; version text to the corridor
   centre; B-side title moved up to clear the new pull-up refs.
3. Every component shows its reference on its own layer. Sole exception:
   the LED refs (D3..D12) print on B.SilkS as board text — at 4.6 mm
   pitch the front has a 0.14 mm courtyard gap, physically no room.
4. Render suite archived per version as
   `renders/<ver>-{normal,dim,realistic,overlay}-{top,bottom}.png`;
   `realistic-dim` dropped from the board's suite (the API keeps the
   style); `overlays/` holds only the client-side assets
   (component-images/ + modules.yaml).

## ADR-020 — v0.3.0: two anchor holes per short edge (user spec)

H1..H4 at x=95/185, y=105/125 — the same 20 mm vertical pair pattern as
MT1's anchors, mirror-symmetric about x=140 AND y=115. With ≥3 holes the
toolkit's hole-VISION pass becomes calibratable, so verify_holes now
attempts it against the version's normal renders (non-fatal when the
detector cannot lock; a completed pass with real deviation fails the
gate). Geometry gate rewritten for the 4-hole pattern.

## ADR-021 — VU-meter LED colors with per-LED 3D bodies

LED1..10 = 3×GREEN, 3×YELLOW, 2×ORANGE, 2×RED (progress bar reads
green→red as it fills). Values drive the BOM; each footprint's 3D model
points at the matching kicad-packages3d body (`LED_D3.0mm_Green/_Yellow`,
base red) — orange doesn't exist upstream, so the service image ships
`LED_D3.0mm_Orange.step` (stock red model, body COLOUR_RGB patched;
eda-pcb-designer assets/3dmodels/).

## ADR-022 — Realistic-bottom bug: /place stripped models from native flips

The cloud /place stripped `(model ...)` from EVERY B.Cu-target footprint
(an MT1-era guard against the tag-swap-flip render artifact,
LESSONS_LEARNED §5) — so the realistic bottom render lost all 20 back
parts and looked identical to `normal`. Fixed in the toolkit: strip only
blocks lacking `(justify mirror)` (native flips keep their correctly-
rendered models). Regression tests in eda-pcb-designer.

## ADR-023 — Brighter Nano overlay photo

`File:Arduino nano.jpg` (Wikimedia Commons, CC BY-SA 4.0) replaces the
dark clone shot: bright perpendicular studio photo of an Arduino-branded
Nano, natively in the board pose (USB west, D12..TX1 / D13..VIN rows).
Measured 57.95 px/mm (row spacing and pin span agree to <0.2%);
image_rotation_deg=90 under the engine's −pcb_rot+img_rot convention
(net rotation 0). Provenance + numbers in overlays/modules.yaml.

## ADR-024 — v0.4.0: 120 × 40 mm frame and a full floor-plan remake

The user asked for a complete remake at 120 × 40 mm with a specific
distribution: Nano in the middle, keys header centred, the whole 5 V-in
filter on the left, buttons on the right centred on the horizontal
mid-line, a parallel 2-pin header per button, buzzer wherever it fits.
Frame: x 90..210, y 100..140 (MT1 coordinate style), centre (150, 120).
Anchor dividers move to x = 100 / 200; the four M2 holes go to the corners
at 5 mm insets (95/205 × 105/135), still mirror-symmetric about both axes.

Consequences of centring the Nano that drove the rest of the plan:

1. The mini-USB corridor is no longer a 14 mm sliver at the west edge — it
   is the whole 42 mm strip west of the socket field. Keeping all of it
   clear would waste a third of the board, so the corridor is redefined as
   the **band a cable actually needs**: x < 130.4 (socket courtyard west
   edge), y 113..127 (±7 mm about the USB centreline). That band is
   completely part-free; `geometry_gate` checks it against real courtyard
   boxes rather than origin coordinates.
2. The filter therefore **folds around** the corridor instead of sitting in
   one strip: VIN section on the south strip (J1 → D1 → D2, all short
   parts), VRAW/+5V section on the north strip (C1 → L1), and the +5 V
   reservoir C3 at the south-strip east end. Three Ø8 mm radial parts do
   not fit in one 27 mm strip (26.76 mm of courtyard in 26.84 mm of space
   — zero clearance), which is what forced the split.
3. **D2 is placed at rot = 180** so pad 2 (anode, VIN) lands WEST of pad 1
   (cathode, VRAW) and the chain reads J1 → D1 → D2 west→east with no
   doubling back. First use of rot=180 on this board; `EXPECTED_PADS` locks
   the convention (pad 2 at x − 5.08).
4. The LED bar is centred over the **D2..D11 pin span** (midpoint x =
   146.2) rather than over the board, so the fan legs stay short at both
   ends while keeping the non-crossing east→west order from ADR-015.
5. `+5V` now runs ~40 mm from the choke to the Nano 5 V pin because the
   user requires the whole filter on the left. At 0.5 mm/1 oz and 200 mA
   worst case that is ≈ 50 mΩ / 10 mV — irrelevant next to the filter's
   own drop. Accepted deliberately over splitting the filter.

## ADR-025 — 5 V entry on a 2-pin 2.54 mm header (supersedes ADR-003)

The user asked for the 5 V input "as 2 pines". J1 becomes
`PinHeader_1x02_P2.54mm_Vertical`; pad 1 = +, pad 2 = GND, unchanged nets.
This reverses ADR-003's screw-terminal choice: the header is what the user
wants, it costs 33 mm² instead of 61 mm² (the Phoenix block's courtyard is
11.2 × 10.9 mm), and the v5.5 BOM's "chop a USB-A cable" pigtail terminates
in exactly this kind of DuPont pair. Trade-off accepted: a header has no
strain relief, so the input wire wants a zip-tie to the board if the piano
gets carried around.

## ADR-026 — A parallel 2-pin header per SENS button

User spec: "put for each button a parallel 2 pins for connect a external
buton too". J3 sits on `/SENS_PLUS` + `/GND` (parallel to SW1) and J4 on
`/SENS_MINUS` + `/GND` (parallel to SW2). Both are wired to the same two
nodes as their button, so shorting a header is electrically identical to
pressing the on-board tact switch — no jumper to cut, no mode select. Each
header sits directly EAST of its button (1.16 mm courtyard gap) so the
pairing is obvious without a wiring diagram; silk reads `SENS+`/`EXT+` and
`SENS-`/`EXT-`. Both buttons keep their internal-pull-up / R18 arrangement,
so an external button needs no extra parts.

## ADR-027 — Buzzer in the NE corner; long /BUZZER trace accepted

"Place the buzzer as you can". The Buzzer_12x9.5RM7.6 courtyard is
12.56 × 12.56 mm and the east block has 28 mm of width for the buzzer, the
button column (9.56) and the external headers (6.14) — the three cannot
share x-lanes, so the buzzer takes its own y-lane in the north-east corner
(y 100.5..113.06) with the buttons and headers below it. Cost: D13 is at
the WEST end of the south socket row, so `/BUZZER` runs ~58 mm across the
board. It carries a ~4 kHz square wave into a piezo — no current, no
timing sensitivity, and it runs on F.Cu over the solid B.Cu GND plane, so
the loop area stays small. The alternative (buzzer west, near D13) would
have to displace the filter the user asked to keep on the left.

## ADR-028 — Silk: title block moves into the USB corridor

With the Nano centred there is no free strip along the south edge for the
title any more (the keys header owns the middle of it). The part-free USB
corridor is the largest open front-side area on the board, so
`LEMON PIANO V5.5` + `pcb vX.Y.Z` go there (F.SilkS at y 118 / 121.5,
B.SilkS at 123.5 / 125.5) — visible, and it stops the corridor reading as
an unfinished gap. The `KEYS` word moved from west of the header to EAST of
it, because the C3 reservoir now occupies the south-west.

## ADR-029 — v0.5.0: the Nano is FLIPPED, mini-USB faces EAST (supersedes ADR-002)

User instruction: *"flip the arduino and put the leds down center vertically,
put key keyboard up"*. These are one change, not three. The Nano's pinout is
fixed (ADR-015): the analog column (D13, 3V3, AREF, A0..A7, 5V, RST, GND,
VIN) and the digital column (D12..D2, GND, RST, RX0, TX1) are on opposite
long edges of the module. Which board edge each column faces is therefore
decided entirely by the module's orientation, and rotating it 180° in-plane
is the ONLY way to swap them — which necessarily moves the mini-USB from
west to east. So "keys up + LEDs down" *requires* the flip, and the user's
next instruction (ADR-030) confirms they understood that consequence.

What the flip changes:
- `U1` (analog) becomes the **NORTH** row, pin 1 (D13) at the **EAST**
  (rot=270, origin x=167.78). `U2` (digital) becomes the **SOUTH** row,
  pin 1 (TX1) at the **WEST** (rot=90, origin x=132.22). The two YAML
  placement entries essentially trade places.
- A0..A6 now **descend** west→east (A0 at 160.16 … A6 at 144.92), so the
  keys header needs **pin 1 (KEY1) at the EAST** to keep straight drops.
  Header still centred: pins 141.11..158.89, centre exactly x=150, so silk
  reads `KEYS G 7 6 5 4 3 2 1` west→east.
- D2..D11 now **ascend** west→east, so the LED bar ascends **west→east**
  (LED1 west, LED10 east) — the mirror of ADR-015's east→west order. The
  bar is centred on x=150 per the user's "center" instruction, rather than
  over its pin span as in v0.4.0.
- Both geometric-0805 groups **flip their pad numbers**. The pull-ups now
  sit south of a NORTH analog row (pin node = north pad = pad 2, rail =
  pad 1); the LED resistors sit north of a SOUTH bar (cathode = south pad =
  pad 1, GND = pad 2). Exactly the trap AGENT_PROMPT warns about: the
  builder assigns these by measured position, so `ground-truth/components.yaml`
  was re-derived from the built board and both groups' pad numbers inverted.
  New builder assertions lock the KEY pad at y=114.6 and the cathode pad at
  y=138.03 so a future move cannot silently swap them.
- Bonus: D13 and D12 move to the EAST end, next to the buzzer and the SENS+
  button. `/BUZZER` drops from ~58 mm (ADR-027) to ~20 mm and `/SENS_PLUS`
  from ~43 mm to ~8 mm.

## ADR-030 — The USB-cable keepout is dropped (supersedes the ADR-024 corridor)

User instruction: *"dont care about usb connector space"*. The corridor
keepout introduced in ADR-024 is therefore removed from the design and from
`geometry_gate`. This is what makes the ADR-031 filter refactor possible —
the corridor was consuming a 30 × 14 mm hole in the middle of the west
block, which is why v0.4.0 had to fold the filter around it.

**Consequence the user should know about**: with the USB now facing east
(ADR-029) and no clearance reserved, the mini-USB shell reaches about
x=173.3 while SW1's courtyard starts at x=173.48 — they nearly touch, and a
plugged USB cable will foul the SENS buttons. **Flashing the Nano means
lifting it out of its socket** (or flashing before it is seated). That is an
accepted trade, not an oversight: it is the direct cost of keys-north +
LEDs-south, and the socket exists precisely so the module can come out.

## ADR-031 — Power-filter section refactored into a compact 3-row block

User instruction: *"put c1 and c3 together and refactorize this section"*.
With the corridor gone (ADR-030) the whole west block (x 100.5..127,
y 100..140) is usable, so the filter is rebuilt as three tidy rows:

| Row | y | Parts | Node |
|---|---|---|---|
| north | 106.0 | **C1 ‖ C3**, 1.24 mm apart | VRAW bulk ‖ +5 V bulk |
| middle | 119.0 | D2 → L1, pads 5.5 mm apart | VIN→VRAW→+5 V |
| south | 134.5 | J1 → D1 | 5 V entry + TVS |
| B.Cu | 112.5 | C2, C4 under the gap between rows | HF bypass at each node |

The two bulk caps are adjacent on one row as asked, and the choke sits
directly beneath the pair so C1's `+` is 13 mm from L1 pad 1 and C3's `+` is
13 mm from L1 pad 2 — both far shorter than v0.4.0, where C3 lived 30 mm
away on the opposite side of the board.

Engineering note on the literal reading: a pi filter is normally laid out
C1—L1—C3 *in series order* so the unfiltered and filtered rails never run
side by side. Putting C1 and C3 adjacent does place VRAW and +5 V ~10 mm
apart. Accepted because the choke physically separates the two nodes'
current paths, the B.Cu ground plane provides the return directly under
both, and this filter's job is conducted mains transients (µs–ms), not
MHz isolation where that coupling would matter. `geometry_gate` now asserts
the two caps stay on one row within 3 mm, so the intent cannot be lost.

## ADR-032 — post_route bugfix: refill the GND zone after split-net repair

The first v0.5.0 run failed the DRC gate with two `clearance 0.2 mm; actual
0.0000 mm` errors, both `Track [/+5V] on B.Cu, length 2.5400 mm` against
`Zone [/GND]`. Cause, straight from post_route's own log ordering:

```
refilled 1 zone(s)                                    <- fill computed here
repaired split net /+5V: (147.46,116.60)->(150.00,116.60)   <- copper added
repaired split net /+5V: (155.08,116.60)->(157.62,116.60)      AFTER the fill
```

`repair_split_nets` fixes the two 2.54 mm pull-up-bus links that freerouting
reproducibly drops (ADR-018 item 3), but it ran *after* `filler.Fill()`, so
the ground fill had never made room for those bridges and they sat directly
on filled copper. `_bridge_fragments`' clearance check looks at tracks, vias
and pads — not at the zone's filled polygons — so it saw nothing wrong.

Fix: `repair_split_nets` now returns the number of bridges it laid, and the
caller refills the zones when that count is non-zero (LESSONS_LEARNED §11 —
the filler must run after *any* copper is added, not just after the SES
import and the stitches). Re-run: DRC 0/0/0.

This was **latent since v0.2.0**, when the repair pass was introduced.
v0.4.0 escaped it only by luck: the same two bus links were dropped there
too, but the bridges happened to land where the fill had already retreated.
Moving the pull-ups to y=115.6 put the bus in the middle of a filled region
and exposed it. Worth remembering as a class of bug: a stage that adds
copper after the last fill is a silent DRC failure waiting for a placement
change.
