# AGENT_PROMPT.md — mission brief for implementing a new Lemon Piano PCB version

Copy everything between the `=== PROMPT ===` markers into a fresh agent
session, fill in the **USER CHANGE REQUEST** block, and let it run. The
prompt encodes the working protocol and every hard-won lesson from
versions v0.0.1 → v0.5.0 of this board.

=== PROMPT ===

# MISSION: Implement a new version of the Lemon Piano V5.5 PCB

You are working autonomously on an EXISTING, verified, released PCB
project. Do not redesign anything the request does not touch. When a
decision is needed, make the engineering-sound choice, record it as a
new ADR in `pcb/docs/DECISIONS.md` (continue the numbering), and keep
going. Stop only for a hard blocker you cannot resolve.

## USER CHANGE REQUEST

> (paste the user's indications here, verbatim)

## What this board is

A 120 × 40 mm 2-layer KiCad-9 board (frame x 90..210, y 100..140, centre
150/120) that replaces the arduino-lemon-piano V5.5 breadboard: socketed
Arduino Nano **centred on the board and FLIPPED so the mini-USB faces EAST**
(2×15 rows, pin field x 132.22..167.78, rows y 112.38/127.62), 7 lemon-key
lines + GND clip on a labelled **north**-edge header **centred** on x=150
with pin 1 (KEY1) at the EAST, ten-LED VU-meter bar (3 green / 3 yellow /
2 orange / 2 red) on the **south** edge **centred** on x=150 ascending
west→east, SENS± buttons in the east block with the **pair centred on
y=120** plus one parallel 2-pin external-button header each (J3/J4), D13
buzzer in the NE corner, and the V5.5 power-entry filter (2-pin 5 V header →
P6KE6.8A TVS → 1N5817 → 470 µF‖100 nF → 100 µH → 470 µF‖100 nF) feeding the
+5 V rail — the whole filter lives in the WEST block as a compact 3-row
group with **C1 ‖ C3 adjacent** on its north row. Four M2 anchor holes, two
per short edge (x=95/205, y=105/135).

**Orientation is the highest-leverage fact on this board.** The Nano's own
pinout is fixed (ADR-015: D12/D13 flank the USB, TX1/VIN at the ICSP end),
so which board edge carries the analogs vs the digitals is decided ONLY by
the module's rotation. Since v0.5.0 (ADR-029) the USB faces EAST, which
means: analogs on the NORTH row descending west→east (A0 at 160.16 … A6 at
144.92) → keys header north with KEY1 EAST; digitals on the SOUTH row
ascending west→east → LED bar south ascending west→east. If a request asks
to move the keys header or the LED bar to the other edge, it is really
asking to flip the Nano — say so, and flip BOTH the socket placements and
every dependent map in one change.

**There is NO USB-cable keepout** (ADR-030, explicit user decision). The USB
shell reaches ~x=173.3 and SW1's courtyard starts at 173.48, so a plugged
cable fouls the SENS buttons: flashing means lifting the Nano out of its
socket. Do not "fix" this by reserving space unless the user asks.

## Where everything lives (two sibling repos, fixed protocol)

- `arduino-lemon-piano/pcb/` — THE BOARD. Config (`lemon-piano.yaml`),
  netlist ground truth (`docs/NETLIST.md`), decisions
  (`docs/DECISIONS.md`), state (`docs/DESIGN_STATE.md`), tools
  (`tools/`), verify ground truth (`ground-truth/`), KiCad artefacts
  (`kicad/`), renders/validation/releases archives, overlay assets
  (`overlays/`).
- `../eda-pcb-designer/` — the TOOLKIT (sibling repo). Board tools
  import it via `sys.path → ../eda-pcb-designer/src` and run inside its
  Docker image `eda-pcb-designer:latest`. Its `projects/mt1/` is a
  reference example — read-only.
- Hosted API: `https://pcb-designer.scv.multitecua.com` (`GET /health`,
  `GET /openapi.json`). Endpoints: `/place`, `/route`, `/drc`,
  `/render?style=bare|dim|realistic|realistic-dim|overlay`, `/fab`.
  Cloud Run service `pcb-designer`, project `sergio-conejero`, region
  `europe-west1` (gcloud account: sergio.conejero@lutech-sweeft.es).

## Mandatory reading, in order (before touching anything)

1. `../eda-pcb-designer/AGENTS.md` and
   `../eda-pcb-designer/docs/LESSONS_LEARNED.md` (§1–§26 — load-bearing;
   §25 `pad.GetLayer()` lies on flipped footprints, §26 overlay rotation
   convention).
2. `pcb/docs/NETLIST.md` — the circuit truth. NEVER invent values, pins
   or parts; the upstream sources are `versions/v5.5-power-filter/` and
   `versions/v5-led-bar/HARDWARE.md` plus the DRC-validated
   `build_v5_5()` builder in `tools/wiring_diagrams.py`.
3. `pcb/docs/DECISIONS.md` — every ADR. Do not contradict one silently.
4. `pcb/docs/DESIGN_STATE.md` — current version, iteration history.
5. `pcb/lemon-piano.yaml` + `pcb/tools/build_board.py` — the single
   sources of the layout (YAML: placements/nets/geometry; build_board:
   pad↔net tables, silk, legends, 3D-model overrides).

## Version bump rule (repo convention)

- Physical change (footprints, holes, outline, placements, netlist) →
  bump MINOR of the pcb version: v0.5.0 → v0.6.0.
- Cosmetic-only change (silk, labels, renders) → bump PATCH:
  v0.5.0 → v0.5.1.
- Set it in `pcb/lemon-piano.yaml` (`project.version`) — the silk label
  "pcb vX.Y.Z" and every artefact name derive from it. Never regenerate
  a released version tag with different content; if a released version
  turns out defective, rename its release folder
  `vX.Y.Z-ERRATUM-do-not-fab` and document why.

## The workflow for a change

1. **Map the request to files.** Typical cases:
   - Move/add/remove a component → `lemon-piano.yaml` (placements,
     pin_counts, pad_half, body_extent, pin_local_positions,
     th_footprints, nets.numbers if nets change) + `build_board.py`
     (FOOTPRINTS, PAD_NETS or GEOMETRIC_NETS, EXPECTED_PADS assertions,
     REF_POS, silk) + `docs/NETLIST.md` + `ground-truth/components.yaml`
     + `tools/build_schematic.py` + geometry_gate checks if geometry
     rules change.
   - Silk/label change → `build_board.py` only (+ docs).
   - Hole/outline change → YAML geometry + build_board + BOTH
     ground-truth files + `geometry_gate.py`.
   - Render/pipeline change → `tools/cloud_pipeline.sh`.
   Footprints must exist in the KiCad 9 libs inside the Docker image
   (`/usr/share/kicad/footprints`) — verify with pcbnew before using,
   and dump real courtyards for floor-planning (never guess extents).
2. **Floor-plan against REAL courtyard boxes** (pcbnew dump), keeping:
   keys header NORTH under A0..A6 and centred (KEY1 east); LED bar SOUTH
   and centred, ascending west→east; the filter in the west block with
   C1 ‖ C3 adjacent; the SENS button pair centred on y=120 with its EXT
   header east of each button; anchor zones (x<100, x>200) component-free;
   silk text ≥ 0.8 mm height everywhere. Before running the pipeline, check
   the plan numerically — the standalone bbox math in `tools/geometry_gate.py`
   (`bbox()` is module-level, importable) will list overlaps and gaps for a
   candidate YAML in seconds, which is far cheaper than a routed iteration.
   **If you move either 0805 resistor group, its pad NUMBERS may invert**:
   the builder nets them GEOMETRICALLY (north/south by position), so build
   first, read the pad→net pairs back off the `.kicad_pcb`, and only then
   fix `ground-truth/components.yaml`. Both groups inverted in v0.5.0.
3. **Run the pipeline** (from `arduino-lemon-piano/`):
   `./pcb/tools/cloud_pipeline.sh vX.Y.Z` — it does: build_board
   (Docker, byte-stable) → cloud /place → cloud /route → post_route
   (Docker: clearance-aware widening, zone-island healing, split-net
   repair) → cloud /drc [HARD GATE: 0 errors AND 0 unconnected AND —
   your goal — 0 warnings] → render suite
   (`renders/<ver>-{normal,dim,realistic,overlay}-{top,bottom}.png`) →
   schematic + ERC → verify_placement + verify_holes(+vision) +
   geometry_gate [HARD GATES]. Add `--fab` only on the final, fully
   green run (writes `releases/<ver>/`).
4. **Iterate until everything is green AND you have LOOKED at the
   renders** (you have vision — DRC cannot see "wrong"). Check: pin
   legends match the REAL Nano (D12/D13 at the USB end, TX1/VIN at the
   ICSP end — v0.1.0 shipped 180° wrong, see ADR-015), LED color order,
   labels, the overlay photo sitting on the socket field **USB-east**
   (since ADR-029 — the photo crop is natively USB-west, so the engine must
   be rotating it 180°; if it looks USB-west, the overlay data is stale).
5. **Docs in the same change**: DESIGN_STATE (state + iteration-log
   row), DECISIONS (new ADRs), NETLIST if the circuit view changed,
   README if user-facing facts changed, CHANGELOG.md (repo root,
   append-only, newest first), regenerate `renders/INDEX.md` with
   `../eda-pcb-designer/.venv/bin/pcb-designer gallery pcb/renders`.
6. **Commit + push** `arduino-lemon-piano` main (this repo pushes on
   every change — Conventional Commits, co-author line
   `Co-Authored-By: <your model name> <noreply@anthropic.com>`). Toolkit
   changes (eda-pcb-designer) ONLY for real bugs: own commit, ruff +
   full pytest green, push; if the change affects the API service,
   rebuild + redeploy:
   `gcloud builds submit --config deploy/cloudbuild.yaml
   --substitutions _IMAGE=europe-west1-docker.pkg.dev/sergio-conejero/pcb-designer/pcb-designer:<sha>
   --project sergio-conejero .` then
   `gcloud run deploy pcb-designer --project sergio-conejero --region
   europe-west1 --image <that image> --memory 4Gi`, and verify
   `/health`.

## Hard-won rules — violating these cost real iterations

- **Docker runs as your uid** with the COMMON PARENT mounted:
  `docker run --rm --user "$(id -u):$(id -g)" -e HOME=/tmp -v
  "$PARENT":/work -w /work --entrypoint python3 eda-pcb-designer:latest
  arduino-lemon-piano/pcb/tools/<tool>.py` (the pipeline script does
  this for you).
- **Idempotency is law**: build_board and build_schematic must be
  byte-stable across re-runs (deterministic UUID rewrite + canonical
  block ordering already implemented — keep pads in library order, the
  hole verifier reads the first pad). Run twice and diff when you touch
  them.
- **Rotation semantics** (verified empirically): KiCad footprint rot=90
  maps local +Y → global +X (pin rows run EAST from pin 1); rot=270 runs
  WEST. `build_board` asserts pad positions (EXPECTED_PADS) — extend the
  assertions for anything you move; they catch convention surprises.
- **Flipped footprints**: `pad.GetLayer()` LIES (returns F_Cu) — always
  `pad.IsOnLayer(...)`. Net assignment for the vertical 0805s is
  GEOMETRIC (north/south pad by position) — after moving them, re-check
  which pad number lands where and fix `ground-truth/components.yaml`
  accordingly (the verifier compares pad NUMBERS).
- **Freerouting is stochastic** and always routes at 0.2 mm (netclasses
  never reach the stateless API): post_route widens clearance-aware
  afterwards. If a run leaves unroutable junk, just re-run the pipeline
  before debugging. post_route already self-heals zone islands (pinch
  stitch → pad bridge → via bridge, chaining through bridged islands)
  and repairs split nets; if it aborts, read its message — do not
  weaken the DRC gate.
- **Never regenerate the base board over a routed one.** `build_board.py`
  emits the BASE board (no tracks, unfilled zone), so running it standalone
  — or the pipeline with `--skip-route` — silently leaves you with bare
  copper: the file still opens in KiCad and still passes verify_placement,
  so the loss is easy to miss until DRC reports every net unrouted. Since
  v0.5.0 `guard_routed_board()` **refuses** that write unless
  `LEMON_PCB_REBUILD=1` is set (`cloud_pipeline.sh` sets it, because /route
  re-applies the routing straight after). If you hit the guard, use the
  pipeline. Cost of learning this: one wasted route cycle on v0.5.0, plus a
  rebuilt fab package so the release matched the committed board.
- **Do not edit `cloud_pipeline.sh` while it is running** — bash reads
  scripts lazily, so changing the file shifts the offsets it has yet to read
  and it can execute a partial line. Wait for the run to finish.
- **Overlay photo rotation**: the engine composes
  `PIL_rot = −pcb_rotation + image_rotation` (MT1-calibrated; do NOT
  "fix" the sign in code). Calibrate `overlays/modules.yaml`
  `image_rotation_deg` by generating the overlay and LOOKING at it.
  Current photo: `File:Arduino nano.jpg` crop, 57.95 px/mm,
  image_rotation 90. `calibration=green_bbox` stays until the vision
  affine gets ≥4 holes visible in the overlay flow.
- **Hole vision verification** uses the transparent-background renders
  the pipeline fetches into `validation/vision-<ver>-*.png`
  (LESSONS_LEARNED §22 — opaque backgrounds bias the bore centroid
  ~0.4 mm). Geometric hole check is the hard gate; a COMPLETED vision
  pass that fails is also fatal.
- **Silk discipline**: all text ≥ 0.8 mm high (DRC warning otherwise);
  target ZERO DRC warnings, not just zero errors — every silk collision
  so far was fixable by nudging. LED refs live on B.SilkS (no front room
  at 4.6 mm pitch). Pin legends (h 0.8 / w 0.65, rot 90) must track any
  socket remap.
- **3D bodies**: colored LED models come from kicad-packages3d
  (`LED_D3.0mm_Green/_Yellow`, base = red) plus the service-shipped
  `LED_D3.0mm_Orange.step` (`eda-pcb-designer/assets/3dmodels/`). New
  custom models must be added there + image rebuilt + redeployed, or
  they will not resolve on the render host.

## Definition of done

- `/drc`: 0 errors, 0 warnings, 0 unconnected on the final board.
- ERC: 0/0. verify_placement (C1–C4), verify_holes (geometric + vision),
  geometry_gate: ALL PASS.
- Renders of BOTH sides in all four styles inspected by eye and
  consistent with the request.
- `--fab` release zip archived under `pcb/releases/<ver>/`.
- Docs + CHANGELOG updated, `renders/INDEX.md` regenerated, everything
  committed and pushed (both repos if the toolkit changed).
- Final report: what changed, iteration log (version → change → DRC →
  verdict), every autonomous decision with its ADR number, anything
  left imperfect stated plainly.

=== PROMPT ===
