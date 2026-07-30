#!/usr/bin/env python3
"""Post-route pass for the Lemon Piano board (runs in the Docker image).

The cloud `/route` endpoint routes every net at the stateless default
0.2 mm (netclasses live in the project file, which the API never sees).
This pass enforces the YAML routing widths afterwards, WITHOUT creating
clearance violations (DECISIONS.md ADR-008):

  1. drop freerouting's sub-0.1 mm junk segments (LESSONS_LEARNED §2);
  2. clearance-aware widening: each segment is widened to its target
     (power nets → `trace_width_power`, others → `trace_width_signal`)
     but capped so that 0.2 mm copper clearance to every other-net pad,
     track and via — and 0.3 mm to the board edge — is preserved. The
     cap is computed from the freerouting CENTERLINES and the *target*
     widths of neighbours (one-shot, order-independent → idempotent).
     Floor = 0.2 mm (CONVENTIONS §7 minimum, always DRC-clean).
  3. re-run the ZONE_FILLER so the B.Cu GND fill honours the new widths
     (fills are stored in the file; /drc does not refill).

Usage:
    docker run --rm --user "$(id -u):$(id -g)" -e HOME=/tmp \
        -v "$PWD":/work -w /work --entrypoint python3 \
        eda-pcb-designer:latest arduino-lemon-piano/pcb/tools/post_route.py <pcb>
"""
from __future__ import annotations

import math
import sys
from pathlib import Path

import pcbnew
import yaml

PROJ = Path(__file__).resolve().parents[1]          # arduino-lemon-piano/pcb
TOOLKIT = Path(__file__).resolve().parents[3] / "eda-pcb-designer"
sys.path.insert(0, str(TOOLKIT / "src"))


CFG_PATH = PROJ / "lemon-piano.yaml"
POWER_NETS = {"/+5V", "/GND", "/VIN", "/VRAW"}
COPPER_CLEARANCE = 0.2   # mm, CONVENTIONS §7
EDGE_CLEARANCE = 0.3     # mm, CONVENTIONS §7


def seg_point_dist(ax, ay, bx, by, px, py) -> float:
    """Distance from point P to segment AB (mm)."""
    abx, aby = bx - ax, by - ay
    l2 = abx * abx + aby * aby
    if l2 == 0:
        return math.hypot(px - ax, py - ay)
    t = max(0.0, min(1.0, ((px - ax) * abx + (py - ay) * aby) / l2))
    return math.hypot(px - (ax + t * abx), py - (ay + t * aby))


def seg_rect_dist(ax, ay, bx, by, rx0, ry0, rx1, ry1) -> float:
    """EXACT distance from segment AB to an axis-aligned rect (0 if
    intersecting). A sampled approximation here once under-measured by
    ~0.02 mm and let the widener create a real DRC clearance violation."""
    # endpoint inside rect → intersecting
    for px, py in ((ax, ay), (bx, by)):
        if rx0 <= px <= rx1 and ry0 <= py <= ry1:
            return 0.0
    edges = ((rx0, ry0, rx1, ry0), (rx1, ry0, rx1, ry1),
             (rx1, ry1, rx0, ry1), (rx0, ry1, rx0, ry0))
    return min(seg_seg_dist((ax, ay, bx, by), e) for e in edges)


def seg_seg_dist(a, b) -> float:
    """Distance between two segments given as (x1,y1,x2,y2)."""
    ax, ay, bx, by = a
    cx, cy, dx, dy = b
    if _segs_intersect(ax, ay, bx, by, cx, cy, dx, dy):
        return 0.0
    return min(seg_point_dist(ax, ay, bx, by, cx, cy),
               seg_point_dist(ax, ay, bx, by, dx, dy),
               seg_point_dist(cx, cy, dx, dy, ax, ay),
               seg_point_dist(cx, cy, dx, dy, bx, by))


def _segs_intersect(ax, ay, bx, by, cx, cy, dx, dy) -> bool:
    def ccw(x1, y1, x2, y2, x3, y3):
        return (y3 - y1) * (x2 - x1) - (y2 - y1) * (x3 - x1)
    d1 = ccw(cx, cy, dx, dy, ax, ay)
    d2 = ccw(cx, cy, dx, dy, bx, by)
    d3 = ccw(ax, ay, bx, by, cx, cy)
    d4 = ccw(ax, ay, bx, by, dx, dy)
    return ((d1 > 0) != (d2 > 0)) and ((d3 > 0) != (d4 > 0))


def main() -> None:
    pcb = Path(sys.argv[1] if len(sys.argv) > 1
               else PROJ / "kicad/lemon-piano.kicad_pcb")
    cfg = yaml.safe_load(CFG_PATH.read_text(encoding="utf-8"))
    w_sig = float(cfg["routing"]["trace_width_signal"])
    w_pow = float(cfg["routing"]["trace_width_power"])
    geom = cfg["geometry"]["pcb"]

    board = pcbnew.LoadBoard(str(pcb))

    # tiny-segment cleanup (LESSONS_LEARNED §2), connectivity-safe version:
    # a blanket text pass once removed a 0.05 mm JOG that was the only link
    # between two /+5V tracks and split the net. Only remove a tiny segment
    # when it sits entirely inside a same-net via barrel (the actual §2
    # freerouting artifact) — anything else stays.
    all_vias = [t for t in board.GetTracks() if t.GetClass() == "PCB_VIA"]
    n_tiny = 0
    for t in list(board.GetTracks()):
        if t.GetClass() != "PCB_TRACK":
            continue
        s, e = t.GetStart(), t.GetEnd()
        if math.hypot((e.x - s.x) / 1e6, (e.y - s.y) / 1e6) > 0.1:
            continue
        for v in all_vias:
            if v.GetNetname() != t.GetNetname():
                continue
            vp = v.GetPosition()
            r = v.GetDrillValue() / 2e6 + 0.15
            if (math.hypot((s.x - vp.x) / 1e6, (s.y - vp.y) / 1e6) <= r
                    and math.hypot((e.x - vp.x) / 1e6, (e.y - vp.y) / 1e6) <= r):
                board.RemoveNative(t)
                n_tiny += 1
                break
    if n_tiny:
        print(f"  removed {n_tiny} tiny in-via segment(s)")
        pcbnew.SaveBoard(str(pcb), board)
        board = pcbnew.LoadBoard(str(pcb))

    # dangling-spur cleanup (non-GND nets; GND legitimately ends in the zone).
    # A segment end is "connected" if it lands on a same-net pad, via, or
    # another same-net track. Iterate to a fixed point.
    removed = 0
    while True:
        tracks = [t for t in board.GetTracks() if t.GetClass() == "PCB_TRACK"]
        all_vias = [t for t in board.GetTracks() if t.GetClass() == "PCB_VIA"]
        pad_list = []
        for fp in board.GetFootprints():
            for pad in fp.Pads():
                bb = pad.GetBoundingBox()
                pad_list.append((pad.GetNetname(),
                                 (bb.GetLeft() / 1e6 - 0.01, bb.GetTop() / 1e6 - 0.01,
                                  bb.GetRight() / 1e6 + 0.01, bb.GetBottom() / 1e6 + 0.01)))

        def end_connected(t, px, py):
            net = t.GetNetname()
            for pnet, (x0, y0, x1, y1) in pad_list:
                if pnet == net and x0 <= px <= x1 and y0 <= py <= y1:
                    return True
            for v in all_vias:
                if v.GetNetname() != net:
                    continue
                vp = v.GetPosition()
                if math.hypot(vp.x / 1e6 - px, vp.y / 1e6 - py) <= v.GetWidth() / 2e6 + 0.01:
                    return True
            for o in tracks:
                if o is t or o.GetNetname() != net or o.GetLayer() != t.GetLayer():
                    continue
                s, e = o.GetStart(), o.GetEnd()
                if seg_point_dist(s.x / 1e6, s.y / 1e6, e.x / 1e6, e.y / 1e6,
                                  px, py) <= o.GetWidth() / 2e6 + 0.01:
                    return True
            return False

        dangling = []
        for t in tracks:
            if t.GetNetname() == "/GND":
                continue
            s, e = t.GetStart(), t.GetEnd()
            if (not end_connected(t, s.x / 1e6, s.y / 1e6)
                    or not end_connected(t, e.x / 1e6, e.y / 1e6)):
                dangling.append(t)
        if not dangling:
            break
        for t in dangling:
            print(f"  removed dangling {t.GetNetname()} spur "
                  f"({t.GetStart().x / 1e6:.2f},{t.GetStart().y / 1e6:.2f})")
            board.RemoveNative(t)
            removed += 1
    if removed:
        pcbnew.SaveBoard(str(pcb), board)
        board = pcbnew.LoadBoard(str(pcb))

    def target(netname: str) -> float:
        return w_pow if netname in POWER_NETS else w_sig

    # obstacle inventories --------------------------------------------------
    pads = []                                # (layers, net, rect)
    for fp in board.GetFootprints():
        for pad in fp.Pads():
            bb = pad.GetBoundingBox()
            rect = (bb.GetLeft() / 1e6, bb.GetTop() / 1e6,
                    bb.GetRight() / 1e6, bb.GetBottom() / 1e6)
            # NOTE: pad.GetLayer() lies for flipped footprints (returns
            # F_Cu for pads living on B.Cu) — IsOnLayer() is authoritative.
            layers = {ly for ly in (pcbnew.F_Cu, pcbnew.B_Cu)
                      if pad.IsOnLayer(ly)}
            pads.append((layers, pad.GetNetname(), rect))

    def via_width(v) -> float:
        try:
            return v.GetWidth(pcbnew.B_Cu) / 1e6
        except TypeError:
            return v.GetWidth() / 1e6

    segs, vias = [], []
    for t in board.GetTracks():
        cls = t.GetClass()
        s, e = t.GetStart(), t.GetEnd()
        if cls == "PCB_TRACK":
            segs.append((t, t.GetLayer(), t.GetNetname(),
                         (s.x / 1e6, s.y / 1e6, e.x / 1e6, e.y / 1e6)))
        elif cls == "PCB_VIA":
            vias.append((t.GetNetname(), s.x / 1e6, s.y / 1e6, via_width(t)))

    # one-shot width solve --------------------------------------------------
    n_full = n_capped = 0
    for t, layer, net, line in segs:
        tw = target(net)
        cap = tw
        ax, ay, bx, by = line
        # board edge
        edge_d = min(ax - geom["x0"], geom["x1"] - ax, ay - geom["y0"],
                     geom["y1"] - ay, bx - geom["x0"], geom["x1"] - bx,
                     by - geom["y0"], geom["y1"] - by)
        cap = min(cap, 2 * (edge_d - EDGE_CLEARANCE))
        # other-net pads on this layer
        for layers, pnet, rect in pads:
            if pnet == net or layer not in layers:
                continue
            d = seg_rect_dist(ax, ay, bx, by, *rect)
            cap = min(cap, 2 * (d - COPPER_CLEARANCE))
        # other-net vias
        for vnet, vx, vy, vw in vias:
            if vnet == net:
                continue
            d = seg_point_dist(ax, ay, bx, by, vx, vy) - vw / 2
            cap = min(cap, 2 * (d - COPPER_CLEARANCE))
        # other-net tracks on this layer (both at their targets)
        for t2, layer2, net2, line2 in segs:
            if net2 == net or layer2 != layer:
                continue
            d = seg_seg_dist(line, line2)
            cap = min(cap, 2 * (d - COPPER_CLEARANCE) - target(net2))
        w = max(0.2, min(tw, math.floor(cap * 1000) / 1000))
        t.SetWidth(int(round(w * 1e6)))
        if w >= tw - 1e-9:
            n_full += 1
        else:
            n_capped += 1
            print(f"  capped {net} segment at ({ax:.2f},{ay:.2f}) "
                  f"to {w:.3f} mm (target {tw})")
    print(f"  widened {n_full} segment(s) to target, {n_capped} capped")

    filler = pcbnew.ZONE_FILLER(board)
    zones = list(board.Zones())
    filler.Fill(zones)
    print(f"  refilled {len(zones)} zone(s)")

    repair_split_nets(board, target)
    heal_zone_islands(board, filler, zones)

    pcbnew.SaveBoard(str(pcb), board)
    print(f"  saved {pcb}")


def repair_split_nets(board, target) -> None:
    """Freerouting occasionally omits a trivial link (observed: two 2.54 mm
    hops of the +5V pull-up daisy-chain, reproducibly). For every non-GND
    net whose copper falls into >1 connected fragment, bridge the closest
    pad pair across fragments with a clearance-checked straight/L track."""
    for _ in range(8):
        split = _find_split(board)
        if split is None:
            return
        net, frag_a, frag_b = split
        if not _bridge_fragments(board, net, frag_a, frag_b, target):
            raise SystemExit(f"net {net} is split and no clear repair "
                             f"path was found")
    raise SystemExit("net repair did not converge in 8 passes")


def _find_split(board):
    """Return (net, pads_a, pads_b) for the first non-GND net whose items
    form more than one connected component, else None."""
    by_net: dict = {}
    for fp in board.GetFootprints():
        for pad in fp.Pads():
            n = pad.GetNetname()
            if n and n != "/GND":
                by_net.setdefault(n, []).append(("pad", pad))
    for t in board.GetTracks():
        n = t.GetNetname()
        if n and n != "/GND":
            by_net.setdefault(n, []).append(
                ("via" if t.GetClass() == "PCB_VIA" else "track", t))

    for net, items in by_net.items():
        parent = list(range(len(items)))

        def find(i):
            while parent[i] != i:
                parent[i] = parent[parent[i]]
                i = parent[i]
            return i

        def union(i, j):
            parent[find(i)] = find(j)

        for i in range(len(items)):
            for j in range(i + 1, len(items)):
                if _touches(items[i], items[j]):
                    union(i, j)
        roots = {}
        for i in range(len(items)):
            roots.setdefault(find(i), []).append(items[i])
        if len(roots) > 1:
            frags = sorted(roots.values(), key=len, reverse=True)
            pads_a = [it for k, it in frags[0] if k == "pad"]
            pads_b = [it for k, it in frags[1] if k == "pad"]
            if pads_a and pads_b:
                return net, pads_a, pads_b
    return None


def _touches(a, b) -> bool:
    (ka, ia), (kb, ib) = a, b
    if ka == "pad" and kb == "pad":
        ra, rb = ia.GetBoundingBox(), ib.GetBoundingBox()
        return (ra.GetLeft() <= rb.GetRight() and rb.GetLeft() <= ra.GetRight()
                and ra.GetTop() <= rb.GetBottom() and rb.GetTop() <= ra.GetBottom())
    if ka == "pad" or kb == "pad":
        pad, tr = (ia, ib) if ka == "pad" else (ib, ia)
        bb = pad.GetBoundingBox()
        rect = (bb.GetLeft() / 1e6, bb.GetTop() / 1e6,
                bb.GetRight() / 1e6, bb.GetBottom() / 1e6)
        if tr.GetClass() == "PCB_VIA":
            p = tr.GetPosition()
            return seg_rect_dist(p.x / 1e6, p.y / 1e6, p.x / 1e6, p.y / 1e6,
                                 *rect) <= 0.3
        if (pad.GetAttribute() == pcbnew.PAD_ATTRIB_SMD
                and not pad.IsOnLayer(tr.GetLayer())):
            return False
        s, e = tr.GetStart(), tr.GetEnd()
        return seg_rect_dist(s.x / 1e6, s.y / 1e6, e.x / 1e6, e.y / 1e6,
                             *rect) <= tr.GetWidth() / 2e6 + 0.01
    # track/via vs track/via
    if ka == "via" or kb == "via":
        via, other = (ia, ib) if ka == "via" else (ib, ia)
        p = via.GetPosition()
        if other.GetClass() == "PCB_VIA":
            q = other.GetPosition()
            return math.hypot((p.x - q.x) / 1e6, (p.y - q.y) / 1e6) <= 0.6
        s, e = other.GetStart(), other.GetEnd()
        return seg_point_dist(s.x / 1e6, s.y / 1e6, e.x / 1e6, e.y / 1e6,
                              p.x / 1e6, p.y / 1e6) <= 0.3 + other.GetWidth() / 2e6
    if ia.GetLayer() != ib.GetLayer():
        return False
    sa, ea = ia.GetStart(), ia.GetEnd()
    sb, eb = ib.GetStart(), ib.GetEnd()
    tol = (ia.GetWidth() + ib.GetWidth()) / 4e6 + 0.01
    return seg_seg_dist((sa.x / 1e6, sa.y / 1e6, ea.x / 1e6, ea.y / 1e6),
                        (sb.x / 1e6, sb.y / 1e6, eb.x / 1e6, eb.y / 1e6)) <= tol


def _bridge_fragments(board, net, pads_a, pads_b, target) -> bool:
    netinfo = board.FindNet(net)
    pairs = []
    for pa in pads_a:
        for pb in pads_b:
            qa, qb = pa.GetPosition(), pb.GetPosition()
            pairs.append((math.hypot((qa.x - qb.x) / 1e6, (qa.y - qb.y) / 1e6),
                          pa, pb))
    pairs.sort(key=lambda p: p[0])
    for d, pa, pb in pairs[:40]:
        qa, qb = pa.GetPosition(), pb.GetPosition()
        ax, ay, bx, by = qa.x / 1e6, qa.y / 1e6, qb.x / 1e6, qb.y / 1e6
        for layer in (pcbnew.B_Cu, pcbnew.F_Cu):
            if (pa.GetAttribute() == pcbnew.PAD_ATTRIB_SMD
                    and not pa.IsOnLayer(layer)):
                continue
            if (pb.GetAttribute() == pcbnew.PAD_ATTRIB_SMD
                    and not pb.IsOnLayer(layer)):
                continue
            for w in (target(net), 0.25, 0.2):
                for path in (((ax, ay, bx, by),),
                             ((ax, ay, bx, ay), (bx, ay, bx, by)),
                             ((ax, ay, ax, by), (ax, by, bx, by))):
                    if all(_repair_clear(board, net, seg, w, layer)
                           for seg in path):
                        for seg in path:
                            t = pcbnew.PCB_TRACK(board)
                            t.SetStart(pcbnew.VECTOR2I(int(seg[0] * 1e6),
                                                       int(seg[1] * 1e6)))
                            t.SetEnd(pcbnew.VECTOR2I(int(seg[2] * 1e6),
                                                     int(seg[3] * 1e6)))
                            t.SetWidth(int(w * 1e6))
                            t.SetLayer(layer)
                            t.SetNet(netinfo)
                            board.Add(t)
                        print(f"  repaired split net {net}: "
                              f"({ax:.2f},{ay:.2f})->({bx:.2f},{by:.2f}) "
                              f"w{w} L{layer} [{len(path)} seg]")
                        return True
    return False


def _repair_clear(board, net, seg, w, layer) -> bool:
    x1, y1, x2, y2 = seg
    need = w / 2 + 0.2
    for fp in board.GetFootprints():
        for pad in fp.Pads():
            if pad.GetNetname() == net:
                continue
            if not pad.IsOnLayer(layer):
                continue
            bb = pad.GetBoundingBox()
            if seg_rect_dist(x1, y1, x2, y2, bb.GetLeft() / 1e6, bb.GetTop() / 1e6,
                             bb.GetRight() / 1e6, bb.GetBottom() / 1e6) < need:
                return False
    for t in board.GetTracks():
        if t.GetNetname() == net:
            continue
        s, e = t.GetStart(), t.GetEnd()
        if t.GetClass() == "PCB_VIA":
            if (seg_point_dist(x1, y1, x2, y2, s.x / 1e6, s.y / 1e6)
                    < need + t.GetDrillValue() / 2e6 + 0.15):
                return False
        elif t.GetLayer() == layer:
            if (seg_seg_dist((x1, y1, x2, y2),
                             (s.x / 1e6, s.y / 1e6, e.x / 1e6, e.y / 1e6))
                    < need + t.GetWidth() / 2e6):
                return False
    return True


def heal_zone_islands(board, filler, zones) -> None:
    """LESSONS_LEARNED §12, automated. Two mechanisms, tried in order:

    1. PINCH STITCH (B.Cu): where the fill fragments only because it needs
       min_thickness + clearance, a plain 0.3 mm GND track fits through the
       pinch between island and main fill.
    2. VIA BRIDGE (F.Cu): a fully FENCED island (surrounded by other-net
       B.Cu copper) is unreachable on its own layer — drop a GND via inside
       the island and run an F.Cu track (straight or L) to the nearest GND
       through-hole pad of the main fragment.

    Refill + recheck after each stitch; abort loudly if an island survives.
    """
    gnd = board.FindNet("/GND")

    def outlines():
        poly = zones[0].GetFilledPolysList(pcbnew.B_Cu)
        outs = []
        for i in range(poly.OutlineCount()):
            ch = poly.Outline(i)
            pts = [(ch.CPoint(k).x / 1e6, ch.CPoint(k).y / 1e6)
                   for k in range(ch.PointCount())]
            bb = ch.BBox()
            outs.append((bb.GetWidth() / 1e6 * bb.GetHeight() / 1e6, pts, ch))
        return sorted(outs, key=lambda o: -o[0])

    def clear_of_others(x1, y1, x2, y2, w, layer) -> bool:
        need = w / 2 + 0.2
        for fp in board.GetFootprints():
            for pad in fp.Pads():
                if pad.GetNetname() == "/GND":
                    continue
                if not pad.IsOnLayer(layer):
                    continue
                bb = pad.GetBoundingBox()
                if seg_rect_dist(x1, y1, x2, y2, bb.GetLeft() / 1e6, bb.GetTop() / 1e6,
                                 bb.GetRight() / 1e6, bb.GetBottom() / 1e6) < need:
                    return False
        for t in board.GetTracks():
            if t.GetNetname() == "/GND":
                continue
            s, e = t.GetStart(), t.GetEnd()
            if t.GetClass() == "PCB_VIA":
                if (seg_point_dist(x1, y1, x2, y2, s.x / 1e6, s.y / 1e6)
                        < need + t.GetDrillValue() / 2e6 + 0.15):
                    return False
            elif t.GetLayer() == layer:
                if (seg_seg_dist((x1, y1, x2, y2),
                                 (s.x / 1e6, s.y / 1e6, e.x / 1e6, e.y / 1e6))
                        < need + t.GetWidth() / 2e6):
                    return False
        return True

    def drill_positions():
        out = []
        for fp in board.GetFootprints():
            for pad in fp.Pads():
                dr = pad.GetDrillSize()
                if dr.x > 0:
                    p = pad.GetPosition()
                    out.append((p.x / 1e6, p.y / 1e6, dr.x / 2e6))
        for t in board.GetTracks():
            if t.GetClass() == "PCB_VIA":
                p = t.GetPosition()
                out.append((p.x / 1e6, p.y / 1e6, t.GetDrillValue() / 2e6))
        return out

    def add_track(x1, y1, x2, y2, layer):
        t = pcbnew.PCB_TRACK(board)
        t.SetStart(pcbnew.VECTOR2I(int(x1 * 1e6), int(y1 * 1e6)))
        t.SetEnd(pcbnew.VECTOR2I(int(x2 * 1e6), int(y2 * 1e6)))
        t.SetWidth(int(0.3 * 1e6))
        t.SetLayer(layer)
        t.SetNet(gnd)
        board.Add(t)

    def try_pinch(island_pts, main_pts) -> bool:
        pairs = sorted(((math.hypot(ax - bx, ay - by), ax, ay, bx, by)
                        for ax, ay in island_pts for bx, by in main_pts),
                       key=lambda p: p[0])
        for d, ax, ay, bx, by in pairs[:400]:
            if d > 10.0:
                break
            if clear_of_others(ax, ay, bx, by, 0.3, pcbnew.B_Cu):
                add_track(ax, ay, bx, by, pcbnew.B_Cu)
                print(f"  pinch-stitched island: ({ax:.2f},{ay:.2f})->({bx:.2f},{by:.2f})")
                return True
        return False

    extra_targets: list = []   # GND anchor points of islands already
                               # bridged to main (electrically connected,
                               # even though geometrically separate fills)

    def try_via_bridge(island_chain, island_pts, main_chain) -> bool:
        drills = drill_positions()
        # source points, cheapest first: (x, y, needs_via)
        # 1. THT GND pads already inside the island — their F.Cu annulus can
        #    take a top-side track directly, no via needed;
        spots = []
        for fp in board.GetFootprints():
            for pad in fp.Pads():
                if (pad.GetNetname() == "/GND"
                        and pad.GetAttribute() != pcbnew.PAD_ATTRIB_SMD):
                    p = pad.GetPosition()
                    if island_chain.PointInside(p):
                        spots.append((p.x / 1e6, p.y / 1e6, False))
        # 2. free via spots: grid over the island, inside the fill, clear of
        #    other-net copper on both layers and of every drill
        xs = [p[0] for p in island_pts]
        ys = [p[1] for p in island_pts]
        step = 0.25
        gx = min(xs)
        while gx <= max(xs):
            gy = min(ys)
            while gy <= max(ys):
                pt = pcbnew.VECTOR2I(int(gx * 1e6), int(gy * 1e6))
                if (island_chain.PointInside(pt)
                        and clear_of_others(gx, gy, gx, gy, 0.6, pcbnew.B_Cu)
                        and clear_of_others(gx, gy, gx, gy, 0.6, pcbnew.F_Cu)
                        and all(math.hypot(gx - dx, gy - dy) >= r + 0.65
                                for dx, dy, r in drills)):
                    spots.append((gx, gy, True))
                gy += step
            gx += step
        if not spots:
            return False
        # targets: GND through-hole pads AND GND vias (earlier bridges count)
        # whose centre lies in the MAIN fill
        targets = []
        for fp in board.GetFootprints():
            for pad in fp.Pads():
                if (pad.GetNetname() == "/GND"
                        and pad.GetAttribute() != pcbnew.PAD_ATTRIB_SMD):
                    p = pad.GetPosition()
                    if main_chain.PointInside(p):
                        targets.append((p.x / 1e6, p.y / 1e6))
        for t in board.GetTracks():
            if t.GetClass() == "PCB_VIA" and t.GetNetname() == "/GND":
                p = t.GetPosition()
                if main_chain.PointInside(p):
                    targets.append((p.x / 1e6, p.y / 1e6))
        targets.extend(extra_targets)
        cands = sorted(((math.hypot(vx - tx, vy - ty), needs_via, vx, vy, tx, ty)
                        for vx, vy, needs_via in spots for tx, ty in targets),
                       key=lambda c: (c[1], c[0]))   # pad sources first
        for d, needs_via, vx, vy, tx, ty in cands[:600]:
            if d > 25.0:
                continue
            for path in (((vx, vy, tx, ty),),
                         ((vx, vy, tx, vy), (tx, vy, tx, ty)),
                         ((vx, vy, vx, ty), (vx, ty, tx, ty))):
                if all(clear_of_others(*seg, 0.3, pcbnew.F_Cu) for seg in path):
                    if needs_via:
                        via = pcbnew.PCB_VIA(board)
                        via.SetPosition(pcbnew.VECTOR2I(int(vx * 1e6), int(vy * 1e6)))
                        via.SetDrill(int(0.3 * 1e6))
                        via.SetWidth(int(0.6 * 1e6))
                        via.SetNet(gnd)
                        board.Add(via)
                    for seg in path:
                        add_track(*seg, pcbnew.F_Cu)
                    print(f"  {'via' if needs_via else 'pad'}-bridged island: "
                          f"({vx:.2f},{vy:.2f}) -> ({tx:.2f},{ty:.2f}) "
                          f"[{len(path)} seg]")
                    return True
        return False

    def unconnected() -> int:
        board.BuildConnectivity()
        try:
            return board.GetConnectivity().GetUnconnectedCount(True)
        except TypeError:
            return board.GetConnectivity().GetUnconnectedCount()

    bridged: set = set()
    for attempt in range(8):
        if unconnected() == 0:
            if attempt:
                print(f"  zone islands healed ({attempt} bridge(s))")
            return
        outs = outlines()
        _, main_pts, main_chain = outs[0]
        target_isles = [(pts, ch) for _, pts, ch in outs[1:]
                        if _bbox_key(pts) not in bridged]
        if not target_isles:
            # the remaining ratsnest is NOT a zone island (e.g. a net
            # freerouting genuinely failed to route) — that is the DRC
            # gate's job to report, not ours to mask
            print(f"  [WARN] {unconnected()} unconnected item(s) remain that "
                  f"are not healable zone islands — the DRC gate will report")
            return
        island_pts, island_chain = target_isles[0]
        bridged.add(_bbox_key(island_pts))
        if try_pinch(island_pts, main_pts) or \
                try_via_bridge(island_chain, island_pts, main_chain):
            # this island is now electrically on the main net — its GND
            # through-hole pads can anchor bridges for later islands
            for fp in board.GetFootprints():
                for pad in fp.Pads():
                    if (pad.GetNetname() == "/GND"
                            and pad.GetAttribute() != pcbnew.PAD_ATTRIB_SMD):
                        pp = pad.GetPosition()
                        if island_chain.PointInside(pp):
                            extra_targets.append((pp.x / 1e6, pp.y / 1e6))
            filler.Fill(zones)
            continue
        raise SystemExit("zone island could not be healed "
                         "(no clear pinch stitch, pad bridge or via bridge)")
    raise SystemExit("zone island healing did not converge in 8 attempts")


def _bbox_key(pts) -> tuple:
    xs = [p[0] for p in pts]
    ys = [p[1] for p in pts]
    return (round(min(xs), 1), round(min(ys), 1),
            round(max(xs), 1), round(max(ys), 1))


if __name__ == "__main__":
    main()
