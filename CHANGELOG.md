# Changelog

Append-only log of significant changes. Newest first.

## 2026-07-26 — Wiring engine extracted to its own repo (eda-wirewright)

The schematic engine that was living in `tools/schematic/` is now a standalone,
professional package: **[eda-wirewright](../eda-wirewright/)** (CLI, declarative
JSON contract format, MCP server for AI, Docker, tests, CI). This project just
*consumes* it now:

- Removed `tools/schematic/` (moved, not deleted — it's `eda-wirewright/src/wirewright/`).
- `tools/wiring_diagrams.py` imports `wirewright` via a `sys.path` shim to
  `../eda-wirewright/src` (no install needed, only Pillow). The three contracts
  are unchanged; `python3 tools/wiring_diagrams.py` still regenerates
  `docs/images/wiring-{v4,v4-plus,v5}.png` (36 / 42 / 55 nets, 0 DRC violations).

## 2026-07-25 (engine) — Real schematic engine: auto-router + DRC (no more overlaps)

Rewrote the wiring-diagram generator from hand-placed coordinates into a proper,
reusable **schematic engine** (`tools/schematic/`, ~1160 lines) — because
patching coordinates by hand kept producing the same faults (wires over
resistors, confusing crossings, wires too close, resistors left visually
unconnected, wires under components). Now the diagrams are *declarative
contracts* and the engine guarantees correctness:

- **Declarative model**: you state components, typed ports and nets (what
  connects to what); a 3+-terminal net (e.g. a Nano pin + button + pulldown that
  are one node) routes as one clean tree. `tools/wiring_diagrams.py` is now just
  the three contracts (~200 lines).
- **Grid A\* maze router** (`router.py`): shortest orthogonal paths on a routing
  grid, component bodies (+clearance) are hard obstacles so a wire can never
  cross one, a large **bend penalty** yields long straight runs, a light
  **proximity penalty** keeps wires apart (they cross cleanly rather than
  detour), and every pin gets a perpendicular **escape stub**. Labels are a soft
  obstacle so wires route around text.
- **DRC that runs before every save** (`validator.py`): raises on wire-over-body,
  coincident wires, unconnected pins, or wires-too-close. The three diagrams now
  report **0 hard violations, 0 spacing warnings** (36 / 42 / 55 nets). A broken
  diagram literally cannot be saved.
- Design follows standard EDA practice (Lee/A\* maze routing, bend/proximity
  cost, net ordering, junction-dot rules, label placement). See
  `tools/schematic/README.md`.

Same three outputs (`docs/images/wiring-{v4,v4-plus,v5}.png`), regenerated with
`python3 tools/wiring_diagrams.py`; canvas widened to 3400 px for breathing room.

## 2026-07-25 (diagrams) — Fix overlapping wires in the wiring diagrams

`tools/wiring_diagrams.py` v1 let several control wires (game-select, buzzer,
restart, MARGIN+/-) share the same y as the component row they fed into, so a
wire bound for a farther component cut straight through a nearer component's
body (and some wires' lanes coincided with LEDs' full-height GND drops).
Fixed with a `connect_over_top()` router: every controls-row wire first climbs
to its OWN private "highway" level — all comfortably above the LED tops — then
crosses over and drops straight down into its own terminal, so no wire ever
overlaps a component's bounding box. Canvas widened (2500→3700px) to give
each component (LEDs, relay+pump, game-select, buzzer, restart, MARGIN+/-)
its own clear horizontal slot. V5 also got its RESTART/BUZZER/LED-bar
positions rebased off `NANO_X1` (they were absolute pixel coordinates that the
wider canvas had shifted the Nano away from). Regenerated all three PNGs.

## 2026-07-25 — V4 touch upgrade (in-place) + manual MARGIN buttons + wiring diagrams

Backported V5's touch-sensing improvements onto the **archived V4** and added a
live manual override. The V4 **game is unchanged** (relay water pump, red/green
LED, 10-penalty death tune, both secret codes all behave exactly as before) —
only the touch layer and two new hardware buttons changed. This deliberately
edits the otherwise-frozen `archive/lemon-piano-v4/` (owner-authorised; the
pristine 2019 original still lives in git history).

- **Noise-adaptive calibration ported to V4** (`archive/lemon-piano-v4/firmware/src/main.cpp`):
  the fixed `TOUCH_MARGIN 100` is gone. `calibrate()` now takes 64 samples over
  ~128 ms per key (mean baseline + peak) and sets
  `threshold = baseline + max(MIN_TOUCH_MARGIN=40, NOISE_FACTOR=3 × (peak−mean))`,
  capped at `THRESHOLD_CAP=900`. It runs at boot **and on every RESTART**
  (moved from `setup()` into loop's `!started` branch), so ghost presses from a
  new fruit / outlet / PSU are re-tuned by pressing RESTART (hands off).
- **Manual MARGIN buttons (hardware only)**: `MARGIN +` on **D10** and
  `MARGIN −` on **D11** (active-HIGH, wired like RESTART, edge-triggered). They
  shift a `manualMargin` offset applied on top of each key's auto margin
  (`applyThresholds()`), bounded to `[−120, +400]` in `MARGIN_STEP=10` nudges,
  with an `EFFECTIVE_MARGIN_FLOOR=8` so a threshold never sits on the baseline.
  Each press gives an audible tick (high = up, low = down) + a serial line.
  Emulation build is untouched — the buttons are `#ifndef VELXIO_EMULATION`
  (its keys are active-low digital and D11 is the buzzer there).
- **Verified**: all four PlatformIO envs green (nanoatmega328 / …new / uno /
  emulation; hardware RAM 341 B / 16.7 %), and the V4 Velxio emulation
  `--mode verify` regression still passes (game 2's code → `WIN`).
- **Wiring diagrams** (new `tools/wiring_diagrams.py`, pure-PIL, adapted from
  the oscilloscope project's technique) → `docs/images/wiring-{v4,v4-plus,v5}.png`:
  one per version (original V4, V4+ with the touch upgrade, and V5), each with
  the 7-lemon keyboard, colour-coded wires and a legend.

## 2026-07-14 (calibration) — Noise-adaptive touch margin, recalibrate on RESTART

Fixes the critical hardware ghost-press problem (margin needs differ per
fruit / mains outlet / 5V PSU — a fixed `TOUCH_MARGIN 100` can't serve all):

- `calibrate()` now measures each key's real noise: 64 samples over ~128ms
  (spans 6+ mains cycles at 50Hz), records mean baseline AND peak, and sets
  `threshold = baseline + max(MIN_TOUCH_MARGIN=40, 3x(peak-mean))`. Quiet
  supplies get MORE sensitive than the old fixed 100; noisy chargers get a
  margin wide enough to kill ghost presses. Thresholds are capped at 900
  with a serial warning (`VERY NOISY - check fruit contact / power supply`).
- **Recalibration on every RESTART press**, not just power-on — `calibrate()`
  moved from `setup()` into the `!started` branch. If the piano misbehaves
  after changing fruit or outlet: hands off the lemons, press RESTART.
- The LED bar sweeps one LED per key during calibration (visual "hands off"
  feedback), and serial logs per-key `baseline / noise / threshold`.
- Emulation path untouched (its calibrate() is the solver boot guard);
  headless verify green; all three PlatformIO envs green (flash 20%).

## 2026-07-14 (light show) — LED bar flashes to the victory theme

`playSong()` now flashes the whole ten-LED bar to the beat: lit while each
note sounds (`buzz` blocks for the note's duration), dark in the inter-note
gap and during rests. Shared game logic — identical on hardware and in the
browser build (new `allLedsOn()` helper mirrors `allLedsOff()`). Verified
headlessly: LED 10 (D13), which only ever lights during the win, toggles 61
times across the Mario victory tail. All three PlatformIO envs green.

## 2026-07-14 — V5: ten-LED progress bar, auto-advancing games

New hardware version. **V4 is frozen** in `archive/lemon-piano-v4/` (firmware +
emulation) and the top-level `firmware/` + `emulation/` are now V5.

- **Removed**: the 2-channel relay pair + water pump, the red LED, and the
  fail-counter / death-tune game-over that was coupled to the pump.
- **Added**: a **ten-green-LED progress bar** on
  `D2,D3,D4,D5,D6,D9,D10,D11,D12,D13`. Each correct note lights the next LED; a
  wrong note blanks all ten (with a short low tone) and restarts the sequence;
  ten lit = win.
- **Auto-advance**: winning a theme flips the game to the other one
  (1 → 2 → 1 …), so both games cycle from a single starting point. On hardware
  the A7 switch picks the *starting* game; the browser build starts at game 1.
- **Game select moved D4 → A7** (analog-in) to free the digital pin the 10th LED
  needed. V5 therefore needs a Nano/Mini (uses A6 **and** A7) — the `uno` env is
  dropped.
- **Emulation reworked for V5** (`emulation/lemon-piano.yaml` + `.vlx`): ten
  green LEDs, keys 1–6 on A0–A5 + key 7 on D12, buzzer on D11. Ten LEDs consume
  every free browser pin, so the emulation has no game-select/restart (it relies
  on the auto-advance). **Headless verify green**: injects game 1's code →
  `Game 1 → OK 1/10 … 10/10 → WIN → Game 2` (all ten LED pins + buzzer toggling).
- Builds green on `nanoatmega328`, `nanoatmega328new`, `emulation` (RAM 311 B /
  15 %, flash 6.7 KB / 22 %).

## 2026-07-13 (routing) — Hard wire-routing rules in the pipeline

The harness now enforces reference-diagram wiring on every generated `.vlx`
(`velxio_harness/routing.py`, based on exact pin geometry extracted from the
live Velxio DOM): **no cable may cross any component** (generation aborts on
violation), no two cables may ride the same lane (staircase nesting in
adjacent 10px lanes, shortest wires innermost), long runs travel in
corridors instead of along pin rows, and stubs exit perpendicular to the
component edge. The lemon piano regenerates with 34 of 46 wires auto-routed
around the components; headless verify (secret code → WIN) still green.
Rules documented in the harness AGENTS.md.

## 2026-07-13 (final) — Emulation polish: keyboard play, switch, clean wiring

- **PC keyboard play**: keys `1`–`7` now play the corresponding lemon
  (Velxio frontend patch `0002-keyboard-shortcuts-for-labeled-pushbuttons`
  in the harness repo — digit keys dispatch press/release to the pushbutton
  labeled with that digit).
- **Game selector is now a slide switch** on D4 (left/GND = Mario, right/5V
  = Underworld, applied at RESTART) instead of a hold-while-restarting
  button. No firmware change needed — the active-low shim semantics already
  fit.
- **Cable layout reworked**: board signal wires terminate at vertical
  pull-up columns above each key; GND is a daisy-chained bus along the
  button legs and 5V a bus along the resistor tops — no cables cross the
  buttons. Harness gained net-aware input seeding (union-find over the wire
  graph) so headless verify still passes with the bus topology.
- **Audio timing** (earlier today, recorded here for completeness): Velxio's
  AVR frame loop ran the sim clock ~1.3× wall speed causing seconds of
  accumulating sound lag; fixed by harness patch
  `0001-avr-cycle-accurate-frame-pacing` (measured 1.00× after). The
  headless verify (secret code → WIN) stayed green through all of it.

## 2026-07-13 (later) — Emulation fixes: dead keys, endless beep, layout

Three user-reported bugs in the browser emulation, root-caused in the Velxio
frontend source and fixed on our side (no upstream patches):

- **Keys 1–6 dead / green LED never lit**: Velxio's SPICE→MCU *digital*
  injector only handles numerically-named pins, so `A0`–`A5` stayed at the
  emulator's default LOW (= pressed forever). Keys are now read via
  `analogRead` (the *analog* injector does cover A0–A5) against 10 kΩ
  pull-ups: idle ≈1023, pressed ≈0. `keyTouched()` is the single seam.
- **Continuous beep (survived even Stop)**: the buzzer part's WebAudio
  note-off fires only on a Timer2 duty→0 event; duty is polled only on PWM
  pins (3/5/6/9/10/11) and `noTone()` leaves OCR2A set — on D8 the first
  tone played forever. Emulation buzzer moved to **D11** and all sound goes
  through `emuTone()` (`tone` → `delay` → `noTone` → `OCR2A = 0`). Also
  added a boot guard: the game waits until inputs read idle-high (first
  SPICE solve) instead of spamming notes/restarts. Minimal repro:
  `velxio-multi-board-emulator/circuits/buzzer-test.yaml`.
- **Layout**: keys 1–7 in one horizontal row, pull-ups above their keys,
  panel/LEDs/buzzer in separate blocks (explicit x/y in the spec).

Hardware builds remain untouched (all changes inside `#ifdef
VELXIO_EMULATION`; all PlatformIO envs green). Headless verify still plays
game 2's code and asserts WIN.

## 2026-07-13 — Interactive browser emulation (Velxio)

The game is now playable in a browser with no hardware, via the
[velxio-multi-board-emulator](../velxio-multi-board-emulator/) pipeline
harness. See `emulation/README.md` for how to play and the full rationale.

- **New `emulation/` folder**: `lemon-piano.yaml` (circuit spec: Nano, 7
  clickable "lemon" buttons, panel buttons, feedback LEDs, pump-indicator
  LED, buzzer) and the generated `lemon-piano.vlx` (import into Velxio at
  `http://localhost:3080` and press Run).
- **`VELXIO_EMULATION` input shim in `firmware/src/main.cpp`**: the emulator
  cannot reproduce the analog lemon divider (its parts are digital
  active-low, ADC injection covers channels 0–5 only, and A6 has no digital
  pin), so the emulation build swaps ONLY the input layer — keys become
  active-low buttons on A0–A5 + D9, GAME_SELECT/RESTART become active-low.
  Game logic, audio and outputs untouched. Hardware envs still build the
  identical 7296-byte binary; the shim is compile-checked by the new
  `emulation` env in `platformio.ini` (`-DVELXIO_EMULATION`).
- **Headless gameplay regression test**: `velxio-pipeline run --mode verify`
  compiles the real firmware, boots it under avr8js, injects game 2's secret
  code (`3,6,1,4,2,5,3,6,1,4`) as timed key presses and asserts the serial
  log prints `WIN`, the buzzer pin toggles and the green LED flashes — all
  green (evidence in `emulation/runs/`, gitignored).

## 2026-07-12 — Firmware fixes & refactor (TODO #1–#12)

Applied all correctness, gameplay and code-quality items from `TODO.md`.
Behavior is now *fixed and improved* rather than a 1:1 translation. All three
PlatformIO envs still build green; **static RAM dropped 778 B → 309 B**
(38 % → 15 %) and flash is 7.3 KB (23 %).

- **Correctness:** clear `pressedNote` on victory (no more instant false-fail);
  defined `pumpOff()` boot state instead of `analogWrite(pin, HIGH)`;
  `selectGame()` resets `fails`/`currentStep`/`dead` so a post-death restart is
  clean; removed the bogus `pinMode(0..7, INPUT)` UART clobber; `buzz()` guards
  `frequency <= 0`; deleted the dead `if (game = 1)` block.
- **Gameplay:** LED feedback timed with `millis()` (`LED_FEEDBACK_MS`);
  **edge-triggered input** (`keyHeld[]` rising edge) — a held finger no longer
  advances the sequence repeatedly, and melodies with repeated consecutive
  notes are now playable. Victory/death/spray playback kept intentionally
  blocking.
- **Code quality:** all melodies moved to `PROGMEM`; the `*_cut` victory tunes
  de-duplicated into an offset into the full theme (`MARIO_VICTORY_FROM` /
  `UNDER_VICTORY_FROM`, lengths via `sizeof`); touch threshold auto-calibrated
  at boot (`calibrate()` → `baseline + TOUCH_MARGIN`), replacing the hardcoded
  `SENSITIVITY`.
- The clean game loop also fixes an off-by-one from the original: you now win
  on the 10th correct note instead of needing a phantom 11th press.

Open: TODO #13 (verify relay/pump polarity on the real board) and #14 (redraw
the full schematic) — both need the physical hardware.

## 2026-07-12 — Rescue & workspace setup

- **Rescued the 2019 originals verbatim** into git history (commit
  `rescue: original 2019 lemon piano files`) before touching anything.
- **Translated everything to English**: folder/file names, code comments, and
  identifiers (active firmware and archived references).
- **Restructured to a PlatformIO workspace**: `Piano_Limones_v4/` →
  `firmware/` (`src/main.cpp` + `include/notes.h` + `platformio.ini` with
  `nanoatmega328` / `nanoatmega328new` / `uno` envs); `Banana-Piano-Original/`
  → `archive/banana-piano-original/` (banana-piano, keyboard-test,
  game-prototype); schematics → `docs/`.
- **Reconstructed one corrupted line**: `Duracion` (now `NOTE_DURATION`) had
  stray keystrokes (`= 5çkp\`ñ´sca…0;`) and didn't compile — restored to
  `50` ms. No other behavior changes; 2019 bugs are preserved and catalogued
  in `TODO.md`.
- **Wrote the docs**: README (game rules, secret codes, lineage, quick start),
  `docs/HARDWARE.md` (deduced pin map + wiring + sensing physics), `TODO.md`
  (14-item fix roadmap), `CLAUDE.md`, `.gitignore`.
