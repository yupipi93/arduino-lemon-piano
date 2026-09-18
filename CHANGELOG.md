# Changelog

Append-only log of significant changes. Newest first.

<!-- ARCHIVE-DIGEST START -->
**Archived history**

50 older entries live in `docs/changelog/`, moved there verbatim. Run
`bin/qpc-changelog-archive <repo> --verify` to prove none of them changed.

[undated](docs/changelog/undated.md), 6 entries:

- theme, VU-meter colours in the emulator
- a more recognisable Castle theme
- the ending loops until reset
- themes, a real castle-clear ending
- not a mistake
- driving the browser, not by guessing

[2026-07](docs/changelog/2026-07.md), 44 entries:

- 2026-07-31 (pcb/) — rotatable 3D models in the pipeline
- 2026-07-31 (pcb/) — v0.6.0: J5, aux speaker output in parallel with the buzzer
- 2026-07-30 (pcb/) — v0.5.1: D1's missing 3D body (Kathode/Cathode filename split)
- 2026-07-30 (pcb/) — v0.5.0: Nano flipped (USB east), keys north, LEDs south, filter refactored
- 2026-07-30 (pcb/) — v0.4.0: 120 × 40 mm remake, centred Nano, external-button headers
- 2026-07-30 (pcb/) — v0.3.0: 4 anchor holes, VU-meter LEDs, render fixes
- 2026-07-30 (pcb/) — v0.2.1: silk label pass + render naming
- 2026-07-30 (pcb/) — v0.2.0: Nano-orientation erratum fixed + full render suite
- 2026-07-30 (pcb/) — Fabricable V5.5 PCB, release v0.1.0
- 2026-07-29 (V5.5) — New version: filtered 5 V supply
- 2026-07-29 (V5 LEDs) — Progressive 10-LED fill for calibration and the win
- 2026-07-29 (V5 audio, part 4) — Faster autoplayer, Castle closes the game,
- 2026-07-29 (V5 audio, fix) — The ending loop actually loops in the emulator now
- 2026-07-29 (V5 audio, part 3) — Stuck-key cue, Castle replaces Underwater,
- 2026-07-29 (V5 audio, part 2) — Level-start announce, full Underwater/Starman
- 2026-07-29 (V5 audio) — Mistake cue is now Mario's death rattle; win order fixed
- 2026-07-29 (V5 autoplayer, v5) — Manual clicks came back dead; a real short,
- 2026-07-29 (V5 emulation) — A "virtual button" that plays every level for you
- 2026-07-29 (V5 autoplayer, v2-v4) — Three real bugs, found by actually
- 2026-07-29 (V5 audio timing) — Fixed the sound race; silences are now policy
- 2026-07-29 (V5) — Four levels, and every sound is Mario's
- 2026-07-28 (V5.5) — Drawn proposal: sensitivity on a KY-040 rotary encoder
- 2026-07-28 (V5 rebuilt) — GND-clip keyboard, live sensitivity, no restart/select
- 2026-07-27 (V2.5) — New version: keyboard test with a LIVE touch threshold
- 2026-07-27 (V5 input) — One key at a time; and the measurement that says the pull-downs are mandatory
- 2026-07-27 (V5 touch) — Notes sustain while held; a key only ever counts once
- 2026-07-27 (V5) — Piano first, puzzle second: free play + a wrong tone that waits its turn
- 2026-07-26 (V0 field result) — The bad "buzzer" was a bad Arduino
- 2026-07-26 (V0) — New buzzer bring-up board: a scale, forever
- 2026-07-26 (V4.5) — V4+ renamed to V4.5, and its relay pair + water pump removed
- 2026-07-26 (restructure) — One version per hardware revision, none archived
- 2026-07-26 — Wiring engine extracted to its own repo (eda-wirewright)
- 2026-07-25 (engine) — Real schematic engine: auto-router + DRC (no more overlaps)
- 2026-07-25 (diagrams) — Fix overlapping wires in the wiring diagrams
- 2026-07-25 — V4 touch upgrade (in-place) + manual MARGIN buttons + wiring diagrams
- 2026-07-14 (calibration) — Noise-adaptive touch margin, recalibrate on RESTART
- 2026-07-14 (light show) — LED bar flashes to the victory theme
- 2026-07-14 — V5: ten-LED progress bar, auto-advancing games
- 2026-07-13 (routing) — Hard wire-routing rules in the pipeline
- 2026-07-13 (final) — Emulation polish: keyboard play, switch, clean wiring
- 2026-07-13 (later) — Emulation fixes: dead keys, endless beep, layout
- 2026-07-13 — Interactive browser emulation (Velxio)
- 2026-07-12 — Firmware fixes & refactor (TODO #1–#12)
- 2026-07-12 — Rescue & workspace setup

<!-- ARCHIVE-DIGEST END -->

---

<!-- ARCHIVE-DIGEST START -->
**Archived history**

18 older entries live in `docs/changelog/`, moved there verbatim. Run
`bin/qpc-changelog-archive <repo> --verify` to prove none of them changed.

[2026-09](docs/changelog/2026-09.md), 17 entries:

- 2026-09-13 — the SENS − button played a note: A7 is on the key multiplexer
- 2026-09-13 — quiet-rest control: 1 MΩ does not ghost, and the game is loaded
- 2026-09-13 — SOLVED: R1 at 1 MΩ, 15 clean presses, no ground clip
- 2026-09-13 (night) — `readKey()` discards its first conversion, with a test that proves it
- 2026-09-13 (late) — R1 → 10 kΩ, twice: a floating node, then the real number
- 2026-09-13 (night) — a log that cannot say what it measured is not evidence
- 2026-09-13 (evening) — the breadboard, measured: it works by two counts
- 2026-09-13 (later) — the board is good, and a wire proved it in thirty seconds
- 2026-09-13 — the probe records to a file at last, and the board's answer
- 2026-09-12 (fix) — the probe flashed fine and printed to nobody
- 2026-09-12 (later) — the bench answers back: the probe learns to look both ways
- 2026-09-12 — V5.5 wiring diagram: every part labelled with its PCB v0.7.1 reference designator
- 2026-09-12 — v0.7.1 assembled and silent: review, board probe, doc correction
- 2026-09-09 — V5.5 firmware: FREE PLAY and the MODE WHEEL (no hardware change)
- 2026-09-06 — PCB v0.7.1: "Created with ♥ by Multitec." maker's mark (cosmetic)
- 2026-09-06 — PCB v0.7.0: Nano turned back to USB-WEST for enclosure cable access (LEDs north, keys south)
- 2026-09-06 — V5.5: add the amplified speaker that was missing from the diagram

[2026-08](docs/changelog/2026-08.md), 1 entries:

- 2026-08-05 — V6 proposal: battery power + amplified speaker (diagram only)

<!-- ARCHIVE-DIGEST END -->

---

## 2026-09-15 (late, take two) — every theme at three sizes

*"Creo que no me he expresado bien. Hay tres versiones de cada canción. La
corta, que es para el menú. Una de siete notas y únicamente de las siete notas
que se tocan. Así es como tiene que ser la canción cuando empieza o cuando el
usuario toca cinco veces la nota. Y luego la versión ultra completa, full, que
es ya cuando el nivel es completado y como celebración."*

The entry above this one made the level announcement the **whole** theme. That
was half of what he meant and the wrong half: a 23-second Castle at the start of
every level is not an announcement, it is a wait. Corrected here — and the
result is better than either version, because the three sizes each have a job.

| Size | Where | What it is |
|---|---|---|
| **SHORT** | the mode wheel | the first `MENU_PREVIEW_NOTES` (8) entries of the raw theme. Browsing four modes has to be fast above everything else. Unchanged |
| **PLAYABLE** | the level announcement, and the five-tap reminder | **seven notes, and only notes this level's lemons can make** |
| **FULL** | the celebration when a level is cleared | the whole piece, 10–23 s |

### The PLAYABLE cut is the interesting one

The announcement **is the clue** — the ten-note code is hidden in the theme — so
it must not be full of notes the keyboard cannot make. Every theme reaches
pitches that are on no lemon: the Overworld goes to AS6, F7 and D7, none of
which is one of level 1's seven keys.

So `playPlayableIntro()` walks the theme and plays only what the level's row
contains, stopping at seven notes. A pitch outside the row is **not skipped** —
it is played as silence of the same length, so the rhythm of the hook survives,
which is most of what makes a hook recognisable. It is a rule rather than four
hand-written excerpts, it costs no flash (the note data is the theme that is
already there), and it lands on each theme's actual hook, which is a pleasant
accident of Mario having written them that way:

| Level | Theme | The seven notes it announces | Time |
|---|---|---|---|
| 1 | Overworld | `E7 E7 E7 C7 E7 G7 G6` — the riff, exactly | 2.0 s |
| 2 | Underworld | `C4 C5 A3 A4 AS3 AS4 C4` | 2.0 s |
| 3 | Starman | `C6 F5 F5 D5 F5 F5 D5` | 1.4 s |
| 4 | Castle | `G3 D4 G3 D4 AS3 D4 AS3` — the alternating pedal | 2.0 s |

### …which freed the celebration to be the whole piece

A win used to play the theme's **tail**, from `*_VICTORY_FROM`, for a reason
that has now evaporated: the level announcement was already playing its head,
and hearing the whole thing twice in a row would have been too much. The
announcement is seven notes now, so `playVictory()` plays from the top —
Overworld 12.2 s, Underworld 13.1 s, Starman 10.4 s, Castle 22.7 s, with the bar
counting up from empty to all ten across the whole thing. The four
`*_VICTORY_FROM` constants are gone with it.

### Evidence

- **All five envs build.** 17 998 B flash / 58.6 %, 558 B RAM / 27.2 %.
- **Host tests: 107 + 174 checks, 0 failed.** New test 26 pins all three sizes,
  and it can see *which* notes the announcement played because
  `playPlayableIntro()` now prints them (`theme: 2637 2637 …`) — a theme is
  bit-banged through `buzz()` rather than `tone()`, so a log line is the only
  window onto it. The test asserts seven notes on every level, **every one of
  them on that level's keyboard**, and that the celebration is many times longer
  than the clue.
- **Flashed and verified**, and confirmed from the board's own serial:

```
  5.4s  Level 1
  7.6s  theme: 2637 2637 2637 2093 2637 3136 1568
```

  That is E7 E7 E7 C7 E7 G7 G6 — seven notes, 2.2 s from the level being
  announced to the tune being over, every note playable on a lemon.

## 2026-09-15 (late) — a level announces itself with its WHOLE theme

*"Tienes que hacer que suene completa la melodía. No solo las primeras notas.
En cambio, cuando estamos usando el menú de selección de nivel, sigue teniendo
que tocar la versión reducida para que no se haga tan largo."*

The reason is the game itself: **the ten-note code is hidden in the theme**, and
the codes draw on the whole piece rather than its opening bar. An announcement
that stopped after twelve notes was showing the player a twelfth of the clue —
and the five-tap reminder, which calls the same function, was handing out that
same twelfth when someone asked to hear it again.

`playLevelIntro()` now plays `*_LEN` instead of `*_INTRO_LEN`. Both callers get
it: the level start (boot, a win, accepting from the wheel, coming back from
free play) and the five-tap reminder. Free play is unchanged — its announcement
was always the whole scale.

**What it costs, measured rather than guessed** (playSong's own pacing: a note,
then 1.3× its length as a gap):

| Level | Theme | Notes | Was | Now |
|---|---|---|---|---|
| 1 | Overworld | 78 | 1.8 s | **12.2 s** |
| 2 | Underworld | 56 | 1.8 s | **13.1 s** |
| 3 | Starman | 48 | 2.1 s | **10.4 s** |
| 4 | Castle | 72 | 4.9 s | **22.7 s** |

So a win now costs the victory tail, the flagpole fanfare and then up to 23
seconds of the next level's theme. That is the trade he asked for, and it is
worth having written down next to the numbers.

**The wheel did not change.** Browsing modes still previews at most
`MENU_PREVIEW_NOTES` (8) notes, because a wheel you turn five times cannot spend
two minutes on it — which is exactly the second half of what he asked for. The
four `*_INTRO_LEN` constants now exist for that one caller and nothing else,
and say so.

### Evidence

- **All five envs build.** Unchanged at 17 806 B flash / 58.0 %, 562 B RAM /
  27.4 % — the lengths were already constants, so this costs nothing.
- **Host tests: 107 + 162 checks, 0 failed.** Test 25's edge-for-edge assertion
  is what covers this without a single change: it compares the five-tap replay
  against the boot announcement through `pinWrites[BUZZER]`, so "the reminder
  plays what the level opened with" stays true whichever length that is.
- **Flashed and verified**: `/dev/ttyUSB0`, 17 806 bytes written and verified.
  Boot log timed over serial: banner at 1.5 s, seven baselines by 5.1 s, margin
  30, `Level 1` at 5.4 s — and then twelve seconds of Overworld.

## 2026-09-15 (later) — the bank says `ICP`, and the fault flipped from too little current to too much

Natalia took the trigger to the bench and came back with two facts that move the
diagnosis further than any amount of reasoning about it would have:

1. **The trigger alone keeps the UGREEN Nexode's output up.** A few mA, far
   under any low-load threshold, and it does not drop. That closes the open
   question from this morning's doc — the decoy really does hold the port open at
   5 V — and it **removes the low-load timeout from the list of suspects
   entirely**. The bleeder resistor may never be needed.
2. **Attach the PCB and the bank shows `ICP` and cuts.** A code on the screen is
   a *protection trip*, and a trip means too much current, not too little. The
   fault we were chasing has inverted.

So the doc grew the branch that actually matters now, and it leads with the
cheapest cause: **reversed polarity**. `D1`'s cathode is on `/VIN` and its anode
on `/GND` (`pcb/docs/NETLIST.md` nets 2 and 3), so a swapped pigtail forward-biases
the TVS into a 0.7 V diode straight across the bank — a short, instantaneous and
repeatable, which is exactly the shape of the symptom. `D2` keeps it away from
the Nano, so the board fails safe and the bank's code is the alarm. Three
unpowered checks (which screw is `+`, a frayed strand across the terminal, Ω
across `J1` — *with the warning that the reversed-probe reading is `D1` conducting
and not a fault*), then a diode test on `D1` itself before trusting it again.

If polarity is clean it is the inrush into `C1 ‖ C3` = 940 µF, and the first
attempt costs nothing: **land the screw terminal before plugging the USB-C in**,
so the bank ramps VBUS into the capacitance on its own soft-start instead of
being hot-plugged onto a discharged 940 µF. Only then C1 470 → 220 µF.

`ICP` itself is left honest: UGREEN does not publish the expansion and searching
found only their generic "unusual code = protection mode" FAQ. The doc reads it
as a current protection and says it is not verified, because the actionable half
— trip, not timeout — does not depend on the acronym.

## 2026-09-15 (late) — the power bank switches itself off, and the PD trigger is only half the answer

Natalia asked for the DIP code that gets 5 V out of the PD/QC trigger module in
the inventory (`mod-pd-trigger-dip100w`), so the V5.5 board can run off a power
bank instead of a charger — the bank keeps cutting out because the piano does
not draw enough.

**The code is `1=OFF, 2=OFF, 3=OFF`.** Full table (1=ON, 0=OFF): `000`→5 V,
`111`→9 V, `110`→12 V, `100`→15 V, `101`→20 V. The module never steps up, so a
wrong code fails safe downwards to 5 V.

But the premise deserved a second look, and the new
[docs/POWER-FROM-A-POWER-BANK.md](docs/POWER-FROM-A-POWER-BANK.md) says so in
full:

- **A trigger set to 5 V does not raise the current draw**, and the draw is what
  the bank is measuring. The shutdown is a low-load timeout, not a voltage
  problem.
- **5 V is the one setting where a decoy may do nothing at all** — a USB-C
  source offers 5 V by default, so the board may take vSafe5V without ever
  sending an explicit Request, leaving the bank exactly as unaware of the piano
  as a bare two-wire pigtail does. Not documented by the seller, not verified
  here, and the KWS-X1's protocol page answers it in one look.
- **Three different faults look the same from the couch** and want different
  fixes: the low-load timeout (raise the draw), inrush into the 940 µF of
  `C1 ‖ C3` (drop C1 to 220 µF), and no PD contract at all (needs a C-to-C
  path). Which one it is follows from *when* the output drops.
- **`D1` makes a wrong DIP code expensive.** The P6KE6.8A is a 600 W part for
  1 ms, not a shunt: a 9 V contract from a 100 W bank puts it into continuous
  avalanche, and the ATmega inherits the 9 V when it fails. Hence the rule the
  doc leads with — measure the screw terminal with the Fluke *before* the wires
  reach `J1`.
- **The keep-alive, if it is still needed**, is a plain resistor across the
  trigger's screw terminal: 150 Ω (33 mA) to start, 2 × 220 Ω in parallel
  (45 mA) if the meter says so, never a single 100 Ω on a ¼ W part. On the
  unfiltered side, so it adds no drop across `L1` and cannot move AVcc. The
  bank's own low-current mode is tried first — it costs nothing.

Nothing on the board changes, so this is not a new version: it is a document,
linked from V5.5's powering rules and from V6's open-risk table. The current
figures in it are computed from the schematic, not measured — the measurement
protocol is step 1 of the doc, and it is what turns the table honest.

## 2026-09-15 (night) — "play me that tune again" moves off two lemons and onto one

*"Creo que es físicamente imposible detectar correctamente la pulsación de
varias teclas a la vez. Por lo cual vamos a cambiar el paradigma."*

The theme reminder shipped an hour ago as **hold the two END lemons together for
two seconds**. It was a good gesture built on the one thing this keyboard cannot
do — the same shared return path that makes the chord hard (every lemon's
current goes home through the player's body and the GND clip, so two fingers are
in each other's way and each dips shallower than one). Sergio called it, and he
is right: a gesture must not be built on the input the hardware is worst at.

**So it is built out of the input this piano has read perfectly since 2019: one
lemon, pressed and released.** Five presses of the same lemon in a row and the
level plays its theme again. Nothing simultaneous, nothing to disambiguate, no
threshold that is not already load-bearing.

### How it behaves

- **The note you just played is press one.** In practice it is four more taps on
  the lemon already under your finger — which is what he described: *"pulsa
  cuatro veces cualquier nota… pulsa, suelta y pulsa de nuevo hasta cinco
  veces"*.
- **It changes nothing about the game**, which is the part he asked for in as
  many words (*"el juego continúa por donde estaba"*). Presses two to five are
  repeats, which already score nothing and cost nothing; the first is an
  ordinary press, because the player chose to touch a lemon. `playThemeReminder()`
  does not touch `currentStep`, and `restoreIdleDisplay()` puts the same count of
  LEDs back on the bar.
- **There is an obvious free move**: drum the lemon the game has *just accepted*.
  Then all five are repeats and the hint costs nothing at all. Deliberately not
  enforced — no snapshot, no undo, no rewinding a wrong note — because a gesture
  that gives you your progress back after a mistake is a gesture that trivialises
  the game. The rule is "asking changes nothing", not "asking un-does things".
- **The keys are dead while it plays**, because `playLevelIntro()` blocks
  (*"se desactivan las teclas temporalmente"*), and the lemon still under the
  finger has to be let go before it counts as anything again — capped at 4 s so
  a channel that never reads clear cannot hang the piano.
- **A pause resets the count.** More than `THEME_REPEAT_GAP_MS` (1.5 s) between
  two presses and it starts over, so drumming is a decision and a lemon poked
  twice a minute apart never adds up to a request.

**Game only, and that is not an omission.** Free play exists precisely so the
same lemon can be played over and over — the mode's whole point, and the one
rule the game has that an instrument must not. Counting repeats there would take
the mode away to give it a hint it has no use for. The test asserts it: eight
taps on one lemon in free play are eight notes and not one byte of a theme.

### What came back

**The widest chord.** With the gesture off the two end lemons, **lemon 1 +
lemon 7 is available again** in free play — the widest interval this keyboard
has, lost for about an hour.

And the code got smaller rather than bigger: `bothEndsDown()`,
`serviceThemeReminder()`, the `stepBeforePress`/`countedBeforePress` snapshot,
the arming meter's trip outside the buttons-only region, and two constants are
all gone, replaced by three variables and an eleven-line counter in the press
branch. **17 806 B** against the 18 050 B of the version it replaces.

### Evidence

- **All five envs build.** `nanoatmega328` 17 806 B flash / 58.0 %, 562 B RAM /
  27.4 %.
- **Host tests: 107 + 162 checks, 0 failed.** Test 25 rewritten around the new
  gesture, and it pins the awkward cases as well as the happy one: four taps are
  not five; a different lemon in the middle starts the run over and the fifth of
  the *new* run is what fires; a 2.5 s pause breaks it; free play counts nothing.
  The replay is still asserted **edge for edge** against the boot announcement
  via `FakeBoard::pinWrites[BUZZER]` — not "a tune played", *that* tune.
- **Flashed and verified**: `/dev/ttyUSB0`, 17 806 bytes written and verified;
  boots, calibrates (margin 34 on a noisy bench this time) and announces
  `Level 1`.

## 2026-09-15 (evening) — the wave is the whole display, and the theme can be asked for

Two more from Sergio, straight after playing the last build.

### 1. Free play's bar only ever MOVES now

*"Quita el estático. El estático de que cuando pulsas la tecla 1 se enciende el
primer LED y cuando tocas la 7 se encienden todos. Solo quiero que salga la
onda, el barrido… están solapados."*

The pitch meter and the wave were both drawing on the same ten LEDs, one after
the other, and they read as one confused thing: a wave that runs and then a
block of light that stays. The static half is gone. In free play the bar is now
**dark, a wave, dark again** — the wave is the only thing a played note ever
draws, and `playKeyWave()` ends with `allLedsOff()` so the pond is left as it
was found.

The two places a single note could still leave a standing light went with it:
when a chord narrows to one voice (either way round), the bar goes dark rather
than falling back to the pitch bar. `showPitchBar()` survives for exactly one
caller — the scale that announces the mode, where the bar climbing *is* the
animation.

Worth noting what is deliberately kept: the chord's **two lone LEDs**. That is
the one standing shape left in free play, and it is the one worth keeping still
— it is the only way to see that two notes are sounding.

### 2. Both END lemons, held two seconds, replay the level's theme

*"Si el usuario toca la tecla 1 y la tecla 7 a la vez durante 2 segundos, suene
de nuevo la musiquita de ese nivel. Para que el usuario pueda recordar cómo
era."*

The theme **is** the clue — the ten-note code is hidden in it — so a player who
half-remembers it had to reset the board or win the level to hear it again. Now
they can ask. Hold lemon 1 and lemon 7 together: the bar becomes a charge meter
with a rising chirp (the same language as the two button holds), and at two
seconds `playLevelIntro()` runs again. In free play that replays the mode's own
announcement, sweep and scale.

**The two ends, because they are the one pair nobody plays by accident** — as
far apart as this keyboard goes, and no melody here asks for both at once.

**And it must not cost anything to ask**, which is most of the work:

- It is checked **before the input layer turns a finger into a guess**
  (`serviceThemeReminder()` returns true and owns the loop), so when both land
  in the same scan neither lemon sounds and neither is scored.
- When the second lemon arrives *late*, the first has already been scored — so
  the gesture **undoes it**, putting `currentStep` and `lastCountedKey` back to
  what that press found. Only a press of lemon 1 or lemon 7 arms that snapshot;
  any other lemon sets it to −1, or a stale snapshot would roll the game
  backwards on the next arming. Same trick the buttons already use for the
  sensitivity knob (`marginBeforePlus`).
- Letting go early cancels with the same bump every other held gesture uses, and
  **leaves the undo in place**: touching both ends is never a move.

**It had to be made shadow-proof, and the host test said so before the fruit
could.** The first version asked "are both ends over `touchMargin`?" — which, on
a rig where one finger's shadow is bigger than the margin, is true *every time
anybody plays anything*. Ten existing tests went red at once: the piano stopped
on every note to offer the theme. The rest of the firmware survives such a rig
because `strongestKey()` only ever takes the deepest channel, so `bothEndsDown()`
now does the same sort of thing — the two ends must also be the **two deepest**,
judged by the same ratio the chord uses. A middle lemon dipping comparably means
a hand on the fruit, not this gesture.

**What it costs:** lemon 1 + lemon 7 is no longer available as a chord in free
play. It is the widest interval the keyboard has, and it is the price of putting
the gesture on the pair nobody hits by accident. Said out loud here and in the
version README rather than left to be discovered.

### The fake board learned to hear a bit-banged tune

Asserting "it replayed the level's own music" was impossible until now: themes
go through `playSong()` → `buzz()`, which **toggles the buzzer pin by hand**
rather than calling `tone()` (that is what the 2019 code did, and what the piezo
is wired for), so `board.tones` never saw a single note of one. `FakeBoard` now
counts raw writes per pin, and that turns out to be a sharper instrument than
expected: the buzzer's edge count straight after boot **is** one playing of the
level-1 intro, so the test asserts the reminder produced **the same number
again, edge for edge**. Not "a tune played" — *that* tune.

### Evidence

- **All five envs build.** `nanoatmega328` 18 050 B flash / 58.8 %, 567 B RAM /
  27.7 %.
- **Host tests: 107 + 161 checks, 0 failed**, up from 107 + 145. Test 7 rewritten
  (the free-play bar has no standing state at all now), test 25 added.
- **Flashed and verified**: `/dev/ttyUSB0`, 18 050 bytes written and verified;
  the board boots, calibrates (margin 18 on a quiet bench) and announces
  `Level 1`.
- **Still needs fingers on fruit**: whether two seconds is the right hold, and
  whether the chord comes now that gate 1 is fixed. Both are single constants,
  and `-DDEBUG_TOUCH` prints the dips for the second one.

## 2026-09-15 (later) — the levels play in their own key, and the light moves where it should

Four reports from Sergio after playing the morning's build, all in
`versions/v5.5-power-filter/firmware/`.

### 1. Level 4's lemons were in the wrong key — and so were level 3's and one of level 2's

*"En el nivel 4 suena la melodía, pero cuando yo toco las notas no están en la
tonalidad correcta de ese nivel. Revisa todos los niveles y las melodías."*

He is right, and it was a design gap rather than a bug. A level announces itself
by playing the first bars of its theme and then hands the player seven lemons,
so those seven have to belong to the same piece of music. **Levels 1 and 2 were
built that way in 2019** — their rows are notes lifted straight out of the
Overworld and the Underworld, which is why playing them sounds like the tune
that just played. **Levels 3 and 4 were given plain C major runs.** On level 4
that is audible from across the room: the Castle theme is in **G minor around
G3-G4**, and the lemons answered it a C major scale an octave above.

| Level | Theme | The seven lemons, low to high |
|---|---|---|
| 1 | Overworld | E6 G6 A6 B6 C7 E7 G7 — unchanged |
| 2 | Underworld | A3 AS3 C4 **D4** A4 AS4 C5 |
| 3 | Starman | C5 D5 E5 F5 G5 A5 **C6** |
| 4 | Castle | **G3 AS3 C4 D4 DS4 F4 G4** |

- **Level 4** was rebuilt in the theme's own key and octave: G natural minor,
  every note one the melody hammers (G3 and D4 are its alternating pedal, AS3
  opens the second bar, and C4/DS4/F4/G4 are where the chromatic answer lands).
- **Level 3** lost a C# that appears nowhere in the Starman theme and gained the
  **C6** the melody opens every phrase on. The theme uses exactly eight pitches;
  the row is seven of them, dropping the B5 that turns up once in the tail.
- **Level 2 was not on his list and turned out to be wrong too.** Its top lemon
  played **D5, a note the Underworld never plays** — the tune's highest note is
  C5. That was inherited from 2019. It is the D4 the melody does play now, moved
  up the row so the keyboard still ascends left to right and the pitch bar keeps
  telling the truth; the three octave pairs the theme alternates (A3/A4, AS3/AS4,
  C4/C5) are untouched.

The fourth case was found because the test was written as a **rule**, not as
four assertions: *every lemon on every level must be a note that level's theme
actually plays*. It is `test_every_level_plays_in_its_own_key`, and it also
checks the seven are distinct (two lemons sharing a note would be
indistinguishable to the guess comparison).

**THE CODES DID NOT CHANGE.** They are written down as key NUMBERS — the
organiser's booklet prints them on paper, in a folder someone carries to an
event — so retuning the keyboard means recomputing the frequencies the sequences
store and leaving the numbers alone. Proven, not asserted:

```
$ python3 docs/cartilla/seqs.py
cross-check against the host test, level 1: OK
level 1: 6  5  6  7  2  5  2  1  3  4      level 3: 2  4  6  1  5  3  7  4  2  6
level 2: 3  6  1  4  2  5  3  6  1  4      level 4: 5  1  3  7  2  6  4  1  5  3
```

### 2. A repeated lemon: the whole bar, held, instead of a sweep

*"Se encienden todos los LEDs a la vez y cuando la suelta se apagan."* The
backwards sweep shipped on 2026-09-13 and lasted two days, for a good reason:
motion is now what free play's wave uses, on the same ten LEDs, and two moving
cues on one bar is one too many.

It is a better shape anyway. Ten lit LEDs is the one thing this bar does that is
**not a count of something** — a score of ten is a win, and a win never leaves
you still holding the lemon. It lasts exactly as long as the finger does, so it
reads as "this is about what you are doing right now" rather than as an event
that has already happened, and it still ends on the identical bar it started
from, which was always the message. `showRepeatSweep()` and `REPEAT_SWEEP_MS`
are gone; `showRepeatHold()` and one `repeatBarOn` flag replace them, and the
release path now restores the bar for **both** modes through one call to
`restoreIdleDisplay()` instead of two special cases.

### 3. Free play: every note breaks a wave, from the lemon that made it

*"Si toca la tecla 1, el efecto tiene que ser de la tecla 1 hasta la 7. Si toca
la 7, de la 7 a la 1. Pero si toca una intermedia, por ejemplo la 4, tiene que
ser de la 4 hacia ambos lados. Como un efecto ola hacia ambos lados."*

That is one shape with seven readings of it, so `playKeyWave()` is one loop and
no special cases: a wavefront expands from the note's own place on the bar, one
LED further out each frame, until it runs out of bar. The end keys have only one
side to travel down — which is exactly the run across the whole strip he
described — and the middle keys give two lights parting. The wave from key 4 is
shorter than the wave from key 1, deliberately: the light stops when it runs out
of pond. The note is already sounding underneath it, and the bar settles on the
pitch meter afterwards, which is the part he said he liked.

Asserted from the LED **film** rather than the final pin states
(`test_free_play_wave_starts_at_the_key`), because by the time the note settles
the wave has been and gone: key 1's film contains `#.........` before
`.........#`, key 7's contains them in the opposite order, and key 4's contains
`...#.#....` and then `#........#` — a symmetry no end key can produce.

### 4. The chord barely came, and the reason is the return path

*"No ha funcionado especialmente bien la doble pulsación... puede ser que sea
una limitación de hardware. Piénsalo."*

Thought about, and it is a fact about this keyboard rather than a limitation of
it. Every lemon's pull-up current goes home through **one shared element**: the
player's body and the GND clip in their other hand. With one finger down, all of
a key's 220 Ω current flows through that single path. With **two** fingers down,
the two channels are pulling the *same* body node up together — so each of them
sits **shallower** than a single touch would. The fingers are in each other's way.

Gate 1 asked the second finger for a dip of `touchMargin + 2`, i.e. **deeper
than a lone finger has to manage**. That is the one thing two fingers cannot do.
It is now just "is this a touch at all" (`>= touchMargin`), and the
discrimination is left entirely to gate 2, the ratio — which the shared path
does not affect at all, because both dips shrink *together* and their ratio
stays near 1 while a shadow's stays low. That is why the ratio is the gate that
does the work.

Gate 2 was lowered with it (70 → 60 %) and put straight back, because the host
test showed what that costs: with a shadow modelled at 42 % of a finger, a 60 %
gate has almost no room under it and a 45 % *hold* ratio would not let a chord
go at all — the released lemon stays shadowed by the one still down, so the
chord never ended. 70/55 sits squarely between "a shadow" and "another finger".

**And the guessing has an end now.** `-DDEBUG_TOUCH` prints
`dips margin=N 1:.. 2:.. … 7:..` four times a second while any lemon is held:

```
pio run -e nanoatmega328-debug -t upload && pio device monitor
```

Press one lemon and read a column; press two and read two. The ratio between
them **is** `CHORD_MIN_RATIO_PCT`, measured rather than argued about — TODO 21.
If the numbers say two fingers genuinely cannot both clear `touchMargin` on this
fruit, that is the hardware limitation he suspected, and the answer is a lower
margin or a better clip contact, not a looser gate.

### Evidence

- **All five envs build.** `nanoatmega328` 17 384 B flash / 56.6 %, 555 B RAM /
  27.1 %; `nanoatmega328-debug` 17 738 B (the dip dump); emulation builds 12 480
  and 12 492 B.
- **Host tests: 107 + 145 checks, 0 failed**, up from 107 + 122. Three new
  scenarios (22 levels-in-key, 23 level 4 chosen and level 3 won, 24 the wave),
  and test 18 rewritten around the held bar.
  - 23 wins **level 3** end to end rather than level 4 on purpose: clearing
    level 4 clears the last level, which drops into `playEndingLoop()` — a piece
    that only stops for a button gesture and therefore never returns on a fake
    board. Level 4 is proven to nine notes of ten plus the tenth lemon's pitch,
    which is the whole code with no infinite loop.
- **`docs/cartilla/seqs.py` exits 0** and prints the four codes unchanged, which
  is what says the printed booklet is still correct.
- **Flashed and verified**: `/dev/ttyUSB0`, env `nanoatmega328`, 17 384 bytes
  written and verified by avrdude; the board boots, calibrates seven baselines
  and announces `Level 1`.
- **Still needs fingers on fruit**: whether the chord now comes reliably, and
  whether `FREE_WAVE_STEP_MS` (14 ms/frame, so 140 ms for an end key) makes fast
  playing feel syrupy. Both are single constants.

## 2026-09-15 — free play opens with a sweep, and two lemons sound as two notes

Two asks from Sergio, both about FREE PLAY, both in
`versions/v5.5-power-filter/firmware/`.

### 1. The two end LEDs are gone, and the mode announces itself instead

Free play used to idle with **its two ends lit** — a shape the game's
left-filling bar can never draw, so one glance said "this is the instrument".
It worked as a badge and failed as a display: it never went away. Press a lemon,
the pitch bar climbs, and those two LEDs are still sitting inside the reading
pretending to be part of it. *"Al tocar y que se vayan encendiendo los LEDs de
la escala se queda un poco raro... que no se queden esos dos LEDs encendidos de
los extremos."*

So the badge became an **event**. `playFreePlayEntrySweep()` runs once, on the
way in, in three beats:

1. two lights walk in from the ends and meet in the middle — the old idle shape,
   collected up and carried away,
2. they open back out, filling the bar to all ten,
3. and the bar empties from the outside in, to nothing.

It ends **dark**, which is exactly where free play now lives, so the animation
hands over to the mode instead of being switched off by it. Then do-re-mi-fa-sol-
la-si plays as it always did, and the bar stays dark until a finger lights it.
`showFreePlayIdle()` is one `allLedsOff()` now.

**The scale also stopped being abortable.** `playFreePlayFlourish()` gave up the
moment either button read down — a leftover from when it doubled as the wheel's
preview for free play, which it has not done since free play left the wheel on
2026-09-13. The only button that can be down while it plays is the **+** that
just asked for the mode, so the check did nothing but cut the announcement short
for anyone who holds a button a beat longer than three seconds. *"Y como siempre
toques la melodía."* It plays to the end now.

### 2. Two lemons at once sound as two notes

*"Cuando el usuario toque dos notas a la vez, esto se detecte y toque la nota
correspondiente a ambas notas."* Free play only — the game recognises a guess by
comparing **one** frequency against the secret sequence, so a chord there is not
a richer guess, it is an unanswerable question. The game's code is untouched.

**Sounding it** is the easy half. `tone()` owns Timer2 and Timer2 makes exactly
one square wave, so the two voices take turns every `CHORD_SWAP_MS` (10 ms):
50 handovers a second, above the ~20 Hz where the ear stops hearing an
alternation and starts hearing an interval, and slow enough that C5 still gets
five whole cycles to establish its pitch. `tone()` reprograms OCR2A in place
rather than restarting the pin, so a handover costs no click. It is the Game
Boy's arpeggio trick.

**Deciding there are two fingers** is the hard half, and it is the whole of the
work. A touch on this keyboard is 3-4 ADC counts and the channels are coupled:
one finger already drags its neighbours part of the way down, which is why
`strongestKey()` picks a winner by depth instead of taking the first channel
over its threshold. A rule that asked "are two channels below threshold?" would
hear a chord on every single note anyone played — and **a false chord is far
worse than a missed one**, because a note that warbles like two ruins every note
in the mode. So `chordPartnerFor()` is deliberately hard to please. Three gates,
all of which the second lemon must pass:

| | gate | why |
|---|---|---|
| 1 | dip ≥ `touchMargin + CHORD_EXTRA_COUNTS` (2) | coupling crumbs sit just over the plain margin |
| 2 | dip ≥ `CHORD_MIN_RATIO_PCT` (70 %) of the first finger's dip | a second finger is the same ORDER as the first; a shadow is a fraction of it. **This is the gate that separates the two cases** |
| 3 | it is the ONLY key that passes 1 and 2 | three lemons' worth of signal is a hand laid across the fruit, not a chord — refuse the lot |

…and then the survivor holds still for `CHORD_CONFIRM_MS` (24 ms), so a spike on
the way into a single touch cannot open a second voice for one scan.

**Letting go needed the same idea, and that was the bug worth catching.** The
first version asked `keyStillDown()` about the second voice and would have held
a chord until BOTH fingers left: lift one lemon while the other is held and its
channel does not come back to its resting level, it falls back to the *shadow*
of the one still down — which is deeper than the release threshold. What does
collapse the instant a finger lifts is the **ratio** between the two dips, so
that is what is watched, with hysteresis (opens at 70 %, lets go under 55 %) and
the same 90 ms confirmation a single note gets. Either voice may be the one that
leaves: let the lower lemon go and the upper one is **promoted** to the note
under the finger rather than the sound stopping, which is what every instrument
does. While a chord is up it owns the release decision for both of its voices —
`loop()` hands over to `serviceChord()` and does not second-guess it.

`trackBaselines()` now skips the second voice too, or a chord held past
`STUCK_MS` would have re-baselined the lemon underneath it.

**The bar shows two lone LEDs**, one at each note's place. A single note fills
the bar from the left, so two separated points is a shape one finger cannot
draw — the light says "two" before the ear has finished deciding. It is also,
on purpose, the shape the idle bar used to wear: keys 1 and 7 are simply the
widest chord this keyboard has.

### Evidence

- **All five PlatformIO envs build**: `nanoatmega328` (the flashed one) at
  **17 348 B flash / 56.5 %** and **555 B RAM / 27.1 %**, up from 15 570 B /
  50.7 % and 536 B / 26.2 %. `nanoatmega328new`, `nanoatmega328-debug`,
  `emulation` and `emulation-freeplay` green.
- **Host tests: 107 + 122 checks, 0 failed** (`firmware/test/run.sh`), up from
  107 + 107. Three new scenarios, and the fake board grew the thing they needed:
  **`FakeBoard::couplingDepth`**, a model of one finger shadowing the other
  channels. It is 0 by default, so every test written before today still sees
  the clean board it was written for; the chord tests set it, and a chord rule
  that only worked on an idealised board would not be a chord rule.
  - *19* — the sweep's three beats are asserted from the LED **film**, not the
    final pin states, because an animation is invisible to a snapshot; plus the
    bar ends dark and the scale still plays all seven notes.
  - *20* — both notes take turns on the one buzzer; the bar is `#.....#...`;
    releasing either voice leaves the other **sustaining**, not re-struck.
  - *21* — the expensive failure, three ways: a shadow too shallow (fails gate
    1), a shadow deep enough to clear the floor but not the ratio (8 counts
    against a finger's 12 — 67 % against the 70 % needed), and three fingers at
    once (fails gate 3). No chord opens in any of them.
- **Not run here**: the Velxio `--mode verify`. The harness repo is not on this
  machine, which is also why TODO 19 is still open. The host tests are V5.5's
  regression suite (`versions/README.md`), and free play has always been tested
  there rather than in the browser — the emulation has no pins left for the two
  buttons.
- **Flashed to the board**, `/dev/ttyUSB0`, env `nanoatmega328` (the Nano was
  plugged in while this was being written — it had not been at the start):
  17 348 bytes written and **verified** by avrdude, and the boot log came back
  over the serial monitor: banner, seven baselines, an auto margin, `Level 1`.
  What that proves is that the firmware runs; the two features themselves need
  fingers on fruit, which is TODO 21 and 22.

```
  key 1 baseline=1017 noise=15 -> threshold=985      auto margin=32
  ...                                                (worst noise 16 x 2)
  key 7 baseline=1018 noise=16 -> threshold=986      Level 1
```

  Note the margin: **32 counts** on a bench where nothing is hanging off the
  pins. The chord's absolute gate is `touchMargin + 2`, so it scales with
  whatever calibration decides on the day rather than being a number tuned to
  one rig — which is the only form of that gate that survives the fruit
  changing. The ratio gate does not depend on the margin at all.

Docs updated in step: the version README, `docs/USER-GUIDE.md`,
`docs/GUIA-DE-USO.es.md`, and TODO items 18 (its two-ends question is answered),
21, 22 and 23. The organiser's booklet is still accurate — it never described
the idle bar — but it now describes less than the piano does; that is TODO 23,
worth a line at the next reprint rather than a reprint of its own.

## 2026-09-14 — the organiser's booklet, built and printed

One A4 sheet, black and white, both sides, folding down the middle into an A5.
It exists so someone who has never seen the lemon piano can switch it on,
explain it and rescue it with Sergio not in the room. `docs/cartilla/`, and it
went to the laser on job `laser-9`.

Four panels: the cover; what the two buttons do plus **si algo va mal**; how the
game works plus the two questions everyone asks about the finger; and **the four
codes**. Almost no prose — nearly every explanation is a drawing of the **LED bar
itself**, ten boxes filled or empty, because that is the only thing the piano can
say and the only thing a helper has to read.

**The codes are derived, never typed.** A level's code lives in the firmware as
note *frequencies*, and which lemon plays a note is different on every level —
each has its own seven-note row — so the answer a player needs exists nowhere in
the source. `docs/cartilla/seqs.py` resolves one table against the other and
**cross-checks level 1 against the sequence hard-coded in the host test**, which
was written independently. It exits non-zero rather than guess. Change a melody
in `main.cpp`, re-run `build.sh`, and the booklet is right again:

```
level 1  6 5 6 7 2 5 2 1 3 4      level 3  2 4 6 1 5 3 7 4 2 6
level 2  3 6 1 4 2 5 3 6 1 4      level 4  5 1 3 7 2 6 4 1 5 3
```

The booklet also answers the question those numbers raise on their own — *which
lemon is number 1?* — physically rather than by wiring diagram: it is the one
that sounds **lowest**, and free play (hold + for 3 s) plays do re mi fa sol la
si left to right, so anyone can check it in ten seconds.

Two things caught in review before printing, both visible only once rendered:
**Press Start 2P has no accented capitals** and drew `CóMO SE JUEGA` and
`LIMóN`, so the pixel face is now used only where Spanish needs no accent and
DejaVu carries the rest; and the ten-LED bars were 42 mm wide in a 19 mm column,
so the text ran straight over them.

Printed duplex **short edge** — the content is landscape, so that is the flip
that reads like a book; long edge would put the inside spread upside down.

## 2026-09-14 — the event poster, stored byte for byte

Sergio made a poster with Gemini to stand behind the lemon piano at a private
event, so people come over and play it, and asked for it in the repo with no
loss of quality. It is at `docs/poster/super-limon-piano-cartel.png`, copied
**byte for byte** — never opened and re-saved, never re-encoded:

```
sha256  b8db4412b7b9a77b31b51f8a2a371763dbf7a310eab1b599980af863dad18f26
bytes   1 010 859
848 x 1264 px, 8-bit RGB
```

`cmp` against the original returns identical. The hash is written into
`docs/poster/README.md` so a later "compress this PNG" is detectable rather than
silent.

**What it can be printed at**, measured rather than assumed: the file is pixel
art in style only — **102 156 distinct colours** with soft anti-aliased edges —
so nearest-neighbour enlargement, which is lossless on real pixel art, would
only magnify the softness. 300 dpi is 72 × 107 mm; full A4 works out at about
**102 dpi**, soft up close and fine from a metre away, which is normal for
large-format printing. Its aspect is 1:1.49 against A4's 1:1.41, so "fit to
page" leaves ~10 mm of white down each side and "fill page" would crop ~16 mm
off the top and bottom, into the wordmark.

Also built, and NOT chosen: an A4 poster composed here in HTML and rendered
through headless Chrome at a true 2480 × 3508 px (300 dpi) — the Super Mario 256
wordmark with a hand-built acute accent the font does not ship, seven
photographed lemons cut out by chroma, and the same four levels. It is outside
git at `~/Pictures/piano-limones/` on quantumpc.

## 2026-09-13 — a repeated lemon is not a mistake: it sounds, and the bar says so

Sergio, after the four changes above landed: the locked-key rattle reads as
"you got it wrong", and pressing the same lemon twice is not wrong. It is just
not a move. He asked for the lock and the scolding noise to go, for the note to
sound properly, and for the piano to say **visually** that the press did not
count — and left the choice of cue to me.

**What the game does now.** A repeat sounds its note at full voice (a piano
before it is a game), scores nothing, advances nothing and — the part that
changed — costs nothing. Gone with it: `KEY_LOCK_COOLDOWN_MS`, `lastReleaseAt`,
`lastSoundedKey` (fully redundant with `lastCountedKey`) and `soundKeyStuck()`.
`sfxKeyStuck` stays in `mario_sfx.h`, unused by V5.5; earlier versions still
play it.

**The cue: the bar runs BACKWARDS.** One LED from the right end to the left,
then the progress bar snaps back exactly as it was. Three deliberate choices,
each one a reason it can be read across the room with the lid closed:

- it **moves**, and this bar is otherwise perfectly steady while playing. Motion
  is already this piano's word for "neither a score nor a question" — it is what
  the wheel used to use to mark free play — so it cannot be read as either.
- it runs **right to left**, against the direction progress fills, so it reads
  as "that took you nowhere" and never as a step forward.
- it **ends on the identical bar it started from**. The score is visibly
  untouched, which is the whole message. A wrong note blanks the bar and LEAVES
  it blank, so the two can never be confused.

It is not the menu's entry sweep either: that one goes out and back, is slower,
and arrives with a three-note cue.

**A cue that starts and ends on the same picture is invisible to a test that
only reads final pin states** — which is how the first version of test 18 passed
with the sweep deleted. So the fake board now records a frame per change to the
bar (`ledFrames`, capped) and the test asserts the **shape of the motion**, not
just where it landed.

Evidence:

- New **test 18**, `A repeated lemon sounds, scores nothing, and costs nothing`:
  it sounds, it is that lemon's own note, no WRONG, no OK, progress untouched,
  the bar ends where it started, one sound only, the cue really is a right-to-
  left run, the next correct lemon still scores, and a wrong note still blanks
  the bar and leaves it blank.
- **107 + 100 checks, 0 failed.**
- **Mutation controls, run separately:** delete `showRepeatSweep()` → 2 checks
  fail (`0 frames, none of them a backwards run`); make the repeat silent again
  → 4 checks fail.
- Two harness defects found and fixed on the way, both mine: `ledFrames` grew
  without bound, and test 18 indexed an empty vector (ASAN: SEGV at
  `piano_sim_test.cpp:585`). The second is why the first mutation run reported
  a crash instead of a failure.
- `pio run` clean on all five envs; 15570 B.
- Flashed and verified on the board: boots, all seven at baseline 1023 /
  noise 0 / margin 4, Level 1.

Both printed instruction sheets and `docs/MARIO-SOUNDS.md` updated — they
documented a lock, a cooldown and a rattle that no longer exist.

**Not yet played by a human.**

## 2026-09-13 — four changes from the first real session with the board

All four are Sergio's own reports after playing the fixed v0.7.1 rig, and each
one lands with a named test and a mutation control.

**1. Neither button ramps the sensitivity any more.** Holding used to fire a
step every 120 ms. Both buttons carry a hold gesture now, so a hold that also
spun the knob thirty times re-tuned the keyboard on every mode change. A tap is
one step and that is all. The nudge still fires on PRESS — a knob that waits for
the release feels broken — so the gesture that fires puts its own step back:
each button snapshots the margin as it goes down (`marginBeforePlus` /
`marginBeforeMinus`) and restores its own.

**2. The LED bar counts SENSITIVITY, not margin.** More LEDs = more sensitive =
a smaller margin, so **+** adds light and **−** takes it away. It was inverted,
and he called it immediately: a button labelled "more" that puts lights out
reads as "less" from across the room. The bar's own comment already claimed it
was a sensitivity meter; now it is one.

**3. FREE PLAY left the wheel and became a hold on +.** His words: "mantener el
botón más es un switch entre el modo libre y el nivel". So the wheel is four
levels and nothing else — turning it can never take the instrument away — and
+ held for 3 s swaps game for instrument and back. Going in remembers the level
it interrupted; coming out restarts it, for the same reason accepting a level
does. A player who never touched the wheel is on level 1, so for them it is
exactly the "free play <-> level 1" switch he asked for, with no special case.
Opening the wheel from inside free play lands on the interrupted level, so the
wheel is still a way out. + and − now arm identically (1 s meter, 3 s fire) and
differ only in where they land.

**4. A HELD lemon sounds once.** In free play, resting a finger on a lemon
machine-gunned the note. The contact is not a switch: through 1 MOhm it breaks
for a few ms at a time without the finger moving, and every one of those ended
the note and started a new one. A release is now only believed after
`RELEASE_CONFIRM_MS` (90 ms) of silence — the note survives any shorter dropout,
a real release still lets it sound again, and fast deliberate playing is
untouched.

Evidence:

- `test/arduino/Arduino.h` grew a flaky-contact model (`dropoutEveryMs` /
  `dropoutMs`) so a held key can read clear in bursts, which is what the bug
  needed to exist at all.
- New tests: gestures 2, 3, 3b, 3c (no ramp; + arms and toggles; + cancels; the
  two holds never cross) and piano 15, 16, 17 (the switch both ways and the
  level it remembers; a held lemon is one note; the bar counts sensitivity).
- **107 + 84 checks, 0 failed.**
- **Mutation controls:** put the ramp back and 5 gesture checks fail; remove the
  release confirmation and test 16 reports **11 notes for one held lemon** —
  the reported symptom, reproduced exactly.
- `pio run` clean on all five envs; 15754 B.
- Flashed and verified on the board: boots, all seven at baseline 1023 /
  noise 0 / margin 4, Level 1.

Docs rewritten to match, including both printed instruction sheets
(`docs/USER-GUIDE.md` and `docs/GUIA-DE-USO.es.md`) — they documented a ramp
that no longer exists and a wheel item that moved.

**Not yet played by a human.** Everything above is the host tests and a boot log.

