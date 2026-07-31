#!/usr/bin/env python3
"""Lemon Piano geometry + routing assertions (mission gate 7d/7b).

Pure-text checks over the routed .kicad_pcb (no pcbnew needed):
  1. outline is exactly 120 x 40 mm at (90,100)-(210,140) (v0.4.0);
  2. exactly 4 mounting holes, mirror-symmetric about x=150 AND y=120
     within 0.1 mm, all at the short-edge extremes;
  3. the v0.5.0 floor plan holds: Nano pin field centred on the board and
     FLIPPED (mini-USB faces EAST, so the analog row is NORTH and the
     digital row SOUTH), key header centred on the NORTH edge with KEY1 at
     the east, LED bar on the SOUTH edge centred on x=150 ascending
     west→east, power entry + whole filter in the WEST block with C1 ‖ C3
     adjacent, SENS buttons in the EAST block with their pair centred on
     y=120, and one parallel external-button header EAST of each button.
     There is deliberately NO USB-cable keepout check (ADR-030);
  4. every net in the YAML has copper (>=1 segment, or GND-zone), and
     every pad's net matches docs/NETLIST.md via the ground-truth file
     (that part is verify_placement's job — here we count copper);
  5. no courtyard overlaps, computed from the YAML pad_half/body_extent
     data (independent re-check of KiCad's courtyard DRC).

Exits non-zero on any failure.
"""
from __future__ import annotations

import math
import re
from pathlib import Path

import yaml

PROJ = Path(__file__).resolve().parents[1]          # arduino-lemon-piano/pcb
CFG = yaml.safe_load((PROJ / "lemon-piano.yaml").read_text(encoding="utf-8"))
PCB = PROJ / "kicad" / CFG["project"]["kicad_pcb_file"]

FAILS: list[str] = []


def check(ok: bool, msg: str) -> None:
    print(f"  [{'PASS' if ok else 'FAIL'}] {msg}")
    if not ok:
        FAILS.append(msg)


def bbox(ref: str, pads_only: bool = False) -> tuple[float, float, float, float]:
    """Global-frame bbox of a footprint from the YAML pad/courtyard data.

    KiCad convention: global = origin + R(rot)·local with +Y down, so
    rot=90 maps local +Y → global +X. B.Cu footprints are mirrored about
    the local Y axis before rotating."""
    pl = CFG["placements"]
    x, y, rot, layer = pl[ref]
    pins = CFG["pin_local_positions"].get(
        ref, [[0, 2.54 * k] for k in range(CFG["pin_counts"][ref])])
    ph = CFG["pad_half"].get(ref, [0.8, 0.8])
    pts = []
    for px, py in pins:
        for sx in (-1, 1):
            for sy in (-1, 1):
                pts.append((px + sx * ph[0], py + sy * ph[1]))
    be = CFG.get("body_extent", {}).get(ref)
    if be and not pads_only:
        ox, oy = be["offset"]
        hx, hy = be["half"]
        for sx in (-1, 1):
            for sy in (-1, 1):
                pts.append((ox + sx * hx, oy + sy * hy))
    out = []
    for px, py in pts:
        if layer == "B.Cu":       # flipped about local Y axis
            px = -px
        r = math.radians(rot)
        gx = x + px * math.cos(r) + py * math.sin(r)
        gy = y - px * math.sin(r) + py * math.cos(r)
        out.append((gx, gy))
    xs = [p[0] for p in out]
    ys = [p[1] for p in out]
    return min(xs), min(ys), max(xs), max(ys)


def main() -> int:
    text = PCB.read_text(encoding="utf-8")
    geom = CFG["geometry"]["pcb"]

    # 1. outline ---------------------------------------------------------
    m = re.search(r'\(gr_rect\s*\(start ([\d.]+) ([\d.]+)\)\s*\(end ([\d.]+) ([\d.]+)\)'
                  r'[\s\S]*?\(layer "Edge.Cuts"\)', text)
    check(m is not None, "Edge.Cuts rectangle present")
    if m:
        x0, y0, x1, y1 = map(float, m.groups())
        check((x0, y0, x1, y1) == (geom["x0"], geom["y0"], geom["x1"], geom["y1"]),
              f"outline {x1 - x0:.1f} x {y1 - y0:.1f} mm at ({x0},{y0})")
        check(abs((x1 - x0) - 120.0) < 1e-6 and abs((y1 - y0) - 40.0) < 1e-6,
              "outline is exactly 120 x 40 mm")

    # 2. mounting holes: TWO per short edge -------------------------------
    holes = {}
    for ref in ("H1", "H2", "H3", "H4"):
        mm_ = re.search(rf'\(property "Reference" "{ref}"', text)
        blk_start = text.rfind("(footprint", 0, mm_.start())
        at = re.search(r'\(at ([\d.\-]+) ([\d.\-]+)', text[blk_start:mm_.start() + 500])
        holes[ref] = (float(at.group(1)), float(at.group(2)))
    n_h = len(re.findall(r'\(property "Reference" "H\d+"', text))
    check(n_h == 4, f"exactly 4 mounting holes (found {n_h})")
    cx = (geom["x0"] + geom["x1"]) / 2
    cy = (geom["y0"] + geom["y1"]) / 2
    for a, b in (("H1", "H2"), ("H3", "H4")):
        (ax, ay), (bx, by) = holes[a], holes[b]
        check(abs((cx - ax) - (bx - cx)) <= 0.1 and abs(ay - by) <= 0.1,
              f"{a}/{b} mirror-symmetric about x={cx}")
    for a, b in (("H1", "H3"), ("H2", "H4")):
        (ax, ay), (bx, by) = holes[a], holes[b]
        check(abs((cy - ay) - (by - cy)) <= 0.1 and abs(ax - bx) <= 0.1,
              f"{a}/{b} mirror-symmetric about y={cy}")
    check(all(h[0] - geom["x0"] <= 10 or geom["x1"] - h[0] <= 10
              for h in holes.values()),
          "holes at the short-edge extremes (<=10 mm from edge)")

    # 3. v0.5.0 floor plan ------------------------------------------------
    # The Nano is FLIPPED versus v0.4.0 (ADR-029): mini-USB faces EAST, so
    # the analog column is the NORTH row (U1) and the digital column the
    # SOUTH row (U2). That is what allows keys-north / LEDs-south. There is
    # no USB-corridor keepout any more (ADR-030, user spec).
    pl = CFG["placements"]
    cx = (geom["x0"] + geom["x1"]) / 2
    cy = (geom["y0"] + geom["y1"]) / 2
    field_w = 2.54 * 14
    # U1 pin1 (D13) at the EAST end, U2 pin1 (TX1) at the WEST end
    check(abs(pl["U1"][0] - (cx + field_w / 2)) < 0.01
          and abs(pl["U2"][0] - (cx - field_w / 2)) < 0.01,
          f"Nano pin field centred on x={cx}, FLIPPED (USB faces east)")
    check(pl["U1"][2] == 270 and pl["U2"][2] == 90,
          "Nano rows rotated for the flip (U1 rot=270, U2 rot=90)")
    check(pl["U1"][1] < cy < pl["U2"][1]
          and abs((pl["U1"][1] - cy) + (pl["U2"][1] - cy)) < 0.01,
          f"analog row NORTH / digital row SOUTH, symmetric about y={cy}")
    # keys header: CENTERED on the NORTH edge, pin 1 (KEY1) at the EAST so
    # KEY1..KEY7 land under A0..A6 without crossings
    keys_centre = pl["J2"][0] - 2.54 * 7 / 2
    check(abs(keys_centre - cx) < 0.01,
          f"keys header centred on x={cx} (pin centre {keys_centre})")
    check(pl["J2"][1] <= geom["y0"] + 5.0, "keys header on the NORTH edge")
    check(pl["J2"][2] == 270, "keys header pin 1 (KEY1) at the EAST end")
    # LED bar: SOUTH edge, CENTERED on cx, ascending west→east
    check(all(pl[f"D{i}"][1] >= geom["y1"] - 2.5 for i in range(3, 13)),
          "LED bar on the SOUTH service edge")
    bar_centre = (pl["D3"][0] + pl["D12"][0]) / 2
    check(abs(bar_centre - cx) < 0.01,
          f"LED bar centred on x={cx} (got {bar_centre})")
    check(pl["D3"][0] < pl["D12"][0],
          "LED bar ascends west→east (LED1 west, matching D2..D11)")
    # power entry + the WHOLE filter live in the WEST block (user spec)
    filt = ["J1", "D1", "D2", "C1", "C2", "C3", "C4", "L1"]
    east_of_west = [r for r in filt if pl[r][0] > cx - 15.0]
    check(not east_of_west,
          f"power entry + filter in the WEST block {east_of_west or ''}")
    # the two bulk caps sit together (user spec v0.5.0)
    c1b, c3b = bbox("C1"), bbox("C3")
    cap_gap = max(c1b[0] - c3b[2], c3b[0] - c1b[2])
    check(0 <= cap_gap < 3.0 and abs(pl["C1"][1] - pl["C3"][1]) < 0.01,
          f"C1 and C3 adjacent on one row (gap {cap_gap:.2f} mm)")
    # SENS buttons: EAST block, the PAIR centred on the board mid-line
    check(pl["SW1"][0] > cx + 20.0 and pl["SW2"][0] > cx + 20.0,
          "SENS buttons in the EAST block")
    pair_cy = ((pl["SW1"][1] - 1.52) + (pl["SW2"][1] + 6.04)) / 2
    check(abs(pair_cy - cy) < 0.01,
          f"SENS button pair centred on y={cy} (got {pair_cy})")
    # one parallel external-button header EAST of each button (ADR-026)
    for sw, hdr in (("SW1", "J3"), ("SW2", "J4")):
        check(pl[hdr][0] > pl[sw][0] + 8.04,
              f"{hdr} (external {sw} button) sits east of {sw}")
    # aux speaker header sits next to the buzzer it parallels (ADR-034)
    jb, bb = bbox("J5"), bbox("BUZ1")
    gap = max(jb[0] - bb[2], bb[0] - jb[2], jb[1] - bb[3], bb[1] - jb[3])
    check(0 < gap < 8.0, f"J5 (aux speaker out) adjacent to BUZ1 "
                         f"(gap {gap:.2f} mm)")

    # 4. copper per net ---------------------------------------------------
    net_numbers = CFG["nets"]["numbers"]
    seg_nets = [int(n) for n in re.findall(r'\(segment[\s\S]*?\(net (\d+)\)', text)]
    from collections import Counter
    seg_count = Counter(seg_nets)
    zone_gnd = bool(re.search(r'\(zone\s*\(net 2\)[\s\S]*?\(filled_polygon', text))
    missing = []
    for name, num in net_numbers.items():
        if name == "GND":
            if not (zone_gnd or seg_count.get(num)):
                missing.append(name)
        elif not seg_count.get(num):
            missing.append(name)
    check(zone_gnd, "GND zone present and filled on B.Cu")
    check(not missing, f"every net has copper ({len(net_numbers)} nets; "
                       f"missing: {missing or 'none'})")

    # 5. courtyard overlaps from YAML data --------------------------------
    th = set(CFG["th_footprints"])
    refs = list(pl)
    overlaps = []
    for i in range(len(refs)):
        for j in range(i + 1, len(refs)):
            a, b = refs[i], refs[j]
            la, lb = pl[a][3], pl[b][3]
            if la == lb:
                # same side: full courtyard vs full courtyard
                ba, bb = bbox(a), bbox(b)
            elif a in th or b in th:
                # THT vs opposite-side SMD: the THT part only occupies the
                # far side with its annuli — compare pad bbox vs full bbox
                ba = bbox(a, pads_only=a in th)
                bb = bbox(b, pads_only=b in th)
            else:
                continue              # SMD on opposite sides: no conflict
            if ba[0] < bb[2] and bb[0] < ba[2] and ba[1] < bb[3] and bb[1] < ba[3]:
                overlaps.append((a, b))
    check(not overlaps, f"no courtyard overlaps from YAML extents {overlaps or ''}")

    print()
    if FAILS:
        print(f"GEOMETRY GATE: {len(FAILS)} FAILURE(S)")
        return 1
    print("GEOMETRY GATE: ALL PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
