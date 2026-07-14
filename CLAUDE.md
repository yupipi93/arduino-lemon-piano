# CLAUDE.md — arduino-lemon-piano

Rescued 2019 university Arduino game: 7 lemons as touch keys, guess the secret
Mario melody. The **active version is V5** — a ten-green-LED progress bar, no
relays, games that auto-advance on a win. Read `README.md` for the game and
`docs/HARDWARE.md` for the pin map before changing firmware.

## Ground rules

- **Everything in English** — code, comments, identifiers, file names, docs.
- **Active = V5**: `firmware/` (Nano firmware, needs A6+A7) + `emulation/`
  (Velxio browser build). **V4 is frozen** in `archive/lemon-piano-v4/`
  (firmware + emulation) — the version with the red LED + relay water pump.
  Never edit `archive/` except its docs; the pristine Spanish 2019 originals are
  in git history (commit `rescue: original 2019 lemon piano files`).
- One source file, two builds: `firmware/src/main.cpp` compiles for hardware
  (macro undefined) and for the browser (`-DVELXIO_EMULATION`, see
  `emulation/README.md`). The shim swaps ONLY input/pin details (active-low
  keys, buzzer D11, no game-select/restart, game auto-advances); game logic and
  melodies are shared. Don't fork the logic.
- Append a dated entry to `CHANGELOG.md` for every significant change; tick the
  matching `TODO.md` item. Never rewrite old changelog entries.

## Build / flash (PlatformIO, no IDE)

```bash
cd firmware
pio run                    # compile check — run before every commit (default nanoatmega328)
pio run -e nanoatmega328new # Nano with new bootloader
pio run -e emulation        # compile-check the VELXIO_EMULATION shim
pio run -t upload           # flash the Nano
pio device monitor          # 9600 baud — logs Game/OK n/10/WIN
```

No `uno` env: V5 uses A6 (key 7) + A7 (game select), absent on the classic Uno.

## Verify the emulation (headless, no browser)

The `emulation/` build has a real regression test via the Velxio harness
(present at `../velxio-multi-board-emulator/`, pipeline in its
`harness/.venv/`). It injects game 1's secret code and asserts
`Game 1 → OK 1/10 … 10/10 → WIN → Game 2`:

```bash
PIPE=../../velxio-multi-board-emulator/harness/.venv/bin/velxio-pipeline
$PIPE stack status                                              # ensure Docker stack is up
$PIPE run --mode verify --spec emulation/lemon-piano.yaml --out emulation/runs
# regenerate the importable project: copy runs/<latest>/project.vlx → emulation/lemon-piano.vlx
```

Read the harness `AGENTS.md` before touching the emulation (routing rules,
AVR buzzer-on-PWM-pin + `OCR2A=0`, active-low pull-up inputs). **Never** destroy
its Docker volumes.

**Verification bar**: `pio run` on the two Nano envs + the `emulation` env, and
`--mode verify` green, before committing anything that touches gameplay.

## Layout

- `firmware/` + `emulation/` — active V5 (the code that evolves)
- `docs/HARDWARE.md` — V5 pin map + deduced schematic
- `archive/lemon-piano-v4/` — frozen V4 (firmware + emulation)
- `archive/banana-piano-original/` — frozen lineage (banana piano →
  keyboard-test → game-prototype/v3)
- `TODO.md` — backlog · `CHANGELOG.md` — append-only history
