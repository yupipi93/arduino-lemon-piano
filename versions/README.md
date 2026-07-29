# Versions — every board, all active

One directory per **hardware revision**. Nothing here is archived or frozen: each
version keeps its own firmware, emulation, wiring diagram and docs, and each is
expected to build and (where the browser emulator can host it) run.

The rule and the recipe for adding one: [../docs/VERSIONING.md](../docs/VERSIONING.md).

| # | Version | Board | Hardware delta vs previous | Firmware | Emulation | Diagram |
|---|---|---|---|---|---|---|
| 0 | [v0-buzzer](v0-buzzer/) | any Nano / Uno | *bring-up rig (2026)*: one passive buzzer on D8, nothing else | ✅ 5 envs | ✅ verify green | [wiring-v0](v0-buzzer/images/wiring-v0.png) |
| 1 | [v1-banana-piano](v1-banana-piano/) | Uno (2019) · Nano for key 7 | *the 2019 origin*: + 7 fruit keys + 220 Ω pull-ups, + HC-SR04 (buzzer already on D8) | ✅ builds | — [why](v1-banana-piano/emulation/README.md) | [wiring-v1](v1-banana-piano/images/wiring-v1.png) |
| 2 | [v2-keyboard-test](v2-keyboard-test/) | Uno · Nano for key 7 | − HC-SR04 (keyboard + speaker only) | ✅ builds | — [why](v2-keyboard-test/emulation/README.md) | [wiring-v2](v2-keyboard-test/images/wiring-v2.png) |
| 2.5 | [v2.5-threshold-buttons](v2.5-threshold-buttons/) | Uno · Nano for key 7 | + 2 buttons (D10/D11, to GND) that tune the touch threshold live; serial readout on | ✅ 4 envs | ✅ verify green | [wiring-v2.5](v2.5-threshold-buttons/images/wiring-v2.5.png) |
| 3 | [v3-game-prototype](v3-game-prototype/) | Uno · Nano for key 7 | + red LED D2, green LED D3, game button D4, 1 relay D5 | ✅ builds | — [why](v3-game-prototype/emulation/README.md) | [wiring-v3](v3-game-prototype/images/wiring-v3.png) |
| 4 | [v4-water-pump](v4-water-pump/) | **Nano** | clip flips to **+5 V** (sensing inverted) · + 2nd relay channel + water pump · + RESTART D7 · SPDT game select | ✅ 4 envs | ✅ verify green | [wiring-v4](v4-water-pump/images/wiring-v4.png) |
| 5 | [v4.5-margin-buttons](v4.5-margin-buttons/) | Nano | **− relay pair + water pump** (D5/D6 free) · + MARGIN + (D10) / MARGIN − (D11) buttons | ✅ 4 envs | ✅ verify green | [wiring-v4.5](v4.5-margin-buttons/images/wiring-v4.5.png) |
| 6 | [v5-led-bar](v5-led-bar/) | Nano only (A6+A7) | keyboard back to **220 Ω pull-ups + GND clip** · − red LED, − MARGIN buttons, − game-select, − restart · + **ten green LEDs** · + 2 sensitivity buttons (D7/A7) | ✅ 4 envs | ✅ 3 specs green | [wiring-v5](v5-led-bar/images/wiring-v5.png) |


**Newest board: [V5](v5-led-bar/).** Change the hardware and the next directory is
`v6-<what-changed>/`.

## Three things worth knowing

- **V0 and V2.5 are instruments, not history.** V1–V5 are the real lineage; V0
  (bare buzzer) and V2.5 (keyboard + live threshold) were built in 2026 to measure
  the thing that was misbehaving. Both are numbered where their *board* sits.
- **V0 is a diagnostic, not history.** V1–V5 are the real lineage; V0 was built in
  2026 to isolate the buzzer from everything else. It is numbered 0 because its
  board is a subset of every other one. Flash it whenever you doubt the sound.

- **The numbers follow the boards, not the calendar.** V1–V4 are 2019; V5 was
  built on 2026-07-14 and V4.5 on 2026-07-25/26 — V4.5 is "between" V4 and V5
  because its *board* is (V4 minus the pump, plus two buttons), even though it was
  wired last.
- **The 2019 boards have no emulation on purpose.** Hosting them in the browser
  AVR would mean editing 2019 sketches (buzzer onto a PWM pin, key 7 off A6);
  each version's `emulation/README.md` spells out exactly what it would take, and
  the decision is open in [../TODO.md](../TODO.md).

## Quick tour of one version

```
versions/v5-led-bar/
├── README.md      ← what this board is, its hardware delta, how to play/build
├── HARDWARE.md    ← pin map, BOM, wiring detail, ⚠️ deductions
├── firmware/      ← PlatformIO project (builds in place: cd firmware && pio run)
├── emulation/     ← Velxio spec + generated .vlx + how to verify it
└── images/        ← wiring-v5.png, rendered by the wirewright engine
```
