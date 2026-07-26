# Versioning — one version per hardware revision

This repo has **no archive**. Every version ever built lives in `versions/` and
stays **active**: buildable, documented, and (where the emulator can host it)
playable in the browser. Nothing is frozen, nothing is "the old one".

## The rule

> **A version is a hardware configuration.** Change the hardware — add, remove
> or rewire a component — and you create a new version. Change only the code,
> and you stay in the version you are in.

That is the whole methodology. A version number is a *board*, not a release: it
answers "what is on the breadboard?", so a diagram, a pin map, a firmware build
and an emulation can all belong to exactly one of them.

Corollaries:

- **Code-only work is not a new version.** Bug fixes, refactors, new melodies,
  better calibration → commit them into the version whose board they run on and
  log them in `CHANGELOG.md`.
- **Removing hardware also counts.** V4.5 exists partly because the relay pair
  and the water pump came *off* the board; V5 because the red LED and the MARGIN
  buttons did.
- **Fractional numbers are fine.** V4.5 sits between V4 and V5 because its board
  does — numbering follows the lineage, not the calendar.
- **A version is never retired.** V1 still builds; V4 still passes its emulation
  verify. If a revision cannot be revived (no emulator support, missing part),
  its README says so explicitly instead of hiding it in an archive.

## Anatomy of a version directory

```
versions/<vN-slug>/
├── README.md      ← what this board is · hardware delta vs the previous version
│                    · gameplay · build/flash · verification status
├── HARDWARE.md    ← pin map, bill of materials, wiring detail, ⚠️ deductions
├── firmware/      ← its own PlatformIO project (self-contained, builds in place)
├── emulation/     ← its own Velxio spec + generated .vlx (or a README saying
│                    why this board has no emulation yet)
└── images/        ← wiring-<vN>.png rendered by the wirewright engine,
                     plus any photos/diagrams specific to this board
```

Nothing outside the directory is needed to build or read a version. The only
shared things are: `docs/` (physics + fundamentals that never change),
`tools/wiring_diagrams.py` (all the diagram contracts in one place, so revisions
stay visually comparable), and the append-only `CHANGELOG.md` / `TODO.md`.

## Checklist — adding a new hardware revision

Say the board gains a rotary encoder. That is a hardware change → **V6**.

1. **Name it after the change.** `versions/v6-<what-changed>/` — the slug names
   the hardware delta (`v5-led-bar`, `v4-water-pump`), not the year.
2. **Copy the closest existing version** (usually the newest) into it:
   ```bash
   cp -r versions/v5-led-bar versions/v6-encoder
   rm -rf versions/v6-encoder/firmware/.pio versions/v6-encoder/emulation/runs
   ```
   Copy — never move. The version you branched from stays active.
3. **Firmware** — edit `versions/v6-encoder/firmware/`. Keep the header comment,
   the version string in the serial banner and `platformio.ini` in sync with the
   new name. Build every env:
   ```bash
   cd versions/v6-encoder/firmware && pio run -e nanoatmega328 -e nanoatmega328new -e emulation
   ```
4. **Diagram** — add a `build_v6()` contract to `tools/wiring_diagrams.py` and a
   `"v6"` row to `TARGETS`, then render. The wirewright engine DRC-validates it;
   **never hand-place wires** and never hand-draw a diagram:
   ```bash
   python3 tools/wiring_diagrams.py v6      # → versions/v6-encoder/images/wiring-v6.png
   ```
   Missing a part? Extend the engine in `../eda-wirewright/src/wirewright/library.py`
   (add a factory + a `registry.py` entry) — that is how `ultrasonic` and the
   1-channel `relay_module` arrived.
5. **Emulation** — update `emulation/lemon-piano.yaml` for the new circuit and
   verify headlessly from the version directory:
   ```bash
   cd versions/v6-encoder
   PIPE=../../../velxio-multi-board-emulator/harness/.venv/bin/velxio-pipeline
   $PIPE stack status
   $PIPE run --mode verify --spec emulation/lemon-piano.yaml --out emulation/runs
   cp emulation/runs/<latest>/project.vlx emulation/lemon-piano.vlx
   ```
   If the browser AVR cannot host the new part, say so in
   `emulation/README.md` — an honest gap beats a silently broken spec.
6. **Docs** — write `README.md` + `HARDWARE.md` for the version. The README must
   open with the **hardware delta vs the previous version**; that diff is the
   reason the version exists.
7. **Wire it into the index** — add a row to the table in the root `README.md`
   and in `versions/README.md`, and update `CLAUDE.md` if the newest board moved.
8. **Log it** — dated entry in `CHANGELOG.md`, tick the matching `TODO.md` item.

## Verification bar

A version is "active" only if these pass in *its* directory:

| Check | Command |
|---|---|
| Firmware builds (every env) | `cd firmware && pio run` |
| Emulation still green | `$PIPE run --mode verify --spec emulation/lemon-piano.yaml --out emulation/runs` |
| Diagram renders, 0 DRC violations | `python3 tools/wiring_diagrams.py <vN>` |

Run them before committing anything that touches a version — and re-run them for
**every** version after a change to something shared (`tools/`, the wirewright
engine, the Velxio harness).
