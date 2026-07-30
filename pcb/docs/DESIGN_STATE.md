# DESIGN_STATE.md — Lemon Piano V5.5 Board

**Current version: v0.4.0 — full floor-plan remake on a 120 × 40 mm frame:
Nano socket centred on the board, lemon-key header centred on the south
edge, the whole 5 V power-entry filter in the west block folded around a
part-free mini-USB cable corridor, 5 V entry as a 2-pin header, SENS±
buttons in the east block with the pair centred on the y=120 mid-line and
one parallel external-button header each, buzzer in the NE corner.**
(2026-07-30)

> v0.1.0 ERRATUM (ADR-015): the Nano socket rows were 180°-rotated
> (TX1/VIN assumed at the USB end; the real board has D12/D13 there).
> v0.2.0 corrects the maps and swaps the keys header (now SOUTH) and
> LED bar (now NORTH, ascending east→west). Do not fabricate v0.1.0.

## Block status

| Block | State |
|---|---|
| Nano socket (U1/U2, 2×15) | ✅ CENTRED (pin field x 132.22–167.78, rows y 112.38/127.62), USB-west, all 23 used pins netted, 7 NC by design |
| Power-entry filter (J1→D1→D2→C1‖C2→L1→C3‖C4) | ✅ whole chain in the WEST block, folded: VIN section on the south strip, VRAW/+5V on the north strip, C3 at the south-strip east end (ADR-024) |
| 5 V entry (J1) | ✅ 2-pin 2.54 mm header, SW corner, `+ −` silk (ADR-025, supersedes ADR-003) |
| Keyboard (R1–R7 pull-ups + J2 header) | ✅ B.Cu pull-ups under the Nano; header CENTRED on the south edge (pin centre exactly x=150), labelled |
| LED bar (D3–D12 + R8–R17) | ✅ north strip, ascending east→west, centred over the D2–D11 pin span, series R on B.Cu |
| UI (BUZ1 D13, SW1 SENS+ D12, SW2 SENS− A7 + R18) | ✅ buttons in the EAST block, pair centred on y=120; buzzer NE corner (ADR-027) |
| External-button headers (J3 ∥ SW1, J4 ∥ SW2) | ✅ 2-pin headers on the same nets as their button, directly east of it (ADR-026) |
| GND | ✅ B.Cu zone, solid connect on all GND pads, island-healing guard (no healing needed this run) |
| Mounting (H1–H4, M2) | ✅ (95,105) / (205,105) / (95,135) / (205,135), symmetric about x=150 and y=120 |
| Schematic | ✅ mirrors PCB (47 symbols incl. 3 PWR_FLAGs), ERC 0/0 |
| Fab package | ✅ `pcb/releases/v0.4.0/lemon-piano-v0.4.0-fab.zip` (14 files) |

## Verification snapshot (v0.4.0)

| Gate | Result |
|---|---|
| Cloud `/drc` | **0 errors, 0 warnings, 0 unconnected** (`validation/drc-v0.4.0.json`; `included_severities` = error + warning, `schematic_parity` empty) |
| ERC | 0 errors, 0 warnings (`validation/erc-v0.4.0.txt`) |
| `verify_placement` (C1 chirality, C2 flip, C3 pad↔net↔function, C4 net intent) | ALL PASS |
| `verify_holes` (geometric + VISION: affine over 4 detected centres, max LOO 0.0222 mm both sides, 7.236 px/mm) | PASS |
| `geometry_gate` (23 checks: outline, hole symmetry, Nano/keys centring, filter-in-west, button-pair centring, EXT headers east of their buttons, USB corridor, copper per net, courtyards) | ALL PASS |
| Render inspection (top+bottom, 4 styles) | PASS — filter order west→east, non-crossing LED fan (LED1→D2 east, LED10→D11 west), VU colours green→red east→west, overlay Nano lands USB-west with the corridor clear |
| post_route widths | 109 segments at target, 11 short pad-entry stubs capped (all ≥0.2 mm); 1 zone refill, 0 islands to heal |
| Idempotency | build_board ×2 byte-identical |

## Iteration history

| Version | What changed | DRC (err/warn/unconn) | Verdict |
|---|---|---|---|
| v0.0.1 | first placement (no routing) | 0 / 44 / 81 | placement fits; silk collisions; renders verified |
| v0.0.2 | first freerouting pass + naive width post-pass | 12 / 44 / 9 | starved thermals on SMD GND pads; widening broke clearances |
| v0.0.3 | solid zone-connect on all GND pads; clearance-aware widener | 0 / 44 / 0 | electrically clean; silk collisions remain |
| v0.0.4 | silk labels/refs repositioned, SW refs hidden | 0 / 25 / 0 | all remaining warnings = text height < 0.8 |
| v0.0.5 | all silk text ≥ 0.8 mm | 0 / 4 / 0 | title/BUZ1 silk clashes + 1 dangling freerouting spur |
| v0.0.6 | title split, BUZ1 ref inside circle, dangling-spur cleaner | **0 / 0 / 0** | fully clean; all gates pass |
| v0.1.0 | release: schematic+ERC, gates wired into pipeline, island-healing guard, /fab | **0 / 0 / 0** | released, later found ERRATUM ADR-015 — do not fabricate |
| v0.2.0 | REAL Nano orientation (ADR-015): keys header south, LED bar north (E→W); post_route self-healing upgrades (ADR-018) | **0 / 0 / 0** | released + render suite via API v0.3.0 |
| v0.2.1 | 'pcb' version label, per-pin silk legends (both rows), all component refs visible per layer, suite naming normal/dim/realistic/overlay (realistic-dim dropped) | **0 / 0 / 0** | released |
| v0.3.0 | 4 anchor holes (ADR-020), VU-meter LED colors + 3D bodies (ADR-021), /place model-strip bugfix (ADR-022), brighter Nano photo (ADR-023), hole VISION pass on transparent renders (LL §22) | **0 / 0 / 0** | released |
| v0.4.0 | 120 × 40 mm frame + full floor-plan remake (ADR-024): Nano centred, keys header centred, filter folded into the west block around a redefined USB corridor, D2 at rot=180, LED bar centred over its pin span; 5 V entry as a 2-pin header (ADR-025); parallel external-button headers J3/J4 (ADR-026); buzzer NE (ADR-027); title block moved into the corridor (ADR-028) | **0 / 0 / 0** | **RELEASED** — green on the first pipeline run |

(One v0.1.0 route attempt produced a GND-zone island — caught by the
DRC gate, fixed by the automatic island healing, re-run clean. Kept in
`validation/` history.)

## How to regenerate

```bash
# full iteration (build → /place → /route → post → /drc → /render → gates):
./pcb/tools/cloud_pipeline.sh v0.4.0
# release (adds /fab):
./pcb/tools/cloud_pipeline.sh v0.4.0 --fab
```

## Open items

- Bench-validate the filter on the physical board (the V5.5 repo's own
  pending check — switch-flipping session with the V5 bench sampler).
- The 11 capped power/signal stubs (ADR-008) are ≥0.2 mm and current-safe;
  a future placement tweak could open those channels if 100 % nominal
  widths are ever required.
- `/BUZZER` runs ~58 mm because D13 sits at the west end of the south row
  while the buzzer is the only part that fits in the NE corner (ADR-027).
  Harmless for a piezo square wave; revisit only if the buzzer moves.
- The south-east quadrant (x 160–200, y 129–140, ≈39 × 10.5 mm) is empty.
  It is 0.6 mm too short for the buzzer courtyard, so nothing currently
  fits there — free real estate for a future addition.
- C3's Ø8 mm can sits 1.6 mm south of the Nano module edge. Insertion and
  removal are done from the east/west ends (both clear), but a taller or
  wider output reservoir would need the strip re-planned.
