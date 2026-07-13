# Changelog

Append-only log of significant changes. Newest first.

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
