# Board probe — v0.7.1 PCB

Not a game and not a version: a diagnostic firmware for the **assembled
v0.7.1 board**, for when it boots, lights its LEDs, and plays nothing.

```bash
cd versions/v5.5-power-filter/firmware/probe
pio run -t upload          # or -e nanoatmega328new for the new bootloader
pio device monitor         # 9600 baud
```

It answers three questions in one flash, in this order:

1. **Does D13 drive BUZ1?** It beeps for 400 ms at startup, before anything can
   go wrong. D13 is also the Nano's on-board `L` LED — so *L lit, no sound* is a
   transducer fault (wrong buzzer type, or something low-impedance on J5), and
   *neither* is D13 itself or a badly seated Nano.
2. **Does the +5 V rail reach the pull-ups?** The calibration block prints each
   channel's baseline. Near **1022** is healthy. Near 0 means R1–R7 or the rail
   are not there, and it says so.
3. **Do the keys move at all?** All seven channels at 10 Hz, raw value and delta
   from baseline, plus — every 10 s — the **largest excursion since boot** per
   channel. That last line is the one that settles it: if a minute of playing has
   never moved a channel more than 1–2 counts, no threshold will rescue it and
   the front end is the fault.

It also prints the two SENS buttons' state, which should read `up` with nothing
pressed. Permanently `DOWN` means SW1/SW2 are shorted to GND — see §4 of
[../../../../pcb/docs/REVIEW-v0.7.1-silent-keys.md](../../../../pcb/docs/REVIEW-v0.7.1-silent-keys.md).

**Why not `-DDEBUG_TOUCH` on the game build?** Because that prints only on an
*accepted* press, so it prints nothing at all when the fault is that no press is
ever accepted.

Each channel is read twice and the first conversion discarded. At 220 Ω that
changes nothing; it matters the moment the pull-ups are raised, which is the fix
the review proposes.
