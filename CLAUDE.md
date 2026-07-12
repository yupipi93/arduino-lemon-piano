# CLAUDE.md — arduino-lemon-piano

Rescued 2019 university Arduino game: 7 lemons as touch keys, guess the secret
Mario melody, water-pump penalty. Read `README.md` for the game and
`docs/HARDWARE.md` for the deduced wiring before changing firmware.

## Ground rules

- **Everything in English** — code, comments, identifiers, file names, docs.
- `firmware/src/main.cpp` is currently a **1:1 translation of the 2019 code,
  bugs included**. Fixes are planned, item by item, from `TODO.md` — don't
  "improve" behavior in passing; one TODO item per change.
- The pristine Spanish originals live in git history (commit
  `rescue: original 2019 lemon piano files`). `archive/` is frozen reference —
  never edit it except for docs.
- Append a dated entry to `CHANGELOG.md` for every significant change; check
  off the matching `TODO.md` item. Never rewrite old changelog entries.

## Build / flash (PlatformIO, no IDE)

```bash
cd firmware
pio run                    # compile check — run before every commit
pio run -t upload          # flash Arduino Nano (old bootloader, default env)
pio run -e nanoatmega328new -t upload   # Nano with new bootloader
pio run -e uno -t upload   # Arduino Uno
pio device monitor         # 9600 baud (Serial is currently disabled in setup)
```

There are no automated tests; **`pio run` for all three envs is the
verification bar**, plus on-hardware behavior checks when a change touches
gameplay.

## Layout

- `firmware/` — active PlatformIO project (the only code that evolves)
- `docs/HARDWARE.md` — pin map + deduced schematic; update its ⚠️ items when
  the physical build confirms/refutes a deduction
- `archive/banana-piano-original/` — frozen lineage (banana piano →
  keyboard-test → game-prototype/v3)
- `TODO.md` — ordered fix backlog · `CHANGELOG.md` — append-only history
