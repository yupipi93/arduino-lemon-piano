# 🍋 Arduino Lemon Piano

**A university Arduino game (02/2019), rescued and reworked in 2026.** Seven
lemons wired to an Arduino Nano become a touch keyboard. Guess the secret
10-note Mario melody — a row of **ten green LEDs** fills up one per correct
note, blanks if you slip, and when all ten are lit the theme plays and the game
**auto-advances** to the next tune.

> **Active version: V5** (10-LED progress bar). The original **V4** — single
> green/red LED and a relay-driven **water pump** that sprayed you on a late
> miss — is frozen in [`archive/lemon-piano-v4/`](archive/lemon-piano-v4/).

<div align="center">
<img src="docs/images/keyboard-breadboard-nano.png" alt="Lemon piano keyboard breadboard (Arduino Nano)" width="85%"/>
<br/><em>Keyboard stage of the circuit — on the real build, each button is a lemon + the player's body.</em>
</div>

---

## How V5 works

1. **Power on.** The A7 game-select switch picks the **starting** game: tied
   HIGH (5 V) = game 1 (Mario Main Theme), LOW (GND) = game 2 (Underworld).
2. **Touch lemons.** Hold the 5 V clip in one hand and touch a lemon with the
   other — your body closes the circuit and the note plays on the buzzer.
3. **Guess the secret sequence** (10 notes). Each correct note lights the next
   green LED (the progress bar climbs 1 → 10). A wrong note **blanks all ten
   LEDs** (with a short low tone) and the sequence restarts from the first note.
4. **Victory + auto-advance.** Light all ten LEDs → the theme plays with the
   whole bar lit, then the game flips to the **other** theme automatically
   (game 1 → 2 → 1 …) so both games cycle from one starting point.
5. **Restart** anytime with the button on D7 (re-reads the game-select switch,
   blanks the bar).

No relays, no water pump, no red LED, no fail-counter — the ten-LED bar is the
whole feedback surface. (All of those lived in V4; see the archive.)

### Secret codes (spoilers!)

Keys numbered 1–7, left to right:

| Game | Melody | Code |
|---|---|---|
| 1 | Super Mario Bros — Main Theme | `6, 5, 6, 7, 2, 5, 2, 1, 3, 4` |
| 2 | Super Mario Bros — Underworld Theme | `3, 6, 1, 4, 2, 5, 3, 6, 1, 4` |

| Key | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|---|---|---|---|---|---|---|---|
| Game 1 note | E6 | G6 | A6 | B6 | C7 | E7 | G7 |
| Game 2 note | A3 | A#3 | C4 | A4 | A#4 | C5 | D5 |

## Hardware (V5)

| Component | Qty | Notes |
|---|---|---|
| **Arduino Nano** (ATmega328P) | 1 | needs A6 **and** A7 → a classic Uno won't do (V4 ran on either) |
| Lemons 🍋 + alligator clips | 7 + 8 | 7 keys on A0–A6 + 1 hand-held 5 V clip |
| **Green LEDs** | **10** | the progress bar, on D2,D3,D4,D5,D6,D9,D10,D11,D12,D13 |
| 220 Ω resistors | 10 | one in series with each LED |
| Passive buzzer | 1 | D8 |
| Game-select switch (SPDT) | 1 | on A7 (analog-in): 5 V = game 1, GND = game 2 |
| Push button | 1 | restart, D7 |

Moving game-select to **A7** freed the digital pin the 10th LED needed; A7 is
analog-in only, so drive it with an SPDT switch (or a switch to 5 V + 10 kΩ
pulldown). Full pin map, deduced wiring and the touch-sensing physics:
**[docs/HARDWARE.md](docs/HARDWARE.md)**.

## Repo layout

```
arduino-lemon-piano/
├── README.md                    ← you are here
├── firmware/                    ← ACTIVE code — V5 PlatformIO project
│   ├── platformio.ini           ← envs: nanoatmega328 (default) · nanoatmega328new · emulation
│   ├── src/main.cpp             ← the V5 game (10-LED bar, auto-advance) + Velxio shim
│   └── include/notes.h          ← note frequency table
├── emulation/                   ← ACTIVE Velxio browser emulation of V5 (10 LEDs)
│   ├── lemon-piano.yaml          ← circuit spec (source of truth)
│   ├── lemon-piano.vlx           ← generated project — import into Velxio and Run
│   └── README.md
├── docs/
│   ├── HARDWARE.md              ← pin map, deduced schematic, sensing explained
│   ├── keyboard-schematic.fzz   ← editable Fritzing source
│   └── images/                  ← breadboard diagrams
├── archive/
│   ├── lemon-piano-v4/          ← ❄️ frozen V4 (firmware + emulation): pump, red/green LED
│   └── banana-piano-original/   ← frozen 2019 reference code (translated)
│       ├── banana-piano/        ← the original banana piano (untitled.es tutorial base)
│       ├── keyboard-test/       ← 7-key piano test, no game logic
│       └── game-prototype/      ← v3-era game (Uno, inverted threshold) — v4's ancestor
├── CHANGELOG.md · TODO.md · CLAUDE.md
```

## 🎮 Play it in the browser (no hardware)

V5 runs in an interactive [Velxio](https://github.com/davidmonterocrespo24/velxio)
emulation — real firmware on an emulated ATmega328, clickable lemons, audible
buzzer and the ten-LED bar:

```bash
../velxio-multi-board-emulator/harness/.venv/bin/velxio-pipeline stack up
# then open http://localhost:3080/editor and import emulation/lemon-piano.vlx → Run
```

Because ten LEDs use every free browser pin, the emulation has no game-select
switch or restart button: it starts at game 1 and **auto-advances** on each win.
Details, key mapping, and the headless `verify` regression test (plays game 1's
code and asserts `WIN` then the flip to `Game 2`):
**[emulation/README.md](emulation/README.md)**. Last verify: ✅ pass.

## Quick start

```bash
# Install PlatformIO CLI once (any of):
pipx install platformio          # or: pip install --user platformio

cd firmware
pio run                          # build (default env: nanoatmega328, old bootloader)
pio run -t upload                # flash the Nano
pio run -e nanoatmega328new -t upload   # if upload fails: new-bootloader Nano
```

No hardware needed to build — `pio run` is the compile check used before
committing.

## History

- **2019 originals** preserved verbatim in git history (commit
  `rescue: original 2019 lemon piano files`).
- **V4** — the rescued game: English translation, PlatformIO layout, the
  `Duracion`-line reconstruction, and TODO #1–#12 bug fixes (edge-triggered
  input, `millis()` timing, PROGMEM melodies, auto-calibrated touch). Frozen in
  [`archive/lemon-piano-v4/`](archive/lemon-piano-v4/); full story in
  [CHANGELOG.md](CHANGELOG.md).
- **V5** — this rework: relays/pump/red-LED removed, ten-LED progress bar,
  game-select on A7, auto-advance between the two games.

### Lineage

1. **Banana piano** ([untitled.es](http://untitled.es) tutorial) — 7 fruit
   keys playing fixed notes on an Uno. → `archive/banana-piano-original/banana-piano/`
2. **Game prototype (v3)** — adds the secret-sequence game, LEDs, one relay.
   → `archive/banana-piano-original/game-prototype/`
3. **Lemon Piano V4 (02/2019)** — Nano, two games, water-pump penalty, death
   melody. → `archive/lemon-piano-v4/`
4. **Lemon Piano V5 (2026)** — ten-LED progress bar, no relays, auto-advancing
   games. → `firmware/` + `emulation/`

---

*Author: Yupipi93 (Sergio Conejero), 2019 · Rescued, reworked & documented with Claude, 2026*
