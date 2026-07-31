# Changelog

Append-only log of significant changes. Newest first.

## 2026-07-31 (pcb/) — rotatable 3D models in the pipeline

- The pipeline now emits a **rotatable 3D model** of the board on every run,
  via the new `POST /export3d` endpoint in eda-pcb-designer 0.4.0:
  `pcb/3d/lemon-piano-<ver>.glb` (browsers / `f3d`) and `.step` (CAD). The
  cloud does it because the service image ships the `kicad-packages3d` body
  library that the slim local Docker image deliberately omits — a local export
  would silently drop every component body.
- `pcb/3d/*.glb` and `*.step` are **committed** (2026-07-31), so cloning the
  repo is enough to look at the board — no KiCad, no Docker, no pipeline run.
  Cost, stated plainly: ~16 MB of binaries per version, permanent in git
  history. Only `pcb/3d/models/` stays untracked (fetched third-party KiCad
  body files, re-downloadable in seconds). They do not preview on github.com:
  GitHub renders only `.stl` inline and only up to 10 MB, and this board's STL
  is 34 MB — measured, not assumed.
- `pcb/tools/export_3d.sh` does the same offline, fetching just the ~15 bodies
  this board needs (~1.4 MB, cached). Its only fidelity gap: the orange LED
  body does not exist upstream, so D9/D10 come out red locally and orange from
  the cloud.
- README gains a "Look at the board in 3D" section: <https://3dviewer.net>
  (drag-and-drop, renders in-browser, nothing installed), `sudo apt install
  f3d`, and the KiCad-9 PPA route — noting that Ubuntu's own repos carry only
  KiCad 7, which cannot open this v9 board.
- Not a release gate: if `/export3d` fails the pipeline warns and carries on,
  because a viewing aid must never block electrically-verified outputs.
- **`.vscode/extensions.json`** recommends `thingraph.cad-viewer`, so VS Code
  offers to install it on first open and the `.glb`/`.step` open in a rotatable
  tab on double-click. Chosen over the 100×-more-popular `cesium.gltf-vscode`
  ("glTF Tools") because that one registers no viewer for `.glb` — it is a
  format validator/converter, so you would have to import to `.gltf` first.
  Verified from the extension manifests, not the marketplace descriptions
  (CAD Viewer's page contradicts itself on STEP; its bundled
  `occt-import-js.wasm` settles it).

## 2026-07-31 (pcb/) — v0.6.0: J5, aux speaker output in parallel with the buzzer

- **New 2-pin header J5 (`SPK`)** on the same two nodes as the buzzer — pad 1
  on `/BUZZER` (the D13 drive line), pad 2 on `/GND` — placed immediately west
  of BUZ1 (5.18 mm courtyard gap). Same pattern as the SENS external-button
  headers: nothing is switched or cut, the on-board buzzer keeps working and
  whatever is plugged into J5 sees exactly the same signal.
- **Load limit, documented on the board's README and in ADR-034.** D13 drives
  a *passive piezo* directly ("no series resistor needed",
  `versions/v0-buzzer/HARDWARE.md`), which is a high-impedance load. A bare
  4–8 Ω voice coil at 5 V would ask ~600 mA against the ATmega328P's 40 mA
  absolute maximum per pin and would damage the pin. J5 therefore suits
  another piezo or an **amplified** speaker module; a raw speaker needs an
  amplifier. No driver stage was added to the PCB — the `versions/` tree
  defines a directly-driven passive buzzer, and inventing parts the sources
  do not specify is against the project's rules.
- Fixed a pre-existing documentation error spotted while editing NETLIST.md:
  the U1/U2 rows in the component table had their descriptions swapped (U1 is
  the *analog* column, U2 the *digital* one — the pad→net tables and the
  ground truth were always right).
- `geometry_gate` gains a check that J5 stays adjacent to the buzzer it
  parallels. verify_placement now covers 74 pad↔net↔function pairs.

## 2026-07-30 (pcb/) — v0.5.1: D1's missing 3D body (Kathode/Cathode filename split)

Cosmetic-only fix, user-reported ("¿por qué D1 no está renderizado?").

- **D1 had no 3D body in any realistic render** — bare pads and silk only,
  while D2 beside it rendered fine. Cause: KiCad 9's
  `D_DO-15_P5.08mm_Vertical_KathodeUp` footprint references a STEP of the
  same name, but kicad-packages3D ships that body spelled the English way,
  `..._Vertical_CathodeUp.step`. The footprint kept the legacy German-ish
  "**K**athode"; the 3D library moved to "**C**athode". `kicad-cli pcb
  render` skips models it cannot resolve **silently** — no warning, no DRC
  item, no build error — so it went unnoticed from v0.3.0 (first release with
  3D bodies) through v0.5.0. D2 is fine because its footprint and model names
  agree (`..._AnodeUp`).
- Fixed by pointing D1 at the file that exists, reusing the per-footprint
  model override already there for the coloured LEDs: `LED_MODELS` is folded
  into a general `MODEL_OVERRIDES` table, and the override loop now raises if
  a target footprint has no `(model ...)` block, so a future library change
  fails loudly instead of silently reverting to no body.
- **No effect on fabrication.** 3D models play no part in gerbers, drill,
  BOM, position files, netlist or DRC, so v0.5.0's fab package was never
  wrong — only its renders were incomplete. PATCH bump per the version rule.
- Audited all 44 model paths the board references against upstream
  kicad-packages3D while in there. Three more are absent upstream but resolve
  on the render host and render correctly, so no action:  L1's Fastron choke
  and the two `LED_D3.0mm_Orange.step` bodies (service-shipped, ADR-021).
  D1 was the only broken reference. ADR-033.

## 2026-07-30 (pcb/) — v0.5.0: Nano flipped (USB east), keys north, LEDs south, filter refactored

Second pass on the 120 × 40 mm board, to the user's follow-up spec. Circuit
unchanged again — this is all orientation and placement.

- **The Nano is flipped 180°: mini-USB now faces EAST** (ADR-029). This is
  the load-bearing change, and it is what "keys up, LEDs down" *requires*:
  the module's analog and digital columns sit on opposite long edges, so the
  only way to swap which board edge they face is to rotate the module. `U1`
  (analog) becomes the NORTH row with pin 1 (D13) at the east; `U2`
  (digital) becomes the SOUTH row with pin 1 (TX1) at the west.
- **Keys header on the NORTH edge**, still centred (pin centre exactly
  x=150), with pin 1 = KEY1 at the EAST because A0..A6 now descend
  west→east. Silk reads `KEYS G 7 6 5 4 3 2 1`, and the GND clip moved to
  the west end of the header.
- **LED bar on the SOUTH edge, centred on x=150**, ascending west→east
  (LED1 west under D2, LED10 east under D11) — the mirror of the old order.
- **Both 0805 resistor groups inverted their pad numbers.** The builder nets
  them geometrically, so the pull-ups (now south of a north analog row) and
  the LED resistors (now north of a south bar) both swapped which pad
  carries which net. `ground-truth/components.yaml` was re-derived from the
  built board, and two new builder assertions pin the KEY pad at y=114.6 and
  the LED cathode pad at y=138.03 so this cannot drift silently.
- **USB-cable keepout dropped** (ADR-030) at the user's explicit request.
  Trade-off, stated plainly: the USB shell now reaches x≈173.3 with SW1's
  courtyard starting at 173.48, so **flashing means lifting the Nano out of
  its socket**. That is the direct cost of keys-north + LEDs-south.
- **Filter section refactored** into a compact 3-row west block (ADR-031)
  now that the corridor no longer punches a hole through it: **C1 ‖ C3
  adjacent** on the north row as asked, D2 → L1 on the middle row with their
  pads 5.5 mm apart, J1 → D1 on the south row. Both bulk caps are now ~13 mm
  from the choke instead of C3's old 30 mm trip across the board.
- The flip also shortened two long v0.4.0 runs for free: `/BUZZER` from
  ~58 mm to ~20 mm and `/SENS_PLUS` from ~43 mm to ~8 mm, because D13 and
  D12 moved to the east end next to the buzzer and the SENS+ button.
- **post_route bugfix** (ADR-032): `repair_split_nets` laid its bridge tracks
  *after* the GND zone was filled, so the fill never made room for them —
  the first v0.5.0 run reported two `clearance 0.2 mm; actual 0.0000 mm`
  errors on the +5 V pull-up bus. It now returns a bridge count and the
  caller refills when it is non-zero. This was latent since v0.2.0; v0.4.0
  only escaped it because its bridges happened to land off the fill.
- Title block moved to the SE quadrant (the north edge belongs to the keys
  header now). `geometry_gate` grew to 24 checks: flip orientation, keys
  north + KEY1-east, LED bar south + centred + west→east order, and C1/C3
  adjacency.

## 2026-07-30 (pcb/) — v0.4.0: 120 × 40 mm remake, centred Nano, external-button headers

Complete floor-plan remake on a bigger board, to the user's distribution
spec. Same V5.5 circuit — no value, pin or part invented; only the board
interface grew.

- **120 × 40 mm frame** (was 100 × 30), KiCad frame x 90–210 / y 100–140.
  Anchor dividers at x=100/200; the four M2 holes move to the corners at
  5 mm insets (95/205 × 105/135), still mirror-symmetric about both axes.
- **Nano socket centred on the board** (pin field x 132.22–167.78, rows
  y 112.38/127.62), mini-USB still facing WEST. Centring it turned the USB
  corridor from a 14 mm sliver into a 42 mm strip, so the corridor is now
  defined as the band a cable actually needs — x < 130.4, y 113–127 — kept
  completely part-free and checked against real courtyard boxes.
- **Whole 5 V filter in the west block, folded around that corridor**:
  VIN section on the south strip (J1 → D1 → D2), VRAW/+5 V on the north
  strip (C1 → L1), output reservoir C3 at the south-strip east end. Three
  Ø8 mm radial parts do not fit in one 27 mm strip, which forced the split.
  D2 is placed at rot=180 so the chain reads west→east with no doubling
  back (first use of that rotation here; the pad assertions lock it).
- **5 V entry is now a 2-pin 2.54 mm header** (J1), replacing the Phoenix
  screw terminal — the user asked for "2 pines". Same nets, same polarity.
- **A parallel 2-pin header per SENS button** (J3 ∥ SW1 on /SENS_PLUS,
  J4 ∥ SW2 on /SENS_MINUS), so an external panel button can be plugged in
  without cutting a trace. Each sits directly east of its button.
- **Lemon-key header centred** on the south edge (pin centre exactly
  x=150); **SENS button pair centred** on the y=120 mid-line; **LED bar**
  centred over the D2–D11 pin span so the fan legs stay short at both ends.
- **Buzzer in the NE corner** — the only place its 12.56 mm courtyard fits
  once the button column and external headers claim their x-lanes.
- Title block moved into the USB corridor (the south edge belongs to the
  keys header now). `geometry_gate` rewritten: 23 checks including the new
  centring and corridor rules.
- **Green on the first pipeline run**: DRC 0 errors / 0 warnings /
  0 unconnected, ERC 0/0, verify_placement + verify_holes (vision ran,
  max LOO 0.0222 mm) + geometry_gate ALL PASS. Fab zip:
  `pcb/releases/v0.4.0/`. ADR-024..028.

## 2026-07-30 (pcb/) — v0.3.0: 4 anchor holes, VU-meter LEDs, render fixes

- **Four M2 anchor holes** (two per short edge at x=95/185, y=105/125,
  MT1's 20 mm pair pattern) — and with ≥3 holes the toolkit's hole-VISION
  verification now runs (affine over detected centres on
  transparent-background renders per LESSONS_LEARNED §22; LOO ≤0.04 mm
  both sides).
- **LED bar colors as the real build**: LED1..10 = 3 green, 3 yellow,
  2 orange, 2 red (fills green→red). Values drive the BOM and each LED's
  3D body is colored in the realistic renders — orange doesn't exist in
  kicad-packages3d, so the pcb-designer service now ships a derived
  `LED_D3.0mm_Orange.step`.
- **Realistic-bottom fixed**: the cloud /place endpoint stripped 3D
  models from every back-side footprint (an MT1-era guard against
  tag-swap flips); natively flipped parts keep their models now — the
  bottom render finally shows all 20 resistors/capacitors.
- **Brighter Nano overlay photo**: Wikimedia `File:Arduino nano.jpg`
  (CC BY-SA 4.0), studio-lit and natively in the board pose; measured at
  57.95 px/mm. Fab zip: `pcb/releases/v0.3.0/`. ADR-020..023.

## 2026-07-30 (pcb/) — v0.2.1: silk label pass + render naming

- Version silk now reads `pcb v0.2.1` (front corridor + back), MT1-style
  **pin legends on both socket rows** (D13 3V3 AREF A0..A7 5V RST GND VIN /
  D12..D2 GND RST RX0 TX1) and **every component labelled** on its own
  layer (bottom passives on B.SilkS; LED refs on the back — 4.6 mm pitch
  leaves no front room). DRC still 0/0/0, all gates green, fab zip at
  `pcb/releases/v0.2.1/`.
- Render suite renamed and trimmed: `renders/<ver>-{normal,dim,realistic,
  overlay}-{top,bottom}.png` (realistic-dim dropped); the overlay
  composite now lives in `renders/`, while `overlays/` keeps only the
  module photos + `modules.yaml`. v0.2.0 artefacts renamed to match.

## 2026-07-30 (pcb/) — v0.2.0: Nano-orientation erratum fixed + full render suite

The photo-overlay work caught a real v0.1.0 bug (exactly what that
verification exists for): the socket net maps assumed TX1/VIN at the
mini-USB end, but the REAL Nano has **D12/D13 at the USB end** (verified
on the official 2008 V2.2 board photo + clones). v0.1.0 would only work
inserted USB-east — where the buzzer blocks the plug.

- **v0.2.0** keeps the mechanics (USB corridor west) and fixes the maps:
  keys header now on the SOUTH edge (`1..7 G` under A0–A6), LED bar on
  the NORTH edge ascending east→west, pull-ups moved with their pins.
  All gates green again: DRC 0/0/0, ERC 0/0, anti-mirror/holes/geometry
  PASS. Fab zip: `pcb/releases/v0.2.0/`. The v0.1.0 release folder is
  renamed `v0.1.0-ERRATUM-do-not-fab`. Full story: `pcb/docs/DECISIONS.md`
  ADR-015..018.
- **Render suite** via the upgraded pcb-designer service (v0.3.0, now
  with the KiCad 3D model packages): per side `bare`, `dim` (MT1-style
  2D plots), `realistic`, `realistic-dim`, and a **photo overlay** with a
  real Arduino Nano image (Wikimedia Commons, CC BY 2.0) composited
  server-side — `pcb/renders/` + `pcb/overlays/`.
- Pipeline self-healing upgrades in `pcb/tools/post_route.py` (zone
  island bridging by connectivity, split-net repair, exact clearance
  geometry, `IsOnLayer` fix) — ADR-018.

## 2026-07-30 (pcb/) — Fabricable V5.5 PCB, release v0.1.0

New top-level `pcb/` folder: a 2-layer 100 × 30 mm KiCad-9 board that
replaces the V5.5 breadboard — same circuit, same firmware pinout, nothing
invented (netlist extracted from V5/V5.5 HARDWARE.md + the DRC-validated
`build_v5_5()` wirewright contract; ground truth in `pcb/docs/NETLIST.md`).

- Socketed Nano (2×15 rows, mini-USB at the west edge, USB 5 V isolated
  behind the 1N5817), labelled lemon-key header (`G 7…1`) on the north
  edge, ten-LED bar + SENS± buttons south, D13 buzzer, and the V5.5
  power-entry filter feeding a `5V IN` screw terminal.
- Designed end-to-end with the **pcb-designer** toolkit (sibling repo
  `../eda-pcb-designer`, same consumption pattern as wirewright): cloud
  API for /place /route /drc /render /fab, Docker image for the
  generative/idempotent builders. Release gates all green: DRC 0 errors /
  0 warnings / 0 unconnected, ERC 0/0, anti-mirror + hole + geometry
  verifiers PASS. Iteration history v0.0.1→v0.1.0 archived under
  `pcb/validation/` + `pcb/renders/`.
- Fab package (gerbers, drill, BOM, positions):
  `pcb/releases/v0.1.0/lemon-piano-v0.1.0-fab.zip`.
- The physical filter bench validation (switch-flipping session with the
  V5 sampler) remains pending — unchanged from the V5.5 entry below.

## 2026-07-29 (V5.5) — New version: filtered 5 V supply

New hardware revision `versions/v5.5-power-filter/` — the V5 board behind a
**power-entry filter**, because USB-powering V5 raw let any mains transient
(a light switch, a loaded PC supply) play phantom notes through the
15-20 mV touch margin.

- **Filter chain:** P6KE6.8A TVS across the input → 1N5817 Schottky in series
  (reverse protection + mini-USB backfeed isolation) → 470 µF ‖ 100 nF →
  100 µH power choke → 470 µF ‖ 100 nF → the +5 V rail (fc ≈ 730 Hz,
  2nd order). Rail lands at ≈ 4.7 V; the ADC is ratiometric so calibration
  doesn't care. Full design + bench validation recipe in the version's
  HARDWARE.md.
- **Board and firmware are V5's** (only the serial banner says `V5.5`);
  emulation is V5's, unchanged — supply hardware is invisible to the browser
  AVR (emulation/README.md says so).
- **Diagram:** `build_v5_5()` in tools/wiring_diagrams.py, rendered on
  wirewright — which gained a power-entry component family (`capacitor`,
  `inductor`, `diode` with `flip`, `power_jack`) for it.
- **Verified:** diagram DRC-clean (58 nets, 0 hard violations); firmware builds
  in all three envs (`nanoatmega328`, `nanoatmega328new`, `emulation`,
  PlatformIO in docker, 2026-07-29). Filter itself not yet measured on the
  real board — validation recipe documented.

## 2026-07-29 (V5 LEDs) — Progressive 10-LED fill for calibration and the win
## theme, VU-meter colours in the emulator

Two owner-requested LED/timing changes:

- **Calibration now fills all ten LEDs, not seven.** Only seven keys exist, so
  the old one-LED-per-key loop never lit LEDs 8-10. `lit = (key_index+1) ×
  LED_COUNT / KEY_COUNT` spreads the seven measurement steps proportionally
  across all ten (1,2,4,5,7,8,10 — uneven but always ends at ten), and a new
  `CAL_STEP_PAUSE_MS` (150 ms) pause per key — after its coin, not touching
  `CAL_SAMPLES` so measurement quality is unchanged — makes the count-up read
  as deliberate instead of a blur. Calibration now takes ~1.05 s longer.
- **The win theme now progressively fills the bar instead of flashing it.**
  `playSong()` gained two optional parameters, `ledTotal`/`ledOffset`: when
  `ledTotal > 0` the bar accumulates LEDs (0 to LED_COUNT, staying lit note to
  note) proportional to position in the theme, instead of the old
  whole-bar-flash-per-note. `playVictory()` passes the tail's own note count
  as `ledTotal`, so the fill paces itself to whichever level's theme is
  playing (26-40 notes) with no separate "make it longer" tuning needed — the
  existing tail lengths already spread comfortably across ten LEDs (230-580 ms
  each). The level-start intro keeps the old flash (`ledTotal` defaults to 0,
  unchanged call site) since only calibration and the win were asked for.
- **Emulator LEDs recoloured like a VU meter**: 1-3 green, 4-6 yellow, 7-8
  orange, 9-10 red, across all five spec files that define the ten-LED bar.
  Emulation-only — the real board's LEDs are all green per the BOM.

**Emulation specs re-timed a fourth time**: the +1.05 s calibration pause is a
fixed one-time delay at boot, so it shifts every input/marker in every spec by
the same flat amount — reapplied via a script rather than hand-editing each
timestamp. All four specs green; cross-checked against a real run (`Level 4`
@54834 ms, `ALL LEVELS CLEAR` @79971 ms). All three `pio run` envs build clean
(flash 39.5%/12140 B on hardware, up from 38.8%/11908 B).

## 2026-07-29 (V5 audio, part 4) — Faster autoplayer, Castle closes the game,
## a more recognisable Castle theme

Three more owner-requested changes:

- **Faster Uno autoplayer**: `emulation/autoplayer.yaml`'s `pressKey()` held
  each press 250 ms + 300 ms gap (550 ms/note, tuned for reliability — 90 ms
  measured flaky ~1-in-3, 250 ms measured reliable 9/9). Cut to 160 ms + 200 ms
  (360 ms/note, ~35% faster) — a middle value chosen to sit clear of the
  known-flaky 90 ms while noticeably quicker than 250/300, but **not
  re-verified live at the same 9/9 rigor** (this is an interactive-only,
  emulation-only test aid; a live Playwright re-verification was judged
  disproportionate to the ask). Compile-checked standalone via `arduino-cli
  compile --fqbn arduino:avr:uno` (clean); re-measure and pull back toward
  250/300 if this ever misfires in practice.
- **Level 3 and 4 themes swapped**: Castle now closes the game at level 4
  (makes more sense as the finale) and Starman moved to level 3. Only
  `playVictory()`/`playLevelIntro()`'s switch statements changed — each
  level's own key notes and secret code stayed exactly where they were through
  both this swap and the earlier Underwater→Castle swap.
- **Castle theme redesigned for recognisability**: a player found the first
  version (a plain repeating four-note pulse) too generic. Rebuilt around the
  two traits every description of the real piece agrees on — a fast
  alternating "pedal" hook (Super Mario Wiki's own trivia notes the opening
  echoes The Twilight Zone TV theme's repeated-note guitar riff) answered by a
  chromatic descending run (the "danger" quality every analysis calls out, and
  the only chromatic movement of the four themes, so it can't be confused with
  the others even out of context). Verse repeats, then the same verse a fourth
  higher (victory tail starts here), then a resolving coda. Still tagged 🔨
  reconstruction — no verbatim source exists for this piece, and the Twilight
  Zone connection is documented trivia used as a design anchor, not something
  verified by ear against the original recording.

**Emulation specs re-timed a third time**: Starman (short, ~7.3 s win total)
now at level 3, Castle (redesigned, ~14.4 s win total — still the longest)
now at level 4. Recomputed from the melody tables' `playSong()` math and
cross-checked against a real run's serial timestamps (measured: `Level 4`
@53772 ms, `WIN` @64175 ms, `ALL LEVELS CLEAR` @78913 ms — all inside the
scheduled margins, and the overall run is noticeably shorter than before since
the redesigned Castle theme is more compact than the original). All four
specs green; all three `pio run` envs build clean (flash unchanged at
38.8%/11908 B on hardware — one byte more than before from the swapped
`switch` cases, same tables just reordered).

## 2026-07-29 (V5 audio, fix) — The ending loop actually loops in the emulator now

Owner-reported: in the browser emulation, the game-complete piece played once
and the game silently reset to level 1, instead of looping. That was actually
by design at the time (see "part 3" below) — emulation has no free digital
pins for the sensitivity buttons the reset gesture needs, so `playEndingLoop()`
just played `sfxEnding` once and returned rather than looping something it
could never exit. But that reasoning defeated the entire point of the feature:
an ending that's supposed to keep celebrating instead quietly played once and
reset, which is exactly what it looked like from the outside — a bug, not a
design choice.

Fixed: `playEndingLoop()` now loops forever in emulation too (`while
(playSfx(sfxEnding, true)) {}` — with no `checkAbort` it always completes
normally, so the loop never exits on its own). There is genuinely no way to
test the reset gesture headlessly without sensitivity-button hardware, so
"reset" in the browser means stopping and re-running the simulation — the
same as pulling power on real hardware, and consistent with how every other
emulation-unavailable gesture in this game (smart adjust, sensitivity
buttons) is handled: verified on the real board, not simulated.

`all-levels-win.yaml` re-verified: the fixed-duration simulated run doesn't
care that the firmware never returns from the ending loop (no assertion needs
anything after `ALL LEVELS CLEAR`), and the buzzer-pin edge count roughly
doubled (58777 -> 86560 over the same window) confirming it actually kept
playing instead of falling silent. All three `pio run` envs still build
identically (flash unchanged at 38.7%/11888 B on hardware).

## 2026-07-29 (V5 audio, part 3) — Stuck-key cue, Castle replaces Underwater,
## the ending loops until reset

Three more owner-requested changes:

- **Stuck-key cue**: pressing an already-locked key (same lemon again, before
  a different one unlocks it) used to be completely silent forever — no
  feedback that anything was even listening. Now it stays silent for the
  first `KEY_LOCK_COOLDOWN_MS` (500 ms) after release (`lastReleaseAt` tracks
  this, so a quick accidental double-tap still gets no cue), but a press
  after that plays `sfxKeyStuck` — a new low, rattling triple-hit, deliberately
  distinct from Bump's two-tone end-stop cue since it means a different thing
  ("this key specifically is locked" vs. "the knob can't go further").
- **Level 3: Castle replaces Underwater.** Underwater's subtle opening
  (a chromatic slide) turned out too hard to recognise as a distinct level.
  Castle — SMB1's dark, driving fortress theme, one of its most recognisable
  pieces after Overworld itself — takes its place: `castleNotes[]/
  castleTempo[]` in `main.cpp`, same full-theme treatment as the other three
  (own `CASTLE_VICTORY_FROM`/`CASTLE_INTRO_LEN`). No letter-note tab exists
  for this piece anywhere searched (unlike Overworld/Underworld/Underwater/
  Starman), so it's tagged 🔨 reconstruction, built to match the piece's
  well-documented key (G minor), tempo (90 BPM, 2/2) and driving/syncopated
  character rather than transcribed note-for-note. Level 3's key notes and
  secret code are unchanged — only the win jingle and level-start intro moved.
- **The game-complete piece now loops.** Clearing all four levels used to
  play the (still fairly new) castle-clear jingle once and move on. Extended
  it from ~1.4 s to a three-phrase, ~4.8 s piece with its own cadence
  (`sfxEnding` in `mario_sfx.h`) and added `playEndingLoop()`, which repeats
  it until the player holds both sensitivity buttons for `RECAL_HOLD_MS`
  (1 s — the same gesture/duration as smart adjust, but reaching it from here
  means something different: reset straight to level 1 with **no
  recalibration**, since the player may not be near the fruit while it plays).
  `playSfx()` gained an optional `checkAbort` callback, polled after every
  single note (not just between repeats), so the 1 s hold is honoured almost
  immediately. Emulation has no sensitivity buttons (every digital pin is
  already a LED or a key), so there it just plays the piece once.

**Emulation specs re-timed again**: Castle's victory tail (39 notes, ~16.9 s)
is longer than Underwater's was (33 notes, ~13.6 s), pushing level 4's start
in `all-levels-win.yaml` later by ~3.5 s. Recomputed from the melody tables'
`playSong()` math and cross-checked against a real run's serial timestamps
(measured: `Level 4` @68867 ms, `WIN` @76685 ms, `ALL LEVELS CLEAR` @84293 ms —
all inside the scheduled margins). `lemon-piano.yaml`, `free-play.yaml` and
`hold-and-repeat.yaml` needed no changes: none of their scripted repeats
exceed the new 500 ms stuck-key cooldown, and none of them ever reach level 3
or the ending. All four specs green; all three `pio run` envs build clean
(flash 38.7%/11888 B on hardware, up from 36.9%/11324 B).

## 2026-07-29 (V5 audio, part 2) — Level-start announce, full Underwater/Starman
## themes, a real castle-clear ending

Three more owner-requested changes to V5's sound design, on top of the same
day's mistake-cue/win-order fix below:

- **Level-start announce**: new `playLevelIntro()` plays the first few notes of
  the level's own theme — `MARIO_INTRO_LEN`/`UNDER_INTRO_LEN`/
  `UNDERWATER_INTRO_LEN`/`STARMAN_INTRO_LEN` notes from index 0 of the SAME
  table the win jingle already plays from, no extra flash needed. Runs once at
  boot (level 1) and once every level transition, so the player always hears
  which of the four they landed on before touching a lemon. Blocking, like
  every other melody here, so a key touched during it is lost, not queued —
  documented in the emulation specs that had to be re-timed for it.
- **Underwater and Starman are now full themes**, same treatment as Overworld/
  Underworld: `underwaterNotes[]/underwaterTempo[]` and
  `starmanNotes[]/starmanTempo[]` moved into `main.cpp` as PROGMEM tables
  played via `playSong()`, each with its own `*_VICTORY_FROM` cut-in — replacing
  the old short `playSfx()` excerpts in `mario_sfx.h` (now removed). Underwater
  grew from a 20-note opening to a 55-note piece (opening phrase-pair sourced
  from the cited tab, a second phrase-pair a step down and a closing bridge
  are this project's own extension of that tab's repeating structure — see
  `docs/MARIO-SOUNDS.md` for the honest provenance split). Starman grew from
  26 to 48 notes — the real theme is a short vamp with nothing further to
  transcribe, so "full" means the validated figure played through twice before
  the closing phrase, matching what the NES itself does (loop the vamp).
  Visual side effect: levels 3/4's win light show changed from `playSfx`'s
  step-one-LED-per-note to `playSong`'s whole-bar-flash-per-note, now matching
  levels 1/2.
- **A real castle-clear ending**: `sfxEnding` (all-four-levels-clear fanfare)
  was a generic invented descent with no connection to the actual game.
  Research turned up that SMB1 plays a *different*, more triumphant fanfare
  after a castle level than after a flagpole (Super Mario Wiki calls it "World
  Clear") — but no verbatim note-by-note transcription of that specific cue
  could be found, only a description ("Mario Cadence" idiom, C major, same
  family as the flagpole fanfare and the power-up SFX). Rebuilt to match that
  description: the same arpeggio idiom as the flagpole fanfare
  (`sfxLevelClear`), arranged as a call-and-response resolving a step higher,
  tagged 🔨 reconstruction like the fanfare it's built from — not claimed as
  verbatim, which would have been dishonest given what was actually sourceable.

**Emulation specs re-timed**: `playLevelIntro()` adds a new blocking delay at
boot and every level transition (~2.0 s for levels 1/2/4's intros, ~5.9 s for
level 3's — 13 notes at a waltz tempo), and levels 3/4's much longer full
victory themes push every later gap out further still. Recomputed every
`inputs:` timestamp in `lemon-piano.yaml`, `free-play.yaml`,
`hold-and-repeat.yaml` and `all-levels-win.yaml` from the melody tables'
`playSong()` math, then verified the computed numbers against a real run's
serial timestamps (measured: `Level 2` @18624 ms, `Level 3` @37958 ms, `Level 4`
@65537 ms, `ALL LEVELS CLEAR` @80788 ms — all comfortably inside the scheduled
margins). All four specs green, including `hold-and-repeat.yaml`'s buzzer
edge-count assertion, which incidentally now clears a threshold it fell just
short of before this change (pre-existing gap, not something this change set
out to fix). All three `pio run` envs still build clean.

## 2026-07-29 (V5 audio) — Mistake cue is now Mario's death rattle; win order fixed

Two owner-reported issues with V5's sound design:

- **The wrong-note tone was a plain low C2 beep**, not a Mario sound. Replaced
  with `sfxMistake` (`firmware/include/mario_sfx.h`): the opening `C5, G4` pair
  of the existing Death jingle, clipped to two notes (~180 ms) so a miss stays
  snappy — unlike the fuller `sfxDeath` used for a failed smart adjust. `NOTE_C2`
  and the now-unused `WRONG_TONE_MS` constant are gone; `wrongTone()` just calls
  `playSfx(sfxMistake)`.
- **The win sequence played the flagpole fanfare before the level's own theme**,
  which read as the "win" sound cutting into the middle of the theme that
  auto-plays afterward. Reordered in `handleGuess()`: the level theme
  (`playVictory()`) now plays out in full first, then the fanfare
  (`sfxLevelClear`), then — on clearing level 4 — the ending melody
  (`sfxEnding`), before advancing/wrapping. Docs (`README.md`,
  `versions/v5-led-bar/README.md` + `HARDWARE.md`, `docs/MARIO-SOUNDS.md`)
  updated to match; the "sit below everything" pitch-policy claim for the
  mistake/death cues was also corrected to describe the real trade-off (their
  C5/G4 register can sit inside a level's own key range, but they only ever
  play after the game has already called the guess wrong).

All three `pio run` envs (`nanoatmega328`, `nanoatmega328new`, `emulation`)
build clean. Headless verify: `lemon-piano.yaml` (covers a `WRONG` and a `WIN`)
and `all-levels-win.yaml` (all four levels' win sequences back to back) both
green — zero unexpected `WRONG`s, all `Level N` / `ALL LEVELS CLEAR` markers in
order. `free-play.yaml` also green. `hold-and-repeat.yaml`'s buzzer edge-count
assertion fails (3755 vs. required 4000) on this code **and** on the
pre-change code identically — a pre-existing issue, not a regression from this
change, left as-is.

## 2026-07-29 (V5 autoplayer, v5) — Manual clicks came back dead; a real short,
## not a mistake

After v4's fixes, autoplay worked end to end — but manual lemon clicks now did
nothing at all. Live-tested (Playwright again) with `window.__spiceDebug()`:
pressing key6 changed the voltage on unrelated nodes but NEVER on key6's own
node, while the finger wire's board-to-board connection to that exact same
node stayed rock steady. Root cause: the finger pin is a driven `OUTPUT`
(idle HIGH) sitting on the SAME node as the button — in the digital
fast-path both AVR boards use, that's a hard short, and the finger's
constant HIGH always won, so a real press could never pull the node down.

Tried making the finger pin high-Z (`INPUT`) while idle instead, so it
wouldn't fight a real click — manual clicks came back, but auto-play went
dead in the other direction, even with the press held 500 ms. This simulator
doesn't re-evaluate a pin's cross-board connection when its `pinMode`
changes at runtime, so once the finger toggled back to `INPUT`, nothing it
did afterward ever reached piano again.

Fix: keep the finger pin a permanent `OUTPUT` (set once, never toggled), but
route it through a 220Ω series resistor instead of a bare wire. That turns
the hard short into an ordinary voltage divider — a real button's near-zero
contact resistance still wins over 220Ω when someone clicks a lemon, and the
finger's driven LOW still wins over piano's own pull-up when auto-playing.
Had to also raise the press hold from 90 ms to 250 ms — the SPICE-resolved
divider needs time to settle, and 90 ms measured flaky (~1-in-3 misfires
across repeated live runs); 250 ms measured 9/9 reliable. Verified live, one
continuous session: autoplay wins level 1 → piano auto-advances to level 2 →
a manual click on level 2's first note registers a fresh `OK 1/10` right
after, no interference either direction.

## 2026-07-29 (V5 emulation) — A "virtual button" that plays every level for you

Levels 3 and 4 (added earlier today) had never been exercised by any automated
test — only level 1's code was ever injected. Added
[`emulation/all-levels-win.yaml`](versions/v5-led-bar/emulation/all-levels-win.yaml):
a scripted stand-in for a player who always presses the right lemon, driving the
game through all four secret codes back to back and asserting the auto-advance
chain all the way to `ALL LEVELS CLEAR` and the wrap back to level 1. Same
circuit as `lemon-piano.yaml`, only the input script + assertions differ — no
firmware change, no new pin (V5 has none spare).

Same circuit reused — same trap as any Velxio timed-input spec: a key pressed
while the AVR is blocked inside a melody's `delay()` is a voltage pulse the
firmware never polls for, so it is simply lost, not queued. First attempt
scheduled level 1's first press at t=2.0s; auto-calibration's boot guard
doesn't clear until **2.918s** (measured identically across two independent
runs — the AVR simulation is cycle-deterministic, not wall-clock), so the
opening presses vanished and the whole run silently played out against a game
stuck on level 1. Fix: start each level's input batch only after the previous
level's full victory sequence (fanfare + phrase gap + that level's theme +
phrase gap) has had time to finish — durations computed from
`firmware/include/mario_sfx.h`'s `{freq, ms}` tables, cross-checked against one
real measured run (WIN → `Level 2` = 9951 ms). Second attempt: clean pass, zero
`WRONG`s, all four `Level N` markers and `ALL LEVELS CLEAR` in one 62 s run.

**Follow-up, same day**: that spec is a fixed script, not a live control — asked
for something pressable. Added
[`emulation/autoplayer.yaml`](versions/v5-led-bar/emulation/autoplayer.yaml) (+
`autoplayer.vlx`): a **second Arduino** (`player`, an Uno) sharing the canvas
with the real V5 board (`piano`). It has a LEVEL SELECT button (cycles 1-4, 4
LEDs show which) and a PLAY button that fires the selected level's code onto
the SAME key nodes the real lemons use — piano cannot tell a player-board pulse
from a touch. The 7 lemons stay clickable throughout, so free play and
deliberate misses are still testable by hand; only the "type the whole code"
part is now optional.

Confirmed against `docs/PIPELINE_DESIGN.md` §9.3 and by trying both: multi-board
specs are **interactive-mode only** in this harness — neither `verify` nor
`document` will run them (`document` fails outright: "multi-board specs are
interactive-only in v1"). So this one couldn't get the usual headless
assertion pass; validated instead by compiling each board's sketch standalone
(`arduino-cli compile --fqbn arduino:avr:nano|uno`, both clean) and confirming
the `.vlx` generates (`routing: loose` — each key node is now a 3-way junction,
which the strict lane router flags but the circuit is still fully wired).

## 2026-07-29 (V5 autoplayer, v2-v4) — Three real bugs, found by actually
## driving the browser, not by guessing

First live test of `emulation/autoplayer.yaml` surfaced three symptoms: no
sound on manual lemon presses, PLAY and LEVEL SELECT both doing nothing, and
the board starting armed on level 2 instead of level 1. v2 shipped two
plausible-sounding fixes (finger pins high-Z while idle instead of driven
HIGH; real external pull-ups + a boot guard for PLAY/LEVEL SELECT instead of
`INPUT_PULLUP`) — **both were reasoned from reading Velxio's source, neither
was actually tested, and neither fixed it.** Re-tested live: still broken.

That's when this stopped being code-reading and started being an actual
investigation: a headless-Playwright probe (harness's `.venv` already has
Playwright + a downloaded Chromium) driving the real editor — import the
`.vlx`, click Run, dispatch `button-press`/`button-release` custom events on
the `wokwi-pushbutton` elements, read `wokwi-led.brightness` and each board's
own Serial Monitor tab. Three real, confirmed-live root causes:

1. **`isBoardComponent()` checks kind-prefixes, not the spec's ids.** It's a
   hardcoded list (`arduino-uno`, `arduino-nano`, `esp32`, …) checked with
   `startsWith` — the working multi-board UART *template* only works because
   it leaves `id:` unset, which defaults to the kind string. This spec gave
   its boards custom ids (`piano`, `player`) for readability, which silently
   broke every pushbutton's own pin-trace **including piano's own 7 keys** —
   not just the new ones. Fixed: board ids reverted to the default
   `arduino-nano` / `arduino-uno`.
2. **Interconnect.ts only bridges wires where *both* endpoints are literal
   board components.** The finger wires landed on a passive (the pull-up
   resistor's node), which is invisible to it — nothing ever propagated
   between the boards, regardless of firmware, regardless of the resistor
   fix from v2. Fixed: every key + the finger wires are now direct
   board-pin-to-pin or board-pin-to-button wires; any pull-up resistor is a
   side branch off the same board pin, never in the button's own path (a
   resistor in that path also tunnels the legacy pin-tracer onto the 5V bus
   and dead-ends at -1/GND — confirmed in the `[verify]` log: a multi-board
   project never gets a real SPICE solve, `nodes:["0"]` vs. 35 real nodes for
   the single-board project, so every button falls back to that legacy
   tracer instead of the correct SPICE-resolved path).
3. **Component click handlers are bound to ONE global simulator, not one per
   board** (`DynamicComponent.tsx`: `useSimulatorStore(s => s.simulator)`).
   A pushbutton wired to the second board's pin calls `setPinState` on the
   FIRST board's CPU instead of its own — a real architectural limitation of
   this Velxio version, not something fixable from spec/wiring at all. Serial
   I/O doesn't have this problem (`serialWriteToBoard(boardId, text)` is
   properly per-board), so PLAY/LEVEL SELECT became serial commands instead
   of physical buttons: type `1`-`4` into the second board's own Serial
   Monitor tab to arm a level, then `p` to play it.

Also fixed along the way, and it does hold up: the finger pins must be a
driven `OUTPUT` even while idle (idle = `HIGH`), never high-Z `INPUT` — this
simulator doesn't model true floating inputs, so an idle input reads a
spurious LOW, and once the finger wires are the direct board-to-board wires
#2 required, that floating LOW gets bridged straight onto piano's key nodes
and permanently "presses" them.

**Verified live, this time**: typed `1` then `p` into the second board's
Serial Monitor → piano's own log showed `OK 1/10` through `OK 10/10` → `WIN`,
zero `WRONG`s.

**Also fixed, in the shared harness** (`velxio-multi-board-emulator`, not this
repo): `.vlx` generation assigned wire colors by *list position*, so a single
electrical node split across multiple wire segments (any junction — which
every key node in this spec now is, with 3 wires apiece) rendered in
different colors per segment. Rewrote `_wire_color` into a net-aware pass
(union-find over every wire's endpoints, classify GND/VCC/signal per net, one
color per net) — verified against the regenerated `autoplayer.vlx`: 88 wires,
49 real nets, zero inconsistent colors. Regenerated `lemon-piano.vlx` too
(still verifies green) so the existing hand-play project picks up the fix.

## 2026-07-29 (V5 audio timing) — Fixed the sound race; silences are now policy

Reported after playing it: the win fanfare stepped on the tenth note. Root cause —
a key note SUSTAINS (`tone()` runs until the lemon is released), so the fanfare's
own `tone()` preempted it mid-sound. Fixed centrally rather than per call site.

- **`silenceKeyNote()`** lets a sounding note finish (its minimum length *and* the
  player's release, capped by `SUSTAIN_CAP_MS`) and then pauses, before a win or a
  miss speaks. **`hushBuzzer()`** stops the tone immediately for the smart adjust,
  where the player must keep touching and the buzzer has to be quiet anyway.
- **Note articulation**, and this was a real bug the measurement caught: notes
  inside an SFX table ran back-to-back, so the seven-note fanfare came out as a
  **single 1071 ms tone**, and Starman's repeated `F5 F5` would have been one long
  note. Every note now gives up its last `SFX_ARTICULATION_MS` (18 ms) to silence,
  taken FROM the note so the table's tempo is what you hear.
- **`SFX_TAIL_MS`** (60 ms) after every effect so seven calibration coins do not
  blur; **`PHRASE_GAP_MS`** (350 ms) between fanfare → level theme → ending melody.
- The sensitivity tick now **stays silent while a key note is sounding** instead of
  chopping it in half; the LED meter still reports the change.
- Closed a related race: if the player held the winning lemon past the 2 s release
  cap, that held key counted as the first guess of the NEXT level. It is now marked
  as already-used — release and press again, like any repeat.
- Measured after the fix (emulator, buzzer-pin edges): winning note plays out →
  **262 ms silence** → fanfare, seven notes at 92/242 ms with 18 ms between →
  **655 ms** → level theme. 10 926 B flash (36 %) / 461 B RAM. 3/3 specs pass,
  flashed to the board. Timing policy documented in `docs/MARIO-SOUNDS.md`.

## 2026-07-29 (V5) — Four levels, and every sound is Mario's

Researched the NES sound data, catalogued it, and rebuilt V5's audio around it.
V5.5 (the rotary-encoder proposal) is deleted — diagram, builder and index row —
though the generic `rotary_encoder` part stays in the wirewright engine, where it
is useful to any project.

**New: `docs/MARIO-SOUNDS.md`** — a global reference for every melody and effect,
with note/duration tables and, for each entry, a **provenance tag**: ✅ sourced
verbatim, 📐 transcribed from a cited letter-note tab, or 🔨 reconstructed from a
cited description. The accuracy genuinely differs between them and pretending
otherwise would be worse than useless. Sourced verbatim: coin (B5 100 ms → E6),
1-up (E6 G6 E7 C7 D7 G7 @125 ms), fireball (G4 G5 G6 @35 ms). Transcribed:
underwater and starman (melody voice only). Reconstructed: power-up (from the
documented Ab→Bb→C sweep), flagpole fanfare, ending melody.

**Two more levels — four in total**, each with its own key notes, code and theme:

| Level | Theme | Code |
|---|---|---|
| 1 | Overworld | 6,5,6,7,2,5,2,1,3,4 |
| 2 | Underworld | 3,6,1,4,2,5,3,6,1,4 |
| 3 | **Underwater** | 2,4,6,1,5,3,7,4,2,6 |
| 4 | **Starman** | 5,1,3,7,2,6,4,1,5,3 |

Two constraints the new codes had to satisfy, both consequences of earlier work: a
level's seven notes must be **distinct** (a guess is recognised by frequency), and
no code may repeat a note **back-to-back** (a repeat of the same key is filtered as
flaky fruit contact).

**Win flow**: clearing a level plays the **flagpole fanfare**, then that level's own
theme with the LED bar as a light show, then advances. Clearing level 4 plays the
**ending melody** and wraps to level 1. Serial now says `Level n` rather than
`Game n`.

**UI sounds are Mario effects** (`firmware/include/mario_sfx.h`, PROGMEM
`{frequency, ms}` tables with a `{0,0}` terminator, played by the new `playSfx()`):
fireball when calibration starts, **a coin per key measured** (seven coins = seven
keys), **power-up** when it is ready, a coin grace-note tick for the sensitivity
buttons (pitch still tracks the margin), a **bump** at the end stops, a coin per
smart-adjust sampling burst, **1-up** when it learns, the **death rattle** when it
cannot. `playTone()` now treats frequency 0 as a rest, so the tables can hold
silence.

- 10 716 B flash (35 %) / 461 B RAM (23 %). Flashed and confirmed: calibration now
  takes ~1.6 s while it counts out seven coins.
- All three emulation specs pass. Two fixes they needed: `extra_files` had to
  declare `mario_sfx.h` (the pipeline copies only the headers a spec lists, so the
  browser build failed to compile until it did), and the `Game n` assertions became
  `Level n`. The victory light-show assertion on LED 10 was relaxed from 40 edges
  to 2: the fanfare and themes now step the bar one LED per note instead of
  flashing the whole row, so the old count no longer described the behaviour.
- Docs: root README (level table + codes), V5 README (levels, sounds table, win
  flow), `docs/HARDWARE.md` and `CLAUDE.md` point at the new sound reference.

## 2026-07-28 (V5.5) — Drawn proposal: sensitivity on a KY-040 rotary encoder

**Diagram only**, at the owner's request: `versions/v5.5-rotary-encoder/` holds the
wiring and the reasoning, with no firmware and no emulation (the folders are absent
rather than empty, and the README says so up front). V5 remains the working board.

- V5's two sensitivity buttons are replaced by **one KY-040 encoder** (5 in stock,
  20 pulses/rev). Everything else is V5: pulled-up keys + GND clip, D2–D11 bar,
  buzzer on D13.
- **The pin budget decides the shape.** Ten LEDs + buzzer occupy eleven of twelve
  digital lines, so both buttons free exactly **D12 and A7** — two pins for a part
  that wants three. **CLK → D12** (the line that must not be missed gets the real
  digital pin), **DT → A7** (only read at the instant CLK changes, so an
  `analogRead`'s ~112 µs is irrelevant), and the **shaft pushbutton is left
  unconnected**. Consequence for whoever writes the firmware: V5's smart-adjust
  gesture needs a new trigger — "keep turning against the end stop" costs no wire.
- Noted in the diagram and README: driving the bar through a **74HC595** would free
  seven pins and let CLK/DT sit on D2/D3 (the hardware-interrupt pins), which is the
  robust way to decode quadrature — but that is a different board, so a different
  version.
- **Engine**: new `rotary_encoder` component in `../eda-wirewright` (knob + header,
  ports `vcc`/`gnd`/`clk`/`dt`[/`sw`]) with a `with_sw` flag, so a deliberately
  unwired pushbutton is drawn greyed instead of tripping the DRC's unconnected-pin
  rule. Registered for the JSON/CLI/MCP contract; the engine's 11 tests still pass.
- 9 diagrams now render, all with 0 DRC violations.

## 2026-07-28 (V5 rebuilt) — GND-clip keyboard, live sensitivity, no restart/select

V5's board is **replaced in place** at the owner's request, rather than becoming a
new version: the previous V5 board (floating +5 V-clip keyboard, A7 game-select
switch, D7 restart button) now lives only in git history and in this log. Its
firmware, emulation and diagram are all rebuilt.

**Hardware**

- **Keyboard back to the 2019 arrangement**: 220 Ω pull-up per key, player holds a
  **GND** clip, so a touch drags the reading DOWN. This is the change that made the
  keyboard readable: idle went from ~250 with 76-104 counts of noise and ~170
  counts of drift (floating pins) to **1022 with 1 count of noise** on every
  channel. Measured before/after on the board.
- **Off**: game-select switch (A7) and restart button (D7).
- **On**: **SENS +** on D7 (to GND, internal pull-up) and **SENS −** on A7 (to GND
  with an external 10 kΩ pull-up — A7 has no internal one, and is read with
  `analogRead(A7) < 512`). Ten LEDs + buzzer take eleven of twelve digital lines,
  which is why the second button had to be an analog pin.

**Firmware** — the V2.5 front end merged into the V5 game:

- Boot **auto-calibration**: per-key baseline *and* idle noise, margin derived as
  `max(4, 2 x worst noise)`; on this rig that lands on margin 4 → threshold 1018,
  the working point found by hand. The **LED bar is the progress display** (one LED
  per key as it is measured = hands off the fruit) and then shows the chosen
  sensitivity as a level meter.
- **Sensitivity knob on the two buttons**: 1-count steps below margin 20, 5 above,
  auto-repeat while held, end stops reported and sounded. The bar shows the level
  after each press; the tick's pitch tracks the setting.
- **Smart adjust** (both buttons 1 s while touching a lemon): identifies the touched
  key, measures the other channels' wander as the noise floor, sets the margin
  midway between them, and refuses to change anything if the touch is not separable
  from noise. Samples in four bursts with the blip BETWEEN them, never during, so
  the buzzer current cannot pollute the reading it is learning from.
- **A key sounds once** until a different key is played (was: only the *game*
  ignored repeats; now the note does too).
- **UI sound vocabulary** at 3.3-4.7 kHz — above every game note (game 1 reaches
  G7 = 3136 Hz, game 2 runs 220-587 Hz) and where a piezo is loudest: calibration
  start/finish, button ticks, end-stop, smart-adjust progress/success/failure,
  stuck-key warble. One `playTone()` now serves hardware and browser, which
  retired `emuTone()`.
- Sensing rewritten for the new polarity: `threshold = baseline − margin`,
  `strongestKey()` picks the channel that dropped furthest below its threshold,
  `TOUCH_HYSTERESIS` down to 2 counts because the whole signal is ~4.
- 10 114 B flash (33 %), 393 B RAM (19 %). Flashed and confirmed on hardware.

**Emulation** — all three specs green, and two assertions had quietly rotted:

- The keys need **no polarity shim any more**: a browser pushbutton with a pull-up
  *is* this sensing model. Only the buzzer (D11, the `OCR2A` rule) and key 7 (D12,
  no A6 in avr8js) still move. The **boot guard is back** and matters more than
  ever — until the first SPICE solve every input reads LOW, which would calibrate
  every baseline to 0.
- **No sensitivity buttons in the browser**: ten LEDs + buzzer + key 7 use all
  twelve digital lines. The buttons and smart adjust are verified on the V2.5 rig,
  which is the same front end without the bar.
- The boot chirps put **2091 edges** on the buzzer pin before any key is touched,
  which silently defanged two `gpio_toggles` thresholds (1000 and 200). Re-measured
  and re-armed at 4000 (sustain, distinguishes 5307 from ~2460) and 3000 (free play,
  measured 3625), with the arithmetic written into the specs so the next person can
  re-derive them.

**Pin map rewired for buildability (same session)** — the bar is now **one
unbroken ascending run, LED n on pin n+1**: LEDs 1-10 on **D2..D11**, **SENS +** on
**D12**, **buzzer on D13**, SENS − still on A7. Wiring the bar left to right no
longer needs a lookup table. The buzzer took D13 rather than the button because
D13 carries the Nano's on-board LED through ~1 kΩ, which fights a ~30 kΩ internal
pull-up and can read as permanently pressed; a buzzer is indifferent to that load.
(D0/D1 stay off-limits — they are the UART, i.e. the serial monitor this rig is
tuned with and the pins avrdude uploads through.)

**Diagram** — `build_v5()` rewritten: 2019 pull-up comb + GND clip, ten-LED bar,
both buttons with the A7 pull-up drawn, no switch and no restart. 0 DRC violations.

## 2026-07-27 (V2.5) — New version: keyboard test with a LIVE touch threshold

V2's touch detection hangs on one hardcoded number (`<= 1019`), and finding a
working value meant edit-compile-reflash for a figure that depends on the fruit,
the PSU, the mains outlet and where your feet are. V2.5 makes it adjustable while
the piano runs. Two buttons is a hardware change, so per `docs/VERSIONING.md` this
is a new version rather than an edit to the 2019 sketch — V2 stays as it was.

- **Firmware** (`versions/v2.5-threshold-buttons/firmware/`, new 2026 code): the
  threshold is the variable `touchThreshold`, stepped 5 at a time by **THRESHOLD +
  (D10)** and **THRESHOLD − (D11)**, with debounce and auto-repeat while held
  (400 ms, then every 120 ms) so a 0-1023 sweep is one long press. Serial monitor
  **on** at 9600 (V2's `Serial.begin` is commented out): a live readout of all
  seven 4-sample averages against the threshold twice a second, with a `*` on each
  channel counted as touched, plus every threshold change and every note played
  with the reading that triggered it. Detection is edge-triggered, so a held key
  plays once instead of machine-gunning. 5 188 B flash, 306 B RAM.
- **Buttons are active-LOW to GND on the internal pull-ups** — no external
  resistors, deliberately unlike V3/V4/V4.5's active-HIGH + 10 kΩ pulldown. A
  bench rig should need as few parts as possible.
- **`TOUCH_WHEN_BELOW` flag**: `true` keeps V2's 2019 polarity (touch drags the
  pin down); `false` switches the comparison for 2026 wiring (touch pushes it up).
  Without the right setting no threshold value can work, so it is stated in the
  banner the firmware prints at boot.
- **Emulation** (`emulation/keyboard-test.yaml`, `--mode verify` **pass** first
  run): seven clickable keys **and both threshold buttons**, so the point of the
  version is playable in the browser. The script plays keys 1/2/7, taps the
  threshold down twice and up once, then plays key 3; it asserts the notes, the
  buzzer edges and `>>> threshold=1014 / 1009 / 1014`. Keys 1-6 need no shim — a
  pushbutton with a pull-up *is* the 2019 polarity — only the buzzer (to D9, a
  duty-polled PWM pin) and key 7 (to D12, since avr8js has no A6) move.
- **Diagram**: new `build_v2_5()` contract → `images/wiring-v2.5.png` (23 nets, 0
  DRC violations), plus a new `_button_to_gnd()` helper in
  `tools/wiring_diagrams.py` for active-low buttons with no pulldown to draw.
- Docs: `README.md` + `HARDWARE.md` for the version, rows in both index tables,
  `CLAUDE.md` and `docs/HARDWARE.md` updated.
- Not yet flashed to hardware: the board dropped off USB (clean `USB disconnect`,
  not the old `error -71`) before the upload could run.

## 2026-07-27 (V5 input) — One key at a time; and the measurement that says the pull-downs are mandatory

Hardware testing of the previous entry did not behave as asked, and the serial log
said why: `OK 1/10` followed by `WRONG` **in the same millisecond**, seven times
over. The repeat filter blocks the same key, so that pattern can only mean two
*different* channels were counted — one finger read as two keys.

**Firmware (verified in emulation, NOT yet verifiable on this board — see below):**

- `strongestKey()` replaces "first index above threshold": the scan now takes the
  channel with the largest margin over its own threshold, so a real touch beats
  coupled ghosts instead of racing them.
- While a key is down, **every other channel is ignored** until it is released.
  `keyHeld[]` is gone — `activeKey` is the single source of truth, which makes
  "one key at a time" structural rather than a patch.
- New `TOUCH_HYSTERESIS` (60 counts): a key releases only when it falls that far
  *below* its press threshold, so a reading sitting on the line cannot chop a
  sustained note into pieces or re-trigger guesses. `keyStillDown()` is used by
  both the scan and `waitKeyRelease()`.
- New env `nanoatmega328-debug` (`-DDEBUG_TOUCH`): logs every accepted press with
  the chosen key and all seven raw ADC readings. Same game logic, extra logging.
- All three V5 emulation specs still pass.

**Measured on the real board (bench sampler, 7 channels at 20 Hz, four held
touches of 1.9-32.5 s) — the keyboard as currently wired cannot be read:**

| Measure | Result |
|---|---|
| Samples with all 7 channels above 400 | 50 % |
| Samples with zero channels above 400 | 38 % |
| Samples pinned at 1023 / at 0 | 27 % / 23 % |
| Highest vs second-highest channel | **median 5 counts** |
| Channel reading highest, whichever lemon was touched | A0 or A6, 92 % of samples |

Five counts of spread across seven channels means the ADC is not resolving seven
voltages; readings arrive as gradient ramps (`148 194 258 332 400 469 545`), i.e.
sample-and-hold residue from the previous channel, because the source impedance is
effectively open. The idle level also drifts ~170 counts on a ~25 s cycle — about
75 % of the touch margin `calibrate()` sets at boot, so thresholds go stale within
seconds. This data cannot distinguish "pins shorted together" from "pins
effectively open"; both look the same when the ADC cannot charge its S/H.

So the fix is hardware: **~1 MΩ from each analog pin to GND**, the ⚠️ that has been
in `docs/HARDWARE.md` since the rescue. Written up with the numbers in
`versions/v5-led-bar/HARDWARE.md`, including how to confirm it worked (the
calibration baselines should fall from ~250 to near 0). The firmware changes above
are the right behaviour once channels are distinguishable, but they are unverified
on hardware until then.

## 2026-07-27 (V5 touch) — Notes sustain while held; a key only ever counts once

Second round of V5 gameplay work, again code-only (the board is unchanged, so this
stays inside V5). Lemons make flaky contacts, and the game was reading that flakiness
as gameplay.

- **A held key keeps sounding.** `keyTone()` became `startKeyTone()` +
  `stopKeyTone()`: `tone()` is called with no duration, so the note runs until the
  lemon is released, with `NOTE_DURATION` (70 ms) as a floor so a quick tap is still
  a proper note. New `activeKey` tracks what is sounding. Measured on the emulator:
  a 600 ms hold → 584 ms tone; 80 ms taps → 84 ms; 250 ms hold → 250 ms.
- **A long press is still ONE guess**, and **re-pressing the same key never counts
  again** (new `lastCountedKey`): repeats sound but do not reach `handleGuess()`
  until a *different* key is played. Four touches of the same key now produce
  exactly one `OK 1/10` — previously each one was a fresh guess, so a flickering
  contact could spray guesses and fail the round on its own.
  **Consequence to remember: a secret code can no longer contain the same note
  twice in a row.** Neither of the two codes does; check it if a third is added.
- The wrong tone now waits for the **release** rather than a fixed 70 ms
  (`waitKeyRelease()`), capped by the new `SUSTAIN_CAP_MS` (2 s) so a stuck or
  ghosting key cannot freeze the game. `resetBoard()` clears the sustain/repeat
  state so a new round may legitimately open with the key that ended the last one.
- **New spec `emulation/hold-and-repeat.yaml`** (green): key 6 held 600 ms then
  tapped three more times, nothing else played — asserts ≥1000 buzzer edges by
  t=3 s (only a real sustain reaches that; a tap gives ~370 at E7), `OK 1/10`
  present, and `OK 2/10` / `WRONG` **absent** for the whole run. The main spec's
  act 3 now presses the first note twice as well, proving the filter does not eat
  real guesses. All three V5 specs verify green.
- Flashed to the Nano on `/dev/ttyUSB0`: 7 502 B flash (24.4 %) / 319 B RAM,
  written and verified, boots and calibrates clean.

## 2026-07-27 (V5) — Piano first, puzzle second: free play + a wrong tone that waits its turn

Two gameplay changes to V5, requested after playing it on hardware. The board is
unchanged, so this stays inside V5 (code-only → no new version).

- **Free play until the sequence starts.** While the bar is empty
  (`currentStep == 0`) any key just sounds its note: no `WRONG`, no low tone, no
  penalty. The puzzle begins only when the player happens to hit the code's
  **first** note (LED 1 lights); from then on a miss blanks the bar and buzzes as
  before, dropping them back to free play. The instrument no longer scolds you for
  exploring it.
- **The wrong tone no longer cuts off the note you played.** `keyTone()` now
  records `keyToneEndsAt`, and the miss path calls a new `waitKeyToneEnd()` before
  `wrongTone()`: the pressed key's note finishes (`NOTE_DURATION` 70 ms), then a
  new `WRONG_TONE_GAP_MS` 60 ms silence, then the 200 ms C2. So you always hear
  *which* note was wrong, then the verdict. Works for both builds — on hardware
  `tone()` is non-blocking so there is real time to wait out; in the Velxio build
  `emuTone()` has already blocked, leaving just the gap.
- **Verified on the emulator, by measurement** — buzzer-pin edges around a miss:
  69.9 ms at ~1330 Hz (E6, the key played) → **60.5 ms silence** → 200.0 ms at
  ~70 Hz (C2, the wrong tone). No overlap.
- **Emulation tests extended, both green**: `lemon-piano.yaml` now plays three
  acts — free play, then first-note-and-miss (asserts `WRONG` *does* fire), then
  the full code to `WIN` and auto-advance. A second spec **`free-play.yaml`**
  presses five non-matching keys and asserts the buzzer sings while `WRONG`, `OK `
  and LED 1 all stay absent. (`serial_absent` is run-wide, not windowed — hence
  two specs rather than one.)
- Flashed to the hardware Nano (`/dev/ttyUSB0`, env `nanoatmega328`): builds to
  7 200 B flash (23.4 %) / 315 B RAM, uploaded and verified, boots and calibrates
  clean.

## 2026-07-26 (V0 field result) — The bad "buzzer" was a bad Arduino

V0 flashed and run on real hardware for the first time, and it solved the problem
it was written for on the first try.

- **Flashed**: `pio run -t upload` on env `nanoatmega328` (old bootloader, 57600) —
  4 020 bytes written and verified, device signature `0x1e950f`. Serial confirms the
  hardware build (`buzzer pin: D8`) and the scale looping cleanly: `262 → 523 Hz`
  and back, `scale done - pausing`, repeat.
- **Root cause of the reported bad sound: the Arduino board, not the audio.** The
  original 2019-era Nano produced one continuous/discontinuous tone regardless of
  firmware; the same V0 build on a **different Nano** played the scale perfectly.
  Not the buzzer, not the wiring, not the amplifier, not the game code.
- The old board's **CH340 also failed** during the session: every plug-in aborted
  with `error -71` (`unable to enumerate`) across two USB ports, two cables, with
  the amp unpowered and with the board fully off the breadboard. Its ATmega may
  still run, but it cannot be reflashed over USB — ICSP only.
- Recorded in `versions/v0-buzzer/HARDWARE.md` as step 7 of the troubleshooting
  checklist ("suspect the board itself") plus a field-result note in that version's
  `README.md`, so the next person reaches for a spare board before rewiring.
- Host-side note: `99-platformio-udev.rules` was installed during the session, so
  `/dev/ttyUSB0` now comes up `crw-rw-rw-` and uploads need no group juggling.

## 2026-07-26 (V0) — New buzzer bring-up board: a scale, forever

Added **`versions/v0-buzzer/`** after a report that the buzzer "doesn't sound as
usual". Trying to diagnose it with V2 was a dead end: V2 is a 2019-wiring sketch
(pull-ups + GND clip, touch = reading *drops* to ≤ 1019), so on a 2026-wired board
every key reads as permanently pressed and the seven `tone()` calls cut each other
off — exactly the "single discontinuous tone" observed.

V0 is the smallest board in the project — an ATmega328 and one passive buzzer on
**D8**, the same pin every version uses, so it runs on an existing build without
moving a wire. It is numbered 0 because its board is a *subset* of every other
version; it is a 2026 diagnostic, not part of the 2019 lineage (V1 remains the
historical origin).

- **Firmware** (`v0-buzzer/firmware/src/main.cpp`): C-major scale C4→C5 and back,
  300 ms per note with 80 ms of real silence between notes, 700 ms between passes,
  looping forever. The on-board LED (D13) is lit for each note — so "LED steps
  through the scale but no sound" isolates the fault to the buzzer/wiring/pin — and
  every note is logged at 9600 baud (`note 3/14 - 330 Hz`, ASCII only).
- **Both playback paths, selectable at build time**: default `tone()`/`noTone()`
  (the key-note path) and `-DUSE_BUZZ` → bit-banged `buzz()` (the path
  `playSong()` uses for the Mario themes), as env `nanoatmega328-buzz`. Flashing
  both tells you whether one path degraded or the hardware did. 5 envs, all green.
- **Emulation**: new spec + `emuTone()` shim (buzzer on D11 in the browser).
  `--mode verify` **pass** — banner, `path: tone()`, 7 496 edges on the buzzer pin,
  and `scale done` proving notes actually END (a note that never stops is the
  classic browser failure). Doubles as the reference recording to compare by ear.
- **Diagram**: new `build_v0()` contract (4 nets, 0 DRC violations) on a smaller
  canvas — `_board()` now takes `w`/`bx`/`by` so a tiny board gets a tiny page.
- **Docs**: `README.md` + `HARDWARE.md` for V0, including an ordered "if it still
  sounds wrong" checklist (D13 running?, passive vs **active** buzzer, module
  polarity, `tone()` vs `buzz()`, and the trap of a `-DVELXIO_EMULATION` build
  which moves the buzzer to **D11**). Both index tables, `CLAUDE.md` and
  `docs/HARDWARE.md` updated; V1's README now notes V0 is a subset of it.
- Also added the missing `nanoatmega328new` env to **V2** (new-bootloader Nano),
  so that board can be flashed too; V2 builds on all three targets.

## 2026-07-26 (V4.5) — V4+ renamed to V4.5, and its relay pair + water pump removed

`versions/v4-plus-margin-buttons/` is now **`versions/v4.5-margin-buttons/`**, and
that board no longer drives the water pump. The penalty survives as sound: a miss
from note 7 onward plays the low warning groan and counts a fail, ten fails still
end the game with the death tune. V4 keeps the pump — it is what makes V4 V4.

- **Firmware** (`v4.5-margin-buttons/firmware/src/main.cpp`): removed `RELAY_1`
  (D5) / `RELAY_2` (D6), the boot-time defined-OFF writes, both `pinMode`s,
  `pumpOff()` and `firePump()`. `firePump()` → **`playPenalty()`** (groan only);
  `PUMP_FROM_STEP` → `PENALTY_FROM_STEP`, `PUMP_MS` → `PENALTY_MS`. Serial banner is
  now `Lemon Piano V4.5`. D5/D6 are unused on this board. All 4 envs build green.
- **Emulation**: dropped the blue pump-indicator LED and its 220 Ω + wires from
  `lemon-piano.yaml`; the `serial_contains` assertion now expects
  `Lemon Piano V4.5`. `--mode verify` **pass**; `.vlx` regenerated from the run.
- **Diagram**: `build_v4(plus=True)` no longer adds the relay module or the pump
  (37 nets, 0 DRC violations) → `versions/v4.5-margin-buttons/images/wiring-v4.5.png`
  (renamed from `wiring-v4-plus.png`); `TARGETS` key is now `"v4.5"`.
- **Docs**: V4.5 `README.md` + `HARDWARE.md` rewritten around the two deltas
  (− relay/pump, + MARGIN buttons); V4's "next revision" text, V5's hardware delta
  (now just − red LED and − MARGIN buttons, since the pump left with V4.5), both
  index tables, `CLAUDE.md`, `docs/HARDWARE.md` and `docs/VERSIONING.md` updated.
  Earlier changelog entries keep the old `V4+` / `v4-plus-margin-buttons` names —
  they were accurate when written.

## 2026-07-26 (restructure) — One version per hardware revision, none archived

Replaced the "active version + archive" layout with a flat set of **six active
hardware revisions** under `versions/`. A version now means a *board*: change the
hardware and you create a new version; change only code and you stay put. The rule
and the checklist for adding one live in the new **`docs/VERSIONING.md`**.

New layout (each directory is self-contained — firmware, emulation, images, docs):

| Version | Was | Hardware delta |
|---|---|---|
| `versions/v1-banana-piano/` | `archive/banana-piano-original/banana-piano/` | origin: 7 fruit keys, speaker D8, HC-SR04 mounted |
| `versions/v2-keyboard-test/` | `archive/banana-piano-original/keyboard-test/` | − HC-SR04 |
| `versions/v3-game-prototype/` | `archive/banana-piano-original/game-prototype/` | + red/green LEDs, game button, 1 relay |
| `versions/v4-water-pump/` | *git history* (`0234d02^`) | clip → +5 V, 2nd relay + pump, RESTART |
| `versions/v4-plus-margin-buttons/` | `archive/lemon-piano-v4/` | + MARGIN +/− buttons (D10/D11) |
| `versions/v5-led-bar/` | top-level `firmware/` + `emulation/` | − relays/pump/red LED/margin buttons, + ten LEDs, select → A7 |

- **V4 and V4+ are now separate versions.** The archived snapshot had absorbed the
  2026-07-25 touch upgrade, so the *board* it documented (V4) no longer matched its
  firmware. The pre-upgrade firmware + emulation were restored from git history
  into `v4-water-pump/`, and the upgraded pair stayed as `v4-plus-margin-buttons/`.
  Both now verify green with their own `.vlx` regenerated from their own firmware.
- **V1–V3 became buildable.** Each 2019 sketch keeps its Arduino-IDE folder and
  gained a `platformio.ini` (`src_dir = <sketch>`, envs `nanoatmega328` + `uno`);
  all three compile. Their code is otherwise untouched.
- **Three new wiring diagrams** — `wiring-v1/v2/v3.png` — for boards that had none,
  including the 2019 pull-up comb and GND-clip polarity. Diagram contracts moved to
  one builder per version (`TARGETS` in `tools/wiring_diagrams.py`) and now render
  into `versions/*/images/`. All six: 0 DRC violations (23/19/32/36/42/55 nets).
- **wirewright engine extended** (`../eda-wirewright`): new `ultrasonic` (HC-SR04)
  component, `relay_module(channels=1|2)`, parametrised `clip_box` (title/sub/size),
  `buzzer(label, pin_label)` and `water_pump(note)`; `ultrasonic` registered for the
  JSON/CLI/MCP contract.
- **Docs rewritten around the new model**: root `README.md` (version matrix),
  `versions/README.md` (comparison table), per-version `README.md` + `HARDWARE.md`
  (12 new files), `docs/HARDWARE.md` reduced to shared fundamentals (touch physics,
  the V3→V4 polarity flip, common parts), `CLAUDE.md` rewritten for the new rules.
- **V1–V3 have no emulation, on purpose.** Hosting them in the browser AVR would
  mean editing 2019 sketches (buzzer onto a PWM pin with `OCR2A` cleared, key 7 off
  A6, pull-ups instead of the divider). Each `emulation/README.md` documents the
  blockers; the decision is TODO #16.
- Findings recorded while documenting: V3's relay `digitalWrite()` burst is inside a
  commented block (wired but never fired); V2's `Serial.begin(9600)` is commented out
  while its `Serial.print` calls are live; every 2019 sketch reads `analogRead(6)`,
  which the Uno's DIP package cannot provide (A6 is TQFP-only).
- Verified after the move: 6/6 firmwares build (v1–v3 `nanoatmega328`+`uno`, V4/V4+
  4 envs, V5 3 envs); V4, V4+, V5 emulation `--mode verify` **pass**; all six
  diagrams re-rendered clean.

## 2026-07-26 — Wiring engine extracted to its own repo (eda-wirewright)

The schematic engine that was living in `tools/schematic/` is now a standalone,
professional package: **[eda-wirewright](../eda-wirewright/)** (CLI, declarative
JSON contract format, MCP server for AI, Docker, tests, CI). This project just
*consumes* it now:

- Removed `tools/schematic/` (moved, not deleted — it's `eda-wirewright/src/wirewright/`).
- `tools/wiring_diagrams.py` imports `wirewright` via a `sys.path` shim to
  `../eda-wirewright/src` (no install needed, only Pillow). The three contracts
  are unchanged; `python3 tools/wiring_diagrams.py` still regenerates
  `docs/images/wiring-{v4,v4-plus,v5}.png` (36 / 42 / 55 nets, 0 DRC violations).

## 2026-07-25 (engine) — Real schematic engine: auto-router + DRC (no more overlaps)

Rewrote the wiring-diagram generator from hand-placed coordinates into a proper,
reusable **schematic engine** (`tools/schematic/`, ~1160 lines) — because
patching coordinates by hand kept producing the same faults (wires over
resistors, confusing crossings, wires too close, resistors left visually
unconnected, wires under components). Now the diagrams are *declarative
contracts* and the engine guarantees correctness:

- **Declarative model**: you state components, typed ports and nets (what
  connects to what); a 3+-terminal net (e.g. a Nano pin + button + pulldown that
  are one node) routes as one clean tree. `tools/wiring_diagrams.py` is now just
  the three contracts (~200 lines).
- **Grid A\* maze router** (`router.py`): shortest orthogonal paths on a routing
  grid, component bodies (+clearance) are hard obstacles so a wire can never
  cross one, a large **bend penalty** yields long straight runs, a light
  **proximity penalty** keeps wires apart (they cross cleanly rather than
  detour), and every pin gets a perpendicular **escape stub**. Labels are a soft
  obstacle so wires route around text.
- **DRC that runs before every save** (`validator.py`): raises on wire-over-body,
  coincident wires, unconnected pins, or wires-too-close. The three diagrams now
  report **0 hard violations, 0 spacing warnings** (36 / 42 / 55 nets). A broken
  diagram literally cannot be saved.
- Design follows standard EDA practice (Lee/A\* maze routing, bend/proximity
  cost, net ordering, junction-dot rules, label placement). See
  `tools/schematic/README.md`.

Same three outputs (`docs/images/wiring-{v4,v4-plus,v5}.png`), regenerated with
`python3 tools/wiring_diagrams.py`; canvas widened to 3400 px for breathing room.

## 2026-07-25 (diagrams) — Fix overlapping wires in the wiring diagrams

`tools/wiring_diagrams.py` v1 let several control wires (game-select, buzzer,
restart, MARGIN+/-) share the same y as the component row they fed into, so a
wire bound for a farther component cut straight through a nearer component's
body (and some wires' lanes coincided with LEDs' full-height GND drops).
Fixed with a `connect_over_top()` router: every controls-row wire first climbs
to its OWN private "highway" level — all comfortably above the LED tops — then
crosses over and drops straight down into its own terminal, so no wire ever
overlaps a component's bounding box. Canvas widened (2500→3700px) to give
each component (LEDs, relay+pump, game-select, buzzer, restart, MARGIN+/-)
its own clear horizontal slot. V5 also got its RESTART/BUZZER/LED-bar
positions rebased off `NANO_X1` (they were absolute pixel coordinates that the
wider canvas had shifted the Nano away from). Regenerated all three PNGs.

## 2026-07-25 — V4 touch upgrade (in-place) + manual MARGIN buttons + wiring diagrams

Backported V5's touch-sensing improvements onto the **archived V4** and added a
live manual override. The V4 **game is unchanged** (relay water pump, red/green
LED, 10-penalty death tune, both secret codes all behave exactly as before) —
only the touch layer and two new hardware buttons changed. This deliberately
edits the otherwise-frozen `archive/lemon-piano-v4/` (owner-authorised; the
pristine 2019 original still lives in git history).

- **Noise-adaptive calibration ported to V4** (`archive/lemon-piano-v4/firmware/src/main.cpp`):
  the fixed `TOUCH_MARGIN 100` is gone. `calibrate()` now takes 64 samples over
  ~128 ms per key (mean baseline + peak) and sets
  `threshold = baseline + max(MIN_TOUCH_MARGIN=40, NOISE_FACTOR=3 × (peak−mean))`,
  capped at `THRESHOLD_CAP=900`. It runs at boot **and on every RESTART**
  (moved from `setup()` into loop's `!started` branch), so ghost presses from a
  new fruit / outlet / PSU are re-tuned by pressing RESTART (hands off).
- **Manual MARGIN buttons (hardware only)**: `MARGIN +` on **D10** and
  `MARGIN −` on **D11** (active-HIGH, wired like RESTART, edge-triggered). They
  shift a `manualMargin` offset applied on top of each key's auto margin
  (`applyThresholds()`), bounded to `[−120, +400]` in `MARGIN_STEP=10` nudges,
  with an `EFFECTIVE_MARGIN_FLOOR=8` so a threshold never sits on the baseline.
  Each press gives an audible tick (high = up, low = down) + a serial line.
  Emulation build is untouched — the buttons are `#ifndef VELXIO_EMULATION`
  (its keys are active-low digital and D11 is the buzzer there).
- **Verified**: all four PlatformIO envs green (nanoatmega328 / …new / uno /
  emulation; hardware RAM 341 B / 16.7 %), and the V4 Velxio emulation
  `--mode verify` regression still passes (game 2's code → `WIN`).
- **Wiring diagrams** (new `tools/wiring_diagrams.py`, pure-PIL, adapted from
  the oscilloscope project's technique) → `docs/images/wiring-{v4,v4-plus,v5}.png`:
  one per version (original V4, V4+ with the touch upgrade, and V5), each with
  the 7-lemon keyboard, colour-coded wires and a legend.

## 2026-07-14 (calibration) — Noise-adaptive touch margin, recalibrate on RESTART

Fixes the critical hardware ghost-press problem (margin needs differ per
fruit / mains outlet / 5V PSU — a fixed `TOUCH_MARGIN 100` can't serve all):

- `calibrate()` now measures each key's real noise: 64 samples over ~128ms
  (spans 6+ mains cycles at 50Hz), records mean baseline AND peak, and sets
  `threshold = baseline + max(MIN_TOUCH_MARGIN=40, 3x(peak-mean))`. Quiet
  supplies get MORE sensitive than the old fixed 100; noisy chargers get a
  margin wide enough to kill ghost presses. Thresholds are capped at 900
  with a serial warning (`VERY NOISY - check fruit contact / power supply`).
- **Recalibration on every RESTART press**, not just power-on — `calibrate()`
  moved from `setup()` into the `!started` branch. If the piano misbehaves
  after changing fruit or outlet: hands off the lemons, press RESTART.
- The LED bar sweeps one LED per key during calibration (visual "hands off"
  feedback), and serial logs per-key `baseline / noise / threshold`.
- Emulation path untouched (its calibrate() is the solver boot guard);
  headless verify green; all three PlatformIO envs green (flash 20%).

## 2026-07-14 (light show) — LED bar flashes to the victory theme

`playSong()` now flashes the whole ten-LED bar to the beat: lit while each
note sounds (`buzz` blocks for the note's duration), dark in the inter-note
gap and during rests. Shared game logic — identical on hardware and in the
browser build (new `allLedsOn()` helper mirrors `allLedsOff()`). Verified
headlessly: LED 10 (D13), which only ever lights during the win, toggles 61
times across the Mario victory tail. All three PlatformIO envs green.

## 2026-07-14 — V5: ten-LED progress bar, auto-advancing games

New hardware version. **V4 is frozen** in `archive/lemon-piano-v4/` (firmware +
emulation) and the top-level `firmware/` + `emulation/` are now V5.

- **Removed**: the 2-channel relay pair + water pump, the red LED, and the
  fail-counter / death-tune game-over that was coupled to the pump.
- **Added**: a **ten-green-LED progress bar** on
  `D2,D3,D4,D5,D6,D9,D10,D11,D12,D13`. Each correct note lights the next LED; a
  wrong note blanks all ten (with a short low tone) and restarts the sequence;
  ten lit = win.
- **Auto-advance**: winning a theme flips the game to the other one
  (1 → 2 → 1 …), so both games cycle from a single starting point. On hardware
  the A7 switch picks the *starting* game; the browser build starts at game 1.
- **Game select moved D4 → A7** (analog-in) to free the digital pin the 10th LED
  needed. V5 therefore needs a Nano/Mini (uses A6 **and** A7) — the `uno` env is
  dropped.
- **Emulation reworked for V5** (`emulation/lemon-piano.yaml` + `.vlx`): ten
  green LEDs, keys 1–6 on A0–A5 + key 7 on D12, buzzer on D11. Ten LEDs consume
  every free browser pin, so the emulation has no game-select/restart (it relies
  on the auto-advance). **Headless verify green**: injects game 1's code →
  `Game 1 → OK 1/10 … 10/10 → WIN → Game 2` (all ten LED pins + buzzer toggling).
- Builds green on `nanoatmega328`, `nanoatmega328new`, `emulation` (RAM 311 B /
  15 %, flash 6.7 KB / 22 %).

## 2026-07-13 (routing) — Hard wire-routing rules in the pipeline

The harness now enforces reference-diagram wiring on every generated `.vlx`
(`velxio_harness/routing.py`, based on exact pin geometry extracted from the
live Velxio DOM): **no cable may cross any component** (generation aborts on
violation), no two cables may ride the same lane (staircase nesting in
adjacent 10px lanes, shortest wires innermost), long runs travel in
corridors instead of along pin rows, and stubs exit perpendicular to the
component edge. The lemon piano regenerates with 34 of 46 wires auto-routed
around the components; headless verify (secret code → WIN) still green.
Rules documented in the harness AGENTS.md.

## 2026-07-13 (final) — Emulation polish: keyboard play, switch, clean wiring

- **PC keyboard play**: keys `1`–`7` now play the corresponding lemon
  (Velxio frontend patch `0002-keyboard-shortcuts-for-labeled-pushbuttons`
  in the harness repo — digit keys dispatch press/release to the pushbutton
  labeled with that digit).
- **Game selector is now a slide switch** on D4 (left/GND = Mario, right/5V
  = Underworld, applied at RESTART) instead of a hold-while-restarting
  button. No firmware change needed — the active-low shim semantics already
  fit.
- **Cable layout reworked**: board signal wires terminate at vertical
  pull-up columns above each key; GND is a daisy-chained bus along the
  button legs and 5V a bus along the resistor tops — no cables cross the
  buttons. Harness gained net-aware input seeding (union-find over the wire
  graph) so headless verify still passes with the bus topology.
- **Audio timing** (earlier today, recorded here for completeness): Velxio's
  AVR frame loop ran the sim clock ~1.3× wall speed causing seconds of
  accumulating sound lag; fixed by harness patch
  `0001-avr-cycle-accurate-frame-pacing` (measured 1.00× after). The
  headless verify (secret code → WIN) stayed green through all of it.

## 2026-07-13 (later) — Emulation fixes: dead keys, endless beep, layout

Three user-reported bugs in the browser emulation, root-caused in the Velxio
frontend source and fixed on our side (no upstream patches):

- **Keys 1–6 dead / green LED never lit**: Velxio's SPICE→MCU *digital*
  injector only handles numerically-named pins, so `A0`–`A5` stayed at the
  emulator's default LOW (= pressed forever). Keys are now read via
  `analogRead` (the *analog* injector does cover A0–A5) against 10 kΩ
  pull-ups: idle ≈1023, pressed ≈0. `keyTouched()` is the single seam.
- **Continuous beep (survived even Stop)**: the buzzer part's WebAudio
  note-off fires only on a Timer2 duty→0 event; duty is polled only on PWM
  pins (3/5/6/9/10/11) and `noTone()` leaves OCR2A set — on D8 the first
  tone played forever. Emulation buzzer moved to **D11** and all sound goes
  through `emuTone()` (`tone` → `delay` → `noTone` → `OCR2A = 0`). Also
  added a boot guard: the game waits until inputs read idle-high (first
  SPICE solve) instead of spamming notes/restarts. Minimal repro:
  `velxio-multi-board-emulator/circuits/buzzer-test.yaml`.
- **Layout**: keys 1–7 in one horizontal row, pull-ups above their keys,
  panel/LEDs/buzzer in separate blocks (explicit x/y in the spec).

Hardware builds remain untouched (all changes inside `#ifdef
VELXIO_EMULATION`; all PlatformIO envs green). Headless verify still plays
game 2's code and asserts WIN.

## 2026-07-13 — Interactive browser emulation (Velxio)

The game is now playable in a browser with no hardware, via the
[velxio-multi-board-emulator](../velxio-multi-board-emulator/) pipeline
harness. See `emulation/README.md` for how to play and the full rationale.

- **New `emulation/` folder**: `lemon-piano.yaml` (circuit spec: Nano, 7
  clickable "lemon" buttons, panel buttons, feedback LEDs, pump-indicator
  LED, buzzer) and the generated `lemon-piano.vlx` (import into Velxio at
  `http://localhost:3080` and press Run).
- **`VELXIO_EMULATION` input shim in `firmware/src/main.cpp`**: the emulator
  cannot reproduce the analog lemon divider (its parts are digital
  active-low, ADC injection covers channels 0–5 only, and A6 has no digital
  pin), so the emulation build swaps ONLY the input layer — keys become
  active-low buttons on A0–A5 + D9, GAME_SELECT/RESTART become active-low.
  Game logic, audio and outputs untouched. Hardware envs still build the
  identical 7296-byte binary; the shim is compile-checked by the new
  `emulation` env in `platformio.ini` (`-DVELXIO_EMULATION`).
- **Headless gameplay regression test**: `velxio-pipeline run --mode verify`
  compiles the real firmware, boots it under avr8js, injects game 2's secret
  code (`3,6,1,4,2,5,3,6,1,4`) as timed key presses and asserts the serial
  log prints `WIN`, the buzzer pin toggles and the green LED flashes — all
  green (evidence in `emulation/runs/`, gitignored).

## 2026-07-12 — Firmware fixes & refactor (TODO #1–#12)

Applied all correctness, gameplay and code-quality items from `TODO.md`.
Behavior is now *fixed and improved* rather than a 1:1 translation. All three
PlatformIO envs still build green; **static RAM dropped 778 B → 309 B**
(38 % → 15 %) and flash is 7.3 KB (23 %).

- **Correctness:** clear `pressedNote` on victory (no more instant false-fail);
  defined `pumpOff()` boot state instead of `analogWrite(pin, HIGH)`;
  `selectGame()` resets `fails`/`currentStep`/`dead` so a post-death restart is
  clean; removed the bogus `pinMode(0..7, INPUT)` UART clobber; `buzz()` guards
  `frequency <= 0`; deleted the dead `if (game = 1)` block.
- **Gameplay:** LED feedback timed with `millis()` (`LED_FEEDBACK_MS`);
  **edge-triggered input** (`keyHeld[]` rising edge) — a held finger no longer
  advances the sequence repeatedly, and melodies with repeated consecutive
  notes are now playable. Victory/death/spray playback kept intentionally
  blocking.
- **Code quality:** all melodies moved to `PROGMEM`; the `*_cut` victory tunes
  de-duplicated into an offset into the full theme (`MARIO_VICTORY_FROM` /
  `UNDER_VICTORY_FROM`, lengths via `sizeof`); touch threshold auto-calibrated
  at boot (`calibrate()` → `baseline + TOUCH_MARGIN`), replacing the hardcoded
  `SENSITIVITY`.
- The clean game loop also fixes an off-by-one from the original: you now win
  on the 10th correct note instead of needing a phantom 11th press.

Open: TODO #13 (verify relay/pump polarity on the real board) and #14 (redraw
the full schematic) — both need the physical hardware.

## 2026-07-12 — Rescue & workspace setup

- **Rescued the 2019 originals verbatim** into git history (commit
  `rescue: original 2019 lemon piano files`) before touching anything.
- **Translated everything to English**: folder/file names, code comments, and
  identifiers (active firmware and archived references).
- **Restructured to a PlatformIO workspace**: `Piano_Limones_v4/` →
  `firmware/` (`src/main.cpp` + `include/notes.h` + `platformio.ini` with
  `nanoatmega328` / `nanoatmega328new` / `uno` envs); `Banana-Piano-Original/`
  → `archive/banana-piano-original/` (banana-piano, keyboard-test,
  game-prototype); schematics → `docs/`.
- **Reconstructed one corrupted line**: `Duracion` (now `NOTE_DURATION`) had
  stray keystrokes (`= 5çkp\`ñ´sca…0;`) and didn't compile — restored to
  `50` ms. No other behavior changes; 2019 bugs are preserved and catalogued
  in `TODO.md`.
- **Wrote the docs**: README (game rules, secret codes, lineage, quick start),
  `docs/HARDWARE.md` (deduced pin map + wiring + sensing physics), `TODO.md`
  (14-item fix roadmap), `CLAUDE.md`, `.gitignore`.
