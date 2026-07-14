# Lemon Piano V4 — frozen snapshot ❄️

The complete **V4** version (firmware + Velxio emulation), frozen when the
active build moved on to **V5** (10-LED progress bar, no water pump). Kept so
V4 stays buildable and playable without digging through git history.

This is the game as documented in the top-level [../../README.md](../../README.md):
7 analog lemon keys, guess the 10-note Mario melody, **green LED = right / red =
wrong**, and a **relay-driven water pump** sprays you on a late miss (10
penalties → death tune). V5 removed the relays and the red LED and replaced the
single green LED with ten green LEDs.

## What's here

```
archive/lemon-piano-v4/
├── firmware/      ← V4 PlatformIO project (frozen copy of the old top-level firmware/)
│   ├── platformio.ini   envs: nanoatmega328 · nanoatmega328new · uno · emulation
│   ├── src/main.cpp      the fixed & improved V4 game (TODO #1–#12 applied)
│   └── include/notes.h
└── emulation/     ← V4 Velxio browser emulation (frozen)
    ├── lemon-piano.yaml   circuit spec (red/green/blue LEDs, pump indicator)
    ├── lemon-piano.vlx    generated project
    └── README.md
```

## Build it

The internal relative paths are self-consistent, so it builds in place:

```bash
cd archive/lemon-piano-v4/firmware
~/.local/bin/pio run                 # nanoatmega328 (default)
~/.local/bin/pio run -e emulation    # compile-check the VELXIO_EMULATION shim
```

## Run the emulation

The emulation spec still references the live Velxio harness. Regenerate/verify
from **this** folder:

```bash
cd archive/lemon-piano-v4
PIPE=../../../../velxio-multi-board-emulator/harness/.venv/bin/velxio-pipeline
$PIPE run --mode verify --spec emulation/lemon-piano.yaml --out emulation/runs
```

> The paths and pin map inside [`emulation/README.md`](emulation/README.md) are
> written relative to the *original* repo-root location (`emulation/`,
> `../../velxio-multi-board-emulator/`). Add one `../` level for each when
> running from this archived location, or just read them as historical
> reference — the authoritative live emulation is the top-level `emulation/`.
