# V2 emulation — not built (and why)

Every 2026 revision (V4, V4.5, V5) ships a
[Velxio](https://github.com/davidmonterocrespo24/velxio) browser emulation with a
headless `verify` regression test. **V2 has none**, and this is a deliberate gap,
not an oversight: hosting this board in the browser AVR (avr8js) would require
editing a 2019 sketch.

Three blockers, all in the sketch rather than the circuit:

1. **Buzzer pin.** Velxio's buzzer part starts a WebAudio note when Timer2 duty
   goes above 0 and stops it only on a duty→0 event, and duty is polled only on
   the PWM pins (3/5/6/9/10/11). On **D8** the first `tone()` would beep forever.
   The 2026 builds work around it with an `emuTone()` helper (`tone()` →
   `delay()` → `noTone()` → `OCR2A = 0`) on D11.
2. **Key 7.** avr8js exposes ADC injection on A0–A5 only; **A6 does not exist**
   in the browser board, so key 7 needs a digital pin and a `digitalRead()` path.
3. **Touch divider.** The 220 Ω pull-up + body divider cannot be reproduced in
   the canvas; the 2026 builds substitute pushbuttons + 10 kΩ pull-ups. (This one
   is nearly free here: the 2019 sensing is *already* inverted — idle high,
   touch pulls down — which is exactly what a pull-up + button does.)

So a V2 emulation is perfectly feasible, but it means adding a
`#ifdef VELXIO_EMULATION` shim to `keyboard-test.ino`. There is also little to
assert: this board has no game, so a `verify` run could only check that touching
a key makes the buzzer sing. That call is open in
[../../../TODO.md](../../../TODO.md); if you take it, follow the recipe in
[../../../docs/VERSIONING.md](../../../docs/VERSIONING.md) § step 5 and copy the
spec shape from [../../v5-led-bar/emulation/lemon-piano.yaml](../../v5-led-bar/emulation/lemon-piano.yaml).

Meanwhile the board is fully documented ([../HARDWARE.md](../HARDWARE.md)),
diagrammed ([../images/wiring-v2.png](../images/wiring-v2.png)) and its firmware
builds green with `pio run` in [../firmware](../firmware).
