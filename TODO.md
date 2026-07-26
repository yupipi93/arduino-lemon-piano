# TODO — fix roadmap

Firmware bugs found in the rescued 2019 code. **Items 1–12 are done** (applied
in the 2026-07-12 refactor to the firmware that is now
[V4](versions/v4-water-pump/), all envs still build green, RAM down from
778 B/38 % to 309 B/15 %). Items 13, 15, 16, 17 are open — 13 and 15 need the
physical V5 build.

Items apply to the newest board ([V5](versions/v5-led-bar/)) unless a version is
named. New hardware belongs in a new version directory, not in this list — see
[docs/VERSIONING.md](docs/VERSIONING.md).

## Correctness — ✅ done

- [x] **1. Post-victory false fail** — clearing `pressedNote = 0` on victory
  (plus the cleaner edge-triggered engine) stops the instant "fail" after a win.
- [x] **2. Relay init** — replaced `analogWrite(pin, HIGH)` with a defined
  `pumpOff()` (`RELAY_1 LOW`, `RELAY_2 HIGH`) set *before* `pinMode(OUTPUT)`.
- [x] **3. `fails` never reset on restart** — `selectGame()` now clears
  `fails`, `currentStep`, `dead` and the LEDs, so a restart after death is a
  clean slate.
- [x] **4. UART clobber + serial** — dropped the bogus `pinMode(0..7, INPUT)`
  block; all debug output goes through one `log()` helper guarded by
  `serialEnabled`, with `Serial.begin()` in `setup()`.
- [x] **5. `buzz()` divide-by-zero** — early `return` on `frequency <= 0`.
- [x] **6. Assignment-in-condition** — the dead `if (game = 1)` toggle block is
  deleted.

## Gameplay / robustness — ✅ done

- [x] **7. LED timeout** — now real-time via `millis()` (`LED_FEEDBACK_MS`),
  not a loop counter.
- [x] **8. Debounce / edge detection** — the sequence engine is edge-triggered
  (`keyHeld[]` rising-edge), so a held finger no longer machine-guns the
  sequence and **melodies with repeated consecutive notes are now playable**.
- [x] **9. Blocking melodies** — reviewed and **kept blocking on purpose**: the
  game is meant to pause while the victory/death tune or the spray plays, and
  idle key input stays responsive. Revisit only if the game grows a UI.

## Code quality — ✅ done

- [x] **10. Melodies in `PROGMEM`** — all tunes moved to flash; static RAM
  dropped to 309 B.
- [x] **11. De-duplicated `*_cut` arrays** — one copy of each theme, played
  from an offset (`MARIO_VICTORY_FROM` / `UNDER_VICTORY_FROM`); lengths derived
  with `sizeof` so they can't drift.
- [x] **12. Auto-calibrated touch** — `calibrate()` samples each key's floating
  baseline at boot and sets `threshold = baseline + TOUCH_MARGIN`, replacing
  the hardcoded 100/170-per-power-supply constant.

## Hardware / docs — open (needs the physical V5 build)

- [ ] **13. Wire and playtest the V5 board**: ten green LEDs on
  D2,D3,D4,D5,D6,D9,D10,D11,D12,D13 (220 Ω each), game-select SPDT on A7
  (5 V/GND), restart on D7. The Velxio emulation is the closest thing to a
  reference schematic today
  ([versions/v5-led-bar/emulation/lemon-piano.yaml](versions/v5-led-bar/emulation/lemon-piano.yaml)).
- [x] **14. Full wiring diagrams** — done, and now **one per hardware revision**:
  `versions/*/images/wiring-{v1,v2,v3,v4,v4.5,v5}.png`, generated from the
  declarative contracts in [tools/wiring_diagrams.py](tools/wiring_diagrams.py) on
  the wirewright auto-router + DRC (0 violations). A formal Fritzing/KiCad redraw
  is still optional.
- [ ] **15. Confirm A7 game-select biasing** on the real board (SPDT vs.
  switch + 10 kΩ pulldown); A7 has no internal pull-up.
- [ ] **16. Decide: emulate the 2019 boards (V1–V3)?** It needs a
  `#ifdef VELXIO_EMULATION` shim inside the 2019 sketches (buzzer onto a PWM pin
  with `OCR2A` cleared, key 7 off A6, pull-up buttons instead of the divider) —
  i.e. editing code whose value is being the untouched original. Blockers per
  board are written up in each `versions/v{1,2,3}-*/emulation/README.md`. If yes,
  start with V3: it has a real game to assert on.
- [ ] **17. V2 quirk:** `Serial.begin(9600)` is commented out while the
  `Serial.print` calls are live — decide whether to fix it (helps anyone reviving
  the rig) or leave it as 2019 shipped it.

## Nice-to-have (new ideas, not blocking)

- [ ] On-hardware playtest: confirm `TOUCH_MARGIN` feels right; tune if needed.
- [ ] LED flourish on win (e.g. chase/blink the full bar) before the theme, and
  on auto-advance.
- [ ] Optional per-note key LED feedback, or a "wrong" blink, if the plain
  blank-on-miss feels too subtle.

## Done in V5 (2026-07-14)

- [x] Removed relays / water pump / red LED / fail-counter / death tune.
- [x] Ten-green-LED progress bar; blank-all-on-wrong; win = all ten lit.
- [x] Auto-advance between the two games on win.
- [x] Game-select moved to A7 to free the 10th LED pin.
- [x] V5 Velxio emulation (10 LEDs) built + headless verify green.
