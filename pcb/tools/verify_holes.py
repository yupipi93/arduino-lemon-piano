#!/usr/bin/env python3
"""Lemon Piano anchor-hole verification gate.

GEOMETRIC check always runs (design .kicad_pcb vs ground-truth/holes.yaml:
presence, position, drill/pad Ø, pattern spacings) — it is the hard gate.

Since v0.3.0 the board has FOUR anchor holes, enough for the toolkit's
VISION pass (6-DOF affine over detected centres + leave-one-out). It runs
opportunistically against the version's normal renders when they exist
and numpy is importable (the Docker image has it); a detector failure
(e.g. background too uniform for the dark-bore finder) reports SKIP and
does not fail the gate — a *completed* vision pass with a real deviation
DOES fail.

Usage:
    python3 pcb/tools/verify_holes.py [--version vX.Y.Z] [--json]
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "eda-pcb-designer" / "src"))

from pcb_designer.verify.holes import (  # noqa: E402
    check_holes_geometric,
    load_holes_groundtruth,
    parse_design_holes,
)

PROJ = Path(__file__).resolve().parents[1]


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description="lemon-piano anchor-hole verifier")
    ap.add_argument("--version", default=None,
                    help="run the vision pass against renders/<v>-normal-*.png")
    ap.add_argument("--pcb", type=Path, default=PROJ / "kicad" / "lemon-piano.kicad_pcb")
    ap.add_argument("--ground-truth", type=Path,
                    default=PROJ / "ground-truth" / "holes.yaml")
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args(argv)

    design = parse_design_holes(args.pcb.read_text(encoding="utf-8"))
    gt = load_holes_groundtruth(args.ground_truth)
    findings = check_holes_geometric(design, gt)
    failed = [f.message for f in findings if not f.ok and f.severity == "critical"]

    vision: dict = {"status": "skipped", "reason": "no --version given"}
    if args.version:
        # transparent-background bare renders (LESSONS_LEARNED §22: with
        # an opaque background the raytrace bore shadow biases the
        # dark-bore centroid — observed 0.40 mm LOO on the top side)
        renders = {s: PROJ / "validation" / f"vision-{args.version}-{s}.png"
                   for s in ("top", "bottom")}
        missing = [s for s, p in renders.items() if not p.exists()]
        if missing:
            vision = {"status": "skipped",
                      "reason": f"render(s) missing: {missing}"}
        else:
            try:
                from pcb_designer.verify.holes import verify_holes as _vh
                rep = _vh(args.pcb, args.ground_truth, renders,
                          PROJ / "validation" / "holes")
                cv_fail = 0
                for sd in rep.get("sides", {}).values():
                    cv_fail += sum(1 for f in sd.get("cv_findings", [])
                                   if not f["ok"])
                vision = {"status": "ran", "pass": cv_fail == 0,
                          "sides": {s: {k: sd[k] for k in
                                        ("ppm", "max_loo_err_mm")
                                        if k in sd}
                                    for s, sd in rep.get("sides", {}).items()
                                    if "error" not in sd}}
                if cv_fail:
                    failed.append(f"vision: {cv_fail} hole(s) off-position")
            except ImportError as e:
                vision = {"status": "skipped", "reason": f"missing dep: {e}"}
            except Exception as e:
                vision = {"status": "skipped",
                          "reason": f"detector: {type(e).__name__}: {e}"}

    if args.json:
        print(json.dumps({
            "pass": not failed, "vision": vision,
            "findings": [{"ref": f.ref, "ok": f.ok, "severity": f.severity,
                          "message": f.message} for f in findings],
        }, ensure_ascii=False, indent=2, default=str))
    else:
        print(f"anchor holes: screw {gt.screw} drill Ø{gt.drill_dia} pad Ø{gt.pad_dia}")
        for f in findings:
            print(f"  [{'PASS' if f.ok else 'FAIL'}] {f.message}")
        print(f"  [vision] {vision}")
        print("  =>", "PASS" if not failed else "FAIL")
    return 0 if not failed else 1


if __name__ == "__main__":
    raise SystemExit(main())
