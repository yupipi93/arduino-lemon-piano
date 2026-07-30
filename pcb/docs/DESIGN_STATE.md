# DESIGN_STATE.md — Lemon Piano V5.5 Board

**Current version: v0.5.1 — the Nano is FLIPPED (mini-USB faces EAST), which
puts the analog column on the north row and the digital column on the south
row: lemon-key header on the NORTH edge (centred, KEY1 east), ten-LED bar on
the SOUTH edge (centred on x=150, ascending west→east), power-entry filter
refactored into a compact 3-row west block with C1 ‖ C3 adjacent, and no
USB-cable keepout. v0.5.1 adds D1's missing 3D body (ADR-033).**
(2026-07-30)

> **Flashing note (ADR-030, deliberate):** the mini-USB faces east with no
> cable clearance reserved — the shell reaches x≈173.3 and SW1's courtyard
> starts at 173.48. Flash the Nano before seating it, or lift it out of the
> socket. This is the accepted cost of keys-north + LEDs-south.

> v0.1.0 ERRATUM (ADR-015): the Nano socket rows were 180°-rotated
> (TX1/VIN assumed at the USB end; the real board has D12/D13 there).
> v0.2.0 corrected the maps. Do not fabricate v0.1.0.

## Block status

| Block | State |
|---|---|
| Nano socket (U1/U2, 2×15) | ✅ CENTRED (pin field x 132.22–167.78) and FLIPPED: **USB-east**, U1 = analog NORTH row (pin1 D13 east, rot 270), U2 = digital SOUTH row (pin1 TX1 west, rot 90). All 23 used pins netted, 7 NC by design |
| Power-entry filter (J1→D1→D2→C1‖C2→L1→C3‖C4) | ✅ compact 3-row WEST block (ADR-031): **C1 ‖ C3 adjacent** at y=106, D2→L1 at y=119 (pads 5.5 mm apart), J1→D1 at y=134.5, ceramics on B.Cu at y=112.5 |
| 5 V entry (J1) | ✅ 2-pin 2.54 mm header, SW corner, `+ −` silk (ADR-025) |
| Keyboard (R1–R7 pull-ups + J2 header) | ✅ header CENTRED on the **NORTH** edge (pin centre exactly x=150), pin 1 = KEY1 at the EAST under A0; pull-ups on B.Cu just south of the analog row |
| LED bar (D3–D12 + R8–R17) | ✅ **SOUTH** strip, CENTRED on x=150, ascending west→east (LED1 west under D2, LED10 east under D11), series R on B.Cu |
| UI (BUZ1 D13, SW1 SENS+ D12, SW2 SENS− A7 + R18) | ✅ buttons in the EAST block, pair centred on y=120; buzzer NE corner, now ~20 mm from D13 |
| External-button headers (J3 ∥ SW1, J4 ∥ SW2) | ✅ 2-pin headers on the same nets as their button, directly east of it (ADR-026) |
| GND | ✅ B.Cu zone, solid connect on all GND pads; now refilled after split-net repair too (ADR-032) |
| Mounting (H1–H4, M2) | ✅ (95,105) / (205,105) / (95,135) / (205,135), symmetric about x=150 and y=120 |
| Schematic | ✅ mirrors PCB (47 symbols incl. 3 PWR_FLAGs), ERC 0/0. Unchanged by the flip — the pin→net map is electrical, only placements moved |
| Fab package | ✅ `pcb/releases/v0.5.1/lemon-piano-v0.5.1-fab.zip` (14 files) |

## Verification snapshot (v0.5.1)

| Gate | Result |
|---|---|
| Cloud `/drc` | **0 errors, 0 warnings, 0 unconnected** (`validation/drc-v0.5.1.json`; `included_severities` = error + warning, `schematic_parity` empty) |
| ERC | 0 errors, 0 warnings (`validation/erc-v0.5.1.txt`) |
| `verify_placement` (C1 chirality, C2 flip, C3 pad↔net↔function, C4 net intent) | **73 OK / 0 FAIL** — including the re-derived pad numbers for both flipped 0805 groups |
| `verify_holes` (geometric + VISION) | PASS |
| `geometry_gate` (27 checks: outline, hole symmetry, flip orientation, keys north + KEY1-east, LED bar south + centred + west→east, filter-in-west, C1/C3 adjacency, button-pair centring, EXT headers east, copper per net, courtyards) | ALL PASS |
| Render inspection (top+bottom, 4 styles) | PASS — every part now shows a 3D body, D1 included (ADR-033) |
| post_route widths | 111 segments at target, 7 short pad-entry stubs capped (all ≥0.2 mm, worst-case current ≈200 mA); no split net this run, 1 zone refill |
| Idempotency | build_board byte-identical across runs (`0fc6335c02c7`) |

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
| v0.2.1 | 'pcb' version label, per-pin silk legends (both rows), all component refs visible per layer, suite naming normal/dim/realistic/overlay | **0 / 0 / 0** | released |
| v0.3.0 | 4 anchor holes (ADR-020), VU-meter LED colors + 3D bodies (ADR-021), /place model-strip bugfix (ADR-022), brighter Nano photo (ADR-023), hole VISION pass (LL §22) | **0 / 0 / 0** | released |
| v0.4.0 | 120 × 40 mm frame + floor-plan remake (ADR-024): Nano centred, keys centred south, filter folded around a USB corridor, D2 at rot=180; 2-pin 5 V header (ADR-025); parallel EXT headers J3/J4 (ADR-026); buzzer NE (ADR-027); title in the corridor (ADR-028) | **0 / 0 / 0** | released (superseded by v0.5.0) |
| v0.5.0 (run 1) | Nano FLIPPED to USB-east (ADR-029), keys north, LED bar south + centred, USB keepout dropped (ADR-030), filter refactored with C1 ‖ C3 adjacent (ADR-031) | 2 / 0 / 0 | **FAILED the DRC gate** — two `/+5V` bus bridges laid on filled ground |
| v0.5.0 (run 2) | post_route now refills the GND zone after split-net repair (ADR-032) | **0 / 0 / 0** | released |
| v0.5.1 | D1's 3D body restored: its footprint asks for `..._KathodeUp.step`, upstream ships `..._CathodeUp.step` (ADR-033). Cosmetic only — fab outputs never affected | **0 / 0 / 0** | **RELEASED** |

(One v0.1.0 route attempt produced a GND-zone island — caught by the
DRC gate, fixed by the automatic island healing, re-run clean. Kept in
`validation/` history.)

## How to regenerate

```bash
# full iteration (build → /place → /route → post → /drc → /render → gates):
./pcb/tools/cloud_pipeline.sh v0.5.1
# release (adds /fab):
./pcb/tools/cloud_pipeline.sh v0.5.1 --fab
```

## Open items

- Bench-validate the filter on the physical board (the V5.5 repo's own
  pending check — switch-flipping session with the V5 bench sampler).
- **Flashing ergonomics** (ADR-030): the Nano must come out of its socket to
  take a USB cable. If that becomes annoying in practice, the fix is either
  a right-angle USB adapter or moving the SENS buttons ~4 mm east, which
  would collide with J3/J4 and need the east block re-planned.
- The 7 capped stubs (ADR-008) are ≥0.2 mm and current-safe.
- C1 and C3 sit adjacent by user request, so the unfiltered VRAW and
  filtered +5 V nodes run ~10 mm apart (ADR-031 explains why that is
  acceptable for this filter's job). If the board ever needs real HF
  isolation, reorder the north row to C1—L1—C3.
- The south-east quadrant is now occupied by the title block; the largest
  remaining free area is the strip west of the socket (x 124–130, full
  height) plus the region under the Nano body on both layers.
