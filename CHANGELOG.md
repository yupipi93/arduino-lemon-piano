# Changelog

Append-only log of significant changes. Newest first.

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
