# Board probe — v0.7.1 PCB

Not a game and not a version: a diagnostic firmware for the **assembled
v0.7.1 board**, for when it boots, lights its LEDs, and plays nothing.

```bash
cd versions/v5.5-power-filter/firmware/probe
pio run -t upload          # or -e nanoatmega328new for the new bootloader
pio device monitor         # 9600 baud
```

> **2026-09-12, from the bench:** questions 1 and 2 below are **already
> answered** on the real board — every boot sound plays, and both SENS buttons
> beep when pressed. SENS− lives on A7 and only works through R18, a pull-up to
> `/+5V` in the same row as R1–R7, so that one press also proves **the 5 V rail
> reaches the key pull-ups**. Buzzer, buttons, LEDs and rail are all alive.
> Exactly one block is dead: the seven key channels. Question 3 is the whole
> job now, and it is the one this probe was written for.

It answers three questions in one flash, in this order:

1. **Does D13 drive BUZ1?** It beeps for 400 ms at startup, before anything can
   go wrong. D13 is also the Nano's on-board `L` LED — so *L lit, no sound* is a
   transducer fault (wrong buzzer type, or something low-impedance on J5), and
   *neither* is D13 itself or a badly seated Nano.
2. **Does the +5 V rail reach the pull-ups?** The calibration block prints each
   channel's baseline. Near **1022** is healthy. Near 0 means R1–R7 or the rail
   are not there, and it says so.
3. **Do the keys move at all, and in which direction?** All seven channels at
   10 Hz, raw value and signed delta, plus — every 10 s — the largest excursion
   since boot **in both directions** and a one-line verdict. Three outcomes,
   three different faults:

   | Verdict | Means |
   |---|---|
   | biggest movement **DOWN** | this board's polarity. The game can work; only the margin needs tuning. |
   | biggest movement **UP** | the rig is the V4/V4.5 front end (pin floating, resistor in series, player on +5 V). The game firmware cannot see it, and neither can smart adjust. |
   | **nothing moves** (≤ 2 counts) | no touch signal exists to threshold, whatever the margin says. |

   The probe is deliberately direction-agnostic because the game is not. Both
   front ends use 220 Ω — the value is the same on the breadboard and on the
   board — so "same resistors" does not mean "same circuit", and only the sign
   of the movement tells them apart.

**Run it on the breadboard too.** Same sketch, same Nano, same 9600 baud. Two
screenshots settle in a minute what no amount of reading the docs can: if the
working rig's numbers go UP and the board's do not move, the topologies differ
and that is the entire fault.

It also prints the two SENS buttons' state, which should read `up` with nothing
pressed. Permanently `DOWN` means SW1/SW2 are shorted to GND — see §4 of
[../../../../pcb/docs/REVIEW-v0.7.1-silent-keys.md](../../../../pcb/docs/REVIEW-v0.7.1-silent-keys.md).

**Why not `-DDEBUG_TOUCH` on the game build?** Because that prints only on an
*accepted* press, so it prints nothing at all when the fault is that no press is
ever accepted.

Each channel is read twice and the first conversion discarded. At 220 Ω that
changes nothing; it matters the moment the pull-ups are raised, which is the fix
the review proposes.
