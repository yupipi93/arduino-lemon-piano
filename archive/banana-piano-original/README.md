# Banana piano — original 2019 reference (frozen)

The code the lemon piano evolved from. **Frozen**: kept for reference, never
edited (comments/identifiers were translated to English during the 2026-07-12
rescue; the pristine Spanish files are in git history, commit
`rescue: original 2019 lemon piano files`).

| Folder | Was (2019) | What it is |
|---|---|---|
| `banana-piano/` | `Banana-Piano/Banana-Piano.ino` + `pitches.h` | The untitled.es tutorial base: 7 fruit keys on A0–A6 play fixed notes (C3–B3) on an Uno. Leftover commented-out HC-SR04 ultrasonic code. |
| `keyboard-test/` | `codigo/codigo (1).ino` | Keyboard-only test: same 7 fixed notes, 4-sample averaging, no game logic. |
| `game-prototype/` | `codigo/codigo.ino` | **v3-era game**, the direct ancestor of the V4 firmware: secret sequences, LEDs, game-select button, single relay. Uses the *inverted* threshold (`<= 1019`, player holds GND) — see `docs/HARDWARE.md` § threshold inversion. |

Schematics that lived here moved to [`../../docs/`](../../docs/):
`Esquema.PNG` → `images/keyboard-breadboard-nano.png`,
`esquema-original.png` → `images/banana-piano-original.png`,
`Esquema.fzz` → `keyboard-schematic.fzz`.

These sketches still open in the Arduino IDE (folder name matches the .ino).
They are not part of the PlatformIO build.
