# TODO — fix roadmap

The firmware is a 1:1 translation of the rescued 2019 code, **bugs
intentionally preserved**. This is the ordered backlog for the fix sessions.
Found during the 2026-07-12 rescue analysis; each item says what's wrong and
where.

## Correctness

1. **Post-victory false fail** — after the 10th correct note, `noteIndex`
   resets to 0 and the victory melody plays, but `pressedNote` still holds the
   last key; the very next check compares it against `sequence[0]`, lights the
   red LED and resets. You "fail" immediately after winning. Clear
   `pressedNote = 0` on victory. (`main.cpp`, victory block)
2. **Relay init uses `analogWrite(pin, HIGH)`** — `HIGH` is 1 ⇒ 1/255 PWM duty
   on D5/D6, not a clean off state; on an active-LOW relay module this
   chatters the pump at boot. Use `digitalWrite`, and set a safe state *before*
   `pinMode(..., OUTPUT)` attaches the pins. (`loop()`, `!started` block)
3. **`fails` never resets on restart** — after death (`game = 3`), pressing
   restart (D7) starts a new game but keeps `fails = 10`, so the first
   late-game mistake kills you again instantly. Reset `fails` in the restart
   path.
4. **`pinMode(0..1, INPUT)` clobbers the UART pins** and `sing()` calls
   `Serial.println()` without `Serial.begin()`. Drop the D0–D7 pinMode block
   (analog pins never needed it) and gate serial output behind the
   `serialEnabled` flag properly.
5. **`buzz(pin, 0, len)` divides by zero** for rest notes
   (`1000000 / frequency`). Works by accident on AVR (numCycles is 0); guard
   it explicitly.
6. **Assignment-instead-of-comparison** in the commented-out game-toggle block
   (`if (game = 1)`) — fix it or delete the dead block.

## Gameplay / robustness

7. **LED timeout is loop-count based** — `count >= 50` was commented as
   "5 seconds" but the loop runs in ~1 ms; LEDs go off after ~50 ms. Use
   `millis()`.
8. **No debounce / hysteresis on touch** — a touch retriggers `tone()` every
   loop, and the "different key advances the sequence" rule means sequences
   with repeated consecutive notes are unplayable (both 2019 codes avoid them,
   but the engine shouldn't require that). Track press/release edges.
9. **Blocking melodies** — `sing()` and the 1 s penalty freeze input for
   seconds. Fine for the original game; consider a non-blocking player if the
   game grows.

## Code quality

10. **Move melody arrays to `PROGMEM`** — the melody tables account for
    ~550 B of the 778 B static RAM in use (38% of the ATmega328's 2 KB).
    Fun fact from the ELF: GCC already dead-strips the *full* `melody`/
    `tempo`/`underworld_*` arrays because `sing(1)`/`sing(2)` are never
    called — only the `_cut` versions occupy RAM today.
11. **De-duplicate `*_cut` arrays** — the trimmed melodies are suffixes of the
    full ones; play the full array from an offset instead (decide together
    with #10, since the full arrays are currently unreachable code).
12. **Auto-calibrate `SENSITIVITY`** — sample the floating baseline at boot
    instead of hardcoding 100/170 per power supply.

## Hardware / docs

13. **Verify the relay/pump wiring** against the physical build and update
    [docs/HARDWARE.md](docs/HARDWARE.md) (⚠️ items).
14. **Redraw the full schematic** (Fritzing or KiCad) including LEDs, buttons,
    relays and pump — the surviving diagram covers only the keyboard stage.
