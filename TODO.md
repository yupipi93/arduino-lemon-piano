# TODO — fix roadmap

Firmware bugs found in the rescued 2019 code. **Items 1–12 are done** (applied
in the 2026-07-12 refactor, all three envs still build green, RAM down from
778 B/38 % to 309 B/15 %). Items 13–14 need the physical build and are open.

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

## Hardware / docs — open (needs the physical build)

- [ ] **13. Verify the relay/pump wiring** against the real board and confirm
  the polarity assumed by `pumpOff()`/`firePump()`; update the ⚠️ items in
  [docs/HARDWARE.md](docs/HARDWARE.md).
- [ ] **14. Redraw the full schematic** (Fritzing/KiCad) including LEDs,
  buttons, relays and pump — the surviving diagram covers only the keyboard
  stage.

## Nice-to-have (new ideas, not blocking)

- [ ] On-hardware playtest: confirm `TOUCH_MARGIN` and `LED_FEEDBACK_MS` feel
  right; tune if needed.
- [ ] Interruptible death tune (press RESTART to cut it short) if the replay-
  on-touch lockout feels long.
