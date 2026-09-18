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

## 2026-09-13 — the SENS − button played a note: A7 is on the key multiplexer

With all seven pull-ups swapped to 1 MΩ and R18 finally a real 10 kΩ, every key
sounds and the dead LED 4 came back. A new symptom arrived with them: pressing
the **−** button beeped like a key and, held on, scored a wrong guess.

**Cause, and it is entirely inside the chip.** SENS − is **A7**, which is
analog-in only (hence R18) and is **channel 7 of the same ADC multiplexer that
carries the seven keys**. `serviceButtons()` reads it at the top of every
`loop()`, immediately before the key scan starts at A0 — and while the button is
held A7 sits at **0 V**, so the sample-and-hold cap is handed to key 1 holding
zero. Through 220 Ω that refilled 4000 τ before the conversion finished; through
1 MΩ one conversion closes only **57 %** of the gap, and `readKey()`'s single
discarded conversion (2026-09-13, earlier today) is only the first step.

Reproduced on the host with no hardware: `readKey(0) = 944` against a threshold
of 1016 — **72 counts below the trigger point**, every loop, for as long as the
button is down. Which also explains why it stopped past the middle of the LED
bar: the droop is finite, so a wide enough margin hides it. That was the margin
covering the bug, not the button behaving.

**Fix:** `readKey()` now converts the channel **until two readings agree**
(≤ 1 count apart, hard stop at 12) before averaging four. Not a bigger constant —
a settled 220 Ω channel breaks out on the second conversion and costs 112 µs, the
0 V → 5 V worst case on 1 MΩ takes eight and costs 0.9 ms, and nothing in it is
tuned to a particular pull-up. These resistors have changed twice already.

Evidence:

- `test/arduino/Arduino.h`: the binary mux-residue model replaced by an
  exponential one (0.575 of the gap per conversion at 1 MΩ), with A7 on the same
  multiplexer and settling instantly, which is the asymmetry that causes the bug.
- New **test 14**, `Holding SENS − does not press a key`. 68 checks, 0 failed.
- **Mutation control:** restore the single discard and test 14 fails 3 of 6.
- `pio run` clean on `nanoatmega328`, `nanoatmega328new`, `emulation`; 15492 B.
- Flashed and verified on the board: boots, calibrates all seven at baseline 1023
  / noise 0 / margin 4, Level 1.

**Verified on the board the same day:** Sergio pressed **−** repeatedly on the
real rig with this firmware on it — **no key sounds**
(`qpc-approvals piano/boton-menos-a7` → approved). That also kills the competing
theory: if the note had been his body coupling into a 1 MΩ node while he held
the button, changing how many times a channel is converted could not have
removed it. It was the multiplexer.

Full write-up: `pcb/docs/REVIEW-v0.7.1-silent-keys.md` §3.

## 2026-09-13 — quiet-rest control: 1 MΩ does not ghost, and the game is loaded

The V5.5 game firmware (with the first-conversion discard) was flashed to the
board: 15454 bytes written and verified. It boots, calibrates all seven at
baseline 1023 / noise 0 / margin 4, enters Level 1, and **detects key 1** — the
`key 1 again - locked` messages are the repeat lock behaving correctly, since
keys 2–7 are still 220 Ω and cannot unlock it.

A first pass showed chatter (11 retriggers in 0.6 s) and the margin walking from
4 to 76 on its own. Both were the operator playing with the board during the
flash — he confirmed it, and then left the rig untouched so it could be proved.
Unattended batch, nobody in the room:

- **Game, 4 min at rest:** after `Level 1`, **zero lines**. No phantom notes.
- **Probe, 4 min at rest, 2087 samples:** the 1 MΩ channel reads **1023 in
  100.00 % of samples** — zero excursion, zero samples below the 1019 threshold.
  Channels 2–7 (still 220 Ω) likewise flat.

Against a press signal of 132–291 counts and a threshold of 4, that settles it:
**do not raise `TOUCH_HYSTERESIS`, do not drop to 470 kΩ.** 1 MΩ has room on both
sides. Logs kept as `*-quiet-rest-4min-2026-09-13.txt`; the batch re-flashed the
game afterwards, so the board is playable as it stands.

Process note: the results sat finished for 26 minutes because the agent's own
wait loop ran `pgrep -f quiet.sh`, a pattern that **matches the waiting shell's
own command line** — it waited on itself and no completion ever fired. The user
had to come and say so.

## 2026-09-13 — SOLVED: R1 at 1 MΩ, 15 clean presses, no ground clip

External 1 MΩ from J2 pin 1 to the Nano's 5V pin, R1 lifted. Baseline **1022,
noise 2**, so the game's auto-margin would be **4**. Running event detection at
that real threshold finds **15 presses on channel 1**, and the operator's account
of the session falls straight out of the timestamps:

| phase | presses | dip |
|---|---|---|
| no ground, shod | 5 | **132–200** |
| gripping ground | 5 | **255–291** |
| barefoot on the floor | 5 | **166–258** |

Worst of the fifteen: **132 counts against a margin of 4 — 33× headroom.** Best:
72×. Presses last 300–1500 ms and return cleanly to 1023. **Channels 2–7 stayed
at exactly 1023 for all 90 s** — no crosstalk, no phantom notes, no stuck keys.

Barefoot does matter and does not matter: the floor is a partial return to
earth, better than isolated and worse than a hand on circuit ground, and the
three phases measure it cleanly. All three clear the threshold thirty times over.

**What the three-day zero actually was.** Not a broken board, not a bad netlist,
not flux, not the fruit header — a **220 Ω pull-up used as a body-resistance
sensor**. It needs the whole hand → body → fruit path under ~56 kΩ; a clipless
touch on this board measures **5–10 MΩ**. Four orders of magnitude, which is why
no assembly fault could have explained it and every intermediate theory missed.
The breadboard was the same design working on **2 counts** of headroom (§2f);
the PCB simply fell off that edge. 1 MΩ turns 2 counts into 33×.

Next: swap R2–R7 to 1 MΩ, then re-run the probe with all seven high-impedance —
the one thing this capture cannot predict, because channels 2–7 are still 220 Ω
and therefore immune to the coupling a MΩ neighbour could induce. Fallback if
ghosting appears: 470 kΩ, still ~65–145 counts on the measured contact.

## 2026-09-13 (night) — `readKey()` discards its first conversion, with a test that proves it

Firmware, V5.5 only. `readKey()` now throws away one conversion on each channel
before averaging its four samples.

**Why now.** The ATmega328P's sample-and-hold charges through the source; the
datasheet's ceiling is a **10 kΩ** source impedance, above which the first
conversion on a newly selected channel has not settled and mostly carries the
*previous* channel's level. At 220 Ω that never mattered. It matters the moment
the pull-ups go up — and the bench numbers say they must: measured on the real
v0.7.1 board, a **clipless touch is a 5–10 MΩ path**, which is 1–2 counts through
220 Ω and **91–168 counts through 1 MΩ**. Without the discard, a high-impedance
keyboard reads as the "gradient ramps" the V4 front end was rejected for in July.
One extra conversion per key, ~112 µs, harmless at any impedance, so it is
unconditional rather than a build flag.

**And a positive control, because 56 green checks proved nothing here.** The fake
board gained an optional `muxResidue` model: with it on, the first conversion on
a newly selected channel returns the previous channel's level — the textbook
description of an unsettled S/H. New section 13 holds key 5 down and asserts the
held channel reads its own level and the idle neighbour is not dragged with it.

Verified by mutation: **delete the discard and section 13 fails**, with the idle
neighbour reading **971 instead of 1023** — 52 counts of smear against a margin
of 4, i.e. a phantom note. Every other check passes either way, which is exactly
why the test had to be written: a 220 Ω front end settles instantly and cannot
show the bug.

97 + 62 checks green, AVR build clean (15454 B flash).

Still not applied to the board: the resistor swap is the owner's call and the
measurement that justifies swapping all seven has not been taken yet.

## 2026-09-13 (late) — R1 → 10 kΩ, twice: a floating node, then the real number

R1 lifted, a 10 kΩ through-hole resistor hung externally from J2 pin 1.

**Run 1, 10 kΩ to J1 ("5V IN").** Channel 1 swung 512 counts (383…895), the
other six flat. It looked like a triumph and was a floating pin: baseline **735**
(≈ 3.6 V) not 1023, the channel rose **160 counts above its own baseline** —
which no pull-up can do — and noise 10 with slow drift. J1 is the power input;
with the board on the Nano's USB the 1N5817 isolates J1 from the rail, so the
resistor was tied to a node held only by two diodes' reverse leakage: a 10 kΩ
antenna on an open pin, swung by a hand *near* it. That is the V4 floating
keyboard of July, called unreadable then. *It moved* is not *it worked*. (Log
not preserved — overwritten before it was copied; the numbers are from the
analysis at the time.)

**Run 2, 10 kΩ to the Nano's 5V pin.** Baseline **1023, noise 0**, all seven.
Channel 1 best dip **14 counts**, typical 1–2. Log
`pcb/validation/probe-v0.7.1-R1-10k-on-rail-2026-09-13.txt`, first log with the
new self-describing header (`label: pcb`).

Solving the divider for the finger's contact resistance, per capture:

| rig / condition | pull-up | dip | contact |
|---|---|---|---|
| breadboard, "no ground" | 220 Ω | 8 | **28 kΩ** |
| breadboard, gripping USB shell | 220 Ω | 40 | **5.4 kΩ** |
| PCB, 10 kΩ on rail, best | 10 kΩ | 14 | **721 kΩ** |
| PCB, 10 kΩ on rail, typical | 10 kΩ | 1–2 | **5–10 MΩ** |

**Same finger, same grip: 5 kΩ on the breadboard, 0.7–10 MΩ on the PCB.** With
that contact, 220 Ω yields 0.02–0.3 counts — the zero of the last three days,
finally with a cause attached. The circuit and the value are no longer in
question; the contact at the header pin is. Two consequences: the breadboard's
"clipless" 28 kΩ cannot be capacitive at DC, so there was an accidental return
path (a finger on an exposed GND jumper tip — a breadboard has dozens), and the
finished piano in its box will behave like the PCB, not the breadboard; and with
the return path held equal, the 100× gap left is at the pin surface — J2 was
hand-soldered from below and rosin flux wicks up pins as an insulating varnish.

Next test is how the instrument is actually played: **alligator clips** on J2
pin 1 and pin 8, which bite through any film. Expected with 10 kΩ: ~660 counts
with a 5.4 kΩ body, ~270 with 28 kΩ. Review §2g has the full table and the
honest route to a clipless version (MΩ pull-ups + first-conversion discard +
the adaptive baseline the firmware already has — a V5.6 design, not a repair).

## 2026-09-13 (night) — a log that cannot say what it measured is not evidence

A capture arrived that could have come from either rig, and the two readings of
it were **opposite conclusions**: either the v0.7.1 board now has a correct 1023
baseline but a finger that cannot reach its pin at all, or it is a recording of
an untouched breadboard and means nothing. The file name (`pcb` / `proto`) is
only a label — `run.sh` records whatever is on `/dev/ttyUSB0`, and both Nanos
here are indistinguishable CH340s (`1a86:7523`, no serial number), so nothing in
the log identified the rig.

- `capture.py` now writes a header into every log: the **label the operator
  chose**, the timestamp, the port, the USB path (`ID_PATH`) and the window.
- `run.sh` passes the label through and shouts it before recording — *"RECORDING
  AS: 'pcb' — is THAT the rig on /dev/ttyUSB0?"* — with the warning that the
  filename is only a label and a mislabelled log proves nothing.

The USB path is the one thing that is at least *physically* meaningful: if the
two rigs live in two different sockets, two logs from the same socket are
provably the same rig. It does not identify the board, but it can falsify a
claim that two captures came from different ones.

## 2026-09-13 (evening) — the breadboard, measured: it works by two counts

The comparison never made in three days. Same probe, same body, same supply,
same USB cable. The three phases the operator described are visible in the
timestamps without being told: touch, a 20 s pause, touch again gripping the
Nano's USB shell.

| phase | ch1 | ch2 | ch3–7 (untouched) |
|---|---|---|---|
| touching, no ground | 1015 → **8 counts** | 1017 → **6 counts** | 1021–1023 |
| pause | 1023 | 1023 | 1023 |
| touching, holding GND | 994 → **29 counts** | 983 → **40 counts** | 1023 |

Against the v0.7.1 board's **0 counts in both conditions**, and a game margin of 4.

**The working rig fires without a ground clip by a margin of two counts.** That
reframes everything: this was never a robust design that the PCB broke. It is a
design with ~2 counts of headroom, and on the PCB it fell off the edge.

§2's arithmetic was right and its conclusion wrong in one specific way. 220 Ω
does give a usable signal — 6 counts, from a **28 kΩ** contact, because a
fingertip pressed hard on bare pin metal is not the megohm of a "dry finger".
Holding ground drops the contact to **5.4 kΩ** and buys 40 counts.

It does not convict the PCB yet: the same finger cannot be 5 kΩ on one rig and
>250 kΩ on the other, so either the boards differ in a way still unfound, or the
**gestures** were not the same — the PCB runs were "fruit, the pin, before and
after the resistor", and may never have included that exact grip on bare J2
metal. So the last experiment is a back-to-back with one variable: identical
gesture, identical finger, identical USB shell, on the board.

And one recommendation moves from theory to evidence: **220 Ω leaves 2 counts of
headroom on the rig that works.** Raising R1–R7 turns 6–8 counts into hundreds
and removes the ground clip from the equation. Still queued behind the
back-to-back, because a fix applied before the cause is known is a coincidence,
not a repair.

## 2026-09-13 (later) — the board is good, and a wire proved it in thirty seconds

A bare wire from J2 pin 1, then pin 2, to J2 pin 8 (GND), probe running:

| channel | range over 176 s | bridged? |
|---|---|---|
| 1, 2 | **1023 → 0**, full scale | yes |
| 3–7 | 1023 → 1022 at worst | no |

Full-scale swing on exactly the two bridged channels and nothing on the other
five. It proves end to end that `J2.n → Rn → A(n-1)` is continuous, that
`J2.8 → GND` is continuous, that the ADC resolves the whole range, and that the
channels do not cross-talk. Log in
`pcb/validation/probe-v0.7.1-jumper-2026-09-13.txt`.

**Every hypothesis that blamed the board is now dead** — routing, buzzer, stuck
buttons, missing pull-ups, bridged pull-ups, shorted key nets, an unsoldered
fruit header. The meter cleared the last of them: all seven key resistors read
220 Ω, and key-to-key reads ~440 Ω, which is R+R through the shared rail, i.e.
exactly the *expected* value for a healthy pull-up bank. (That ~440 Ω is also,
measured, the same observation the owner made on day one: "all the resistors
look joined to each other". They are, and correctly.)

Two incidental faults found and confirmed irrelevant: **R18 fitted as 220 Ω
instead of 10 kΩ** (only touches `/SENS_MINUS` and `/+5V`; identical behaviour
with the button up, and the button works), and **LED4 dead** (anode on D5, a
digital output sharing nothing with A0–A6).

**The method lesson, which is the expensive part.** This wire test costs thirty
seconds and should have been the first thing done — before any theory about
pull-up values, skin impedance or earth references. It substitutes a known
resistance for the human and turns an argument into a number. Instead two days
went into arithmetic that was correct and aimed at the wrong question: *is the
board able to see a key at all?* was never asked. Worse, every test that WAS run
used a finger, and a dry finger is ~1 MΩ — which reads 1023 on a perfectly
working board. Every one of those tests was incapable of producing a signal, so
none of them could ever have been evidence.

What remains is not on the board: the path from the header pin to the hand — the
fruit, the clips, the wire, the return to GND. And the one comparison never made
in three days is the probe on the **breadboard**.

## 2026-09-13 — the probe records to a file at last, and the board's answer

`pio device monitor` wraps pySerial's **miniterm**, which is an interactive
console: point its stdout at a pipe or a file and it writes **nothing**. So the
second run produced a **zero-byte log from a correctly flashed board** — the
board was printing perfectly and the recorder was the broken part, for the
second round trip running.

- **`probe/capture.py`** — reads the port with pySerial directly, timestamps
  every line, flushes per line (a killed run still leaves a usable log), and
  toggles DTR first so the log always opens on the boot block with the seven
  baselines in it.
- `probe/run.sh` uses it instead of `pio device monitor`.

**First real reading from the assembled board**, taken over SSH with the Nano
plugged into quantumpc:

```
  key 1  baseline=1023  noise=0
  key 2  baseline=1023  noise=0     ... all seven identical
keys:  1023(-0)  1023(-0)  1023(-0)  1023(-0)  1023(-0)  1023(-0)  1023(-0)
```

All seven channels **pinned at 1023 with exactly zero counts of noise**, held
flat for the whole window. Two things follow immediately, and the second one
closes a question this review had left open:

1. **R1–R7 are populated and connected.** A floating analog pin does not read a
   rock-steady 1023 — it wanders, and it drags the previous mux channel's
   residue with it (that is precisely what the V4 keyboard measurements looked
   like: "gradient ramps", 76–104 counts of noise, 170 counts of drift). Seven
   identical, immovable 1023s are the signature of a stiff pull-up doing its
   job. So "the resistors or the rail are missing" is dead, positively rather
   than by inference.
2. **Zero noise means the auto-margin is at its floor of 4**, which is the
   best case the firmware can offer. Nothing is mis-calibrated. The margin was
   never the problem.

The board is doing exactly what it was designed to do. That is the finding.

**Then the long run: 156.5 s, 1345 samples, seven channels, and the set of
distinct values seen on every one of them is `{1023}`.** Zero movement, both
directions, for two and a half minutes with the fruit being played. Log kept at
`pcb/validation/probe-v0.7.1-board-2026-09-13.txt`.

That is the *predicted* number. §2's table said 0.2–0.7 counts for a normal-to-dry
touch through 220 Ω, and an ADC cannot show a fraction of a count — so the
prediction was zero, and zero is what the instrument reports. Working backwards,
a reading of exactly 1023 means the hand → body → fruit path stays above
~250 kΩ, which dry skin does on its own.

And one consequence worth writing down plainly: **no firmware change can rescue
this board.** A 220 Ω path to the rail defeats the resistive divider (sub-count
signal) *and* RC-timing / `CapacitiveSensor` alike, because the node recharges
through 220 Ω in nanoseconds. The resistor has to physically come off.

The plan is now in §2d of the review, cheapest step first: **remove R1–R7, fit
nothing, measure again** — no new parts, and it settles what the breadboard is
doing without anyone having to describe it. Then fit 1 MΩ (already in the parts
drawer, top unit) and make `readKey()` discard its first conversion.

## 2026-09-12 (fix) — the probe flashed fine and printed to nobody

First real run on the board: upload SUCCESS on `/dev/ttyUSB0`, and then nothing.
The probe's `platformio.ini` declared two environments and no `default_envs`, so
`pio run -t upload` uploaded **twice** — the second one out of sync against a
board that had just been reset — the command exited non-zero, and the
`&& pio device monitor` chained after it never ran. A correctly flashed board
printed to nobody, and the round trip was spent on a bug of mine.

The second attempt then auto-detected `/dev/ttyS0`, the motherboard's own serial
port, and failed with "Permission denied" — which reads like a permissions
problem to go and fix, and is not one.

- `default_envs = nanoatmega328`, with the reason written next to it.
- `upload_port` / `monitor_port` pinned to `/dev/ttyUSB*`, so auto-detect can
  never wander onto `ttyS0` again.
- **`probe/run.sh`** — one command, one file: `./run.sh pcb` flashes, records a
  fixed window into `~/sonda-pcb.txt`, and prints the verdict lines at the end.
  If no `/dev/ttyUSB*` is present it says so in one line instead of failing on
  the wrong port. `./run.sh proto` does the breadboard.

The file matters as much as the fix: the board is flashed from quantumpc over
SSH, so the log lands on the machine the agent is already on — nobody has to
copy a screenful of serial output out of a terminal to be read.

## 2026-09-12 (later) — the bench answers back: the probe learns to look both ways

Three facts from Sergio with the board in his hands, and they delete two of the
three branches of this morning's review:

- **Every boot sound plays and both SENS buttons beep.** The buzzer branch is
  closed. So is the stuck-switch worry — the buttons respond, so SW1/SW2's pad
  grouping is right.
- SENS− is on A7 and works **only** through R18, a pull-up to `/+5V` in the same
  bottom-side row as R1–R7. That beep proves the 5 V rail reaches the key
  pull-ups, with no meter. "The rail or R1–R7 are missing" is closed too.
- **He never touches a clip.** On the breadboard, with the same 220 Ω, he touches
  only the fruit, calibrates, and it plays.

Buzzer, buttons, LEDs and rail alive; exactly one block dead. And the third fact
does not fit this board at all: a 220 Ω pull-up is a stiff node, so a body that
is not galvanically tied to circuit GND couples nanovolts into it. Clipless play
cannot work on the v0.7.1 front end — not by four counts, by four orders of
magnitude. The working rig must have a **high-impedance** key node: either the
V4/V4.5 front end (pin floating, the 220 Ω in *series*, touch reads **UP**) or an
earth reference through an earthed PC's USB. Same resistor value in both, which
is exactly why "the same values work on the breadboard" was never evidence of the
same circuit.

Also answered, because it was a fair objection: *"pressing both buttons should
auto-calibrate whatever the value is — it has range."* It does; `MARGIN_MAX` is
600. The range is not the problem, the **direction** is. `learnFromTouch()`
tracks `lo[]` only, and clamps `dropped[i]` to 0, so an upward touch scores zero
on every channel and the gesture reports "not separable from noise — margin
unchanged" forever. `autoCalibrate()`, `keyTouched()` and `strongestKey()` all
look down too. A one-directional search cannot be widened into a two-directional
one.

So the probe is now **direction-agnostic**: it reports the largest excursion
since boot **both ways** and prints a one-line verdict — signal DOWN (this
board's polarity, tune the margin), signal UP (wrong front end, and no
calibration gesture will ever find it), or nothing moves (no signal to
threshold). A probe that only looked down would have hidden the very thing it
was flashed to find. Builds clean.

The protocol in the review is now one experiment with two runs: the same sketch
on the **board** and on the **working breadboard**, comparing the sign. Only one
of the four outcomes puts the fault back inside the PCB, and it is named. Plus a
free test in the same session — power the board from an earthed PC instead of a
charger, since V5.5's own powering rule ("never the PC's 5 V") is what would have
removed the earth path that makes clipless play work.

Nothing desoldered, no values changed, V5.6 still unwritten: it waits on the two
runs.

## 2026-09-12 — V5.5 wiring diagram: every part labelled with its PCB v0.7.1 reference designator

- **Why**: the V5.5 breadboard is built and the PCB exists, so a part has to be
  findable on both. The diagram previously showed values only (`470 µF`,
  `220 Ω`), which cannot be matched to a board silkscreen.
- **`build_v5_5()`** in `tools/wiring_diagrams.py` now labels all 42 parts with
  the designator from `pcb/docs/NETLIST.md`: `J1 D1 D2 C1 C2 L1 C3 C4`
  (filter), `R1..R7` (key pull-ups, R1 = A0 = KEY1), `D3..D12` + `R8..R17`
  (LED bar), `BUZ1 SW1 SW2 R18` (UI) and `R19 R20 C5` (the off-board amp).
  Values move to the legend where a part has only one text slot.
- **LED bar now carries BOTH numbers**, because they are off by one and that is
  the easiest mistake to make: the bold ref is the PCB designator `D3..D12`,
  the small line under it is the Arduino pin `D2..D11` that drives it.
- **LED colours now match the PCB VU meter** (3 green, 3 yellow, 2 orange,
  2 red per ADR-021) instead of ten green, verified by sampling the rendered
  pixels at the bar centres.
- **`keyboard_2019()` gains `refs=False`** so only V5.5 opts in. Re-rendering
  all ten revisions changed **only** `wiring-v5.5.png`; V1, V2, V2.5, V3, V5
  and V6 are byte-identical.
- The legend gains the PCB cross-reference: the keys land on `J2` (pin 8 = `G`,
  the hand-held clip), `J3`/`J4` parallel the SENS buttons, `J5` is where the
  amp plugs in, `U1`/`U2` are the Nano's socket rows, `H1..H4` the M2 holes.
  It also states the two physical differences: the PCB runs the LED bar the
  other way round (LED1/D3 at the EAST end), and the amp stage is off-board.
- The SENS buttons keep their `SW1`/`SW2` hardware labels; the FREE PLAY /
  MODE WHEEL roles added on 2026-09-09 are firmware on the same two buttons,
  no wiring change.
- `versions/v5.5-power-filter/HARDWARE.md` gains the full designator table.
- **Verification**: 64 nets, **0 spacing warnings, 0 hard violations** (the
  wirewright DRC treats label boxes as obstacles, so it is what proves the new
  longer labels do not collide); all ten diagrams re-rendered clean; label
  text dumped from the built schematic to confirm all 42 designators are
  actually drawn.

## 2026-09-12 — v0.7.1 assembled and silent: review, board probe, doc correction

The real PCB arrived, was assembled, and plays nothing — touching a lemon or an
A-pin directly produces no note, on several power supplies, while the Nano boots
and the LED bar runs. The owner suspected a design fault, having noticed that on
the board every key resistor looks joined to every other.

**The copper is not the fault.** `pcb/kicad/lemon-piano.kicad_pcb` was re-derived
independently of the design tooling — pads parsed out of the footprints, tracks
joined geometrically, the B.Cu pour treated as the GND node. All 34 nets are one
connected island each, no net is split, no island carries two nets, no non-GND
copper falls inside the pour, and the Nano socket rows map pin-for-pin onto the
real Nano. KiCad 9.0.9 DRC agrees (0 violations, 0 unconnected). The resistors
that look joined *are* joined, correctly: R1–R7 share `/+5V`, R8–R17 share the
GND pour.

Which means DRC never answered the question being asked. **DRC checks the copper
against the netlist; it cannot check the netlist against the intent.**

**The fault is the front end, and it was in the file all along.** The touch
signal on this board is **4 ADC counts out of 1023**: a 220 Ω pull-up against
skin only fires if the entire hand → body → fruit → clip path stays under
**56 kΩ**, and dry-skin contact alone is routinely 100 kΩ–1 MΩ. One extra count
of measured noise pushes `max(4, 2 × noise)` above the whole signal and the board
goes silent exactly as reported. `versions/v5-led-bar/HARDWARE.md` already named
the fix in its last paragraph and did not take it.

Added:

- **`pcb/docs/REVIEW-v0.7.1-silent-keys.md`** — the review: the connectivity
  evidence, the signal budget as a table of pull-up against body resistance, the
  buzzer branch as the cheaper thing to rule out first (the firmware plays a
  fireball, seven coins and a power-up sweep *before* any lemon is touched, so
  "silent at boot too" and "silent only on touch" are different faults), and a
  proposed V5.6: R1–R7 at 220 kΩ plus a discarded first ADC conversion.
- **`versions/v5.5-power-filter/firmware/probe/`** — a standalone PlatformIO
  project that turns "it does not sound" into numbers: beeps D13, prints every
  channel's baseline, then all seven channels at 10 Hz with the largest
  excursion since boot. Builds clean (5242 B flash, 269 B RAM). The game build's
  `-DDEBUG_TOUCH` cannot do this — it logs only *accepted* presses, so it says
  nothing when nothing is ever accepted.

Corrected — **two documents disagreed with the board, the firmware and
themselves**, and were being read as ground truth while the board sat silent:

- `versions/v5-led-bar/HARDWARE.md` "Touch sensing" claimed "pins float near 0,
  the +5 V clip through the body raises the reading,
  `threshold = baseline + max(40, 3 × noise)`" — V4.5's front end, copied forward
  and never updated, three paragraphs below the measured section that says the
  opposite.
- `docs/HARDWARE.md` filed V5 under "2026 boards — the clip is on +5 V" in the
  prose, the polarity table and the calibration section.

Both now say what `main.cpp` does: `threshold = baseline − max(4, 2 × noise)`,
pins pulled **up**, player on **GND**, touch drags **down**.

Nothing was changed on the board or in the game firmware: V5.6 waits on the
measurements, because the buzzer branch may turn out to be the whole story.

## 2026-09-09 — V5.5 firmware: FREE PLAY and the MODE WHEEL (no hardware change)

Requested by the owner with the board unplugged: *"diseña todo el software y
valídalo para que cuando pueda pueda cargarlo en el Arduino"*. Everything below
is firmware and docs. **V5.5 only** — V5 is untouched and still builds; the two
firmwares were byte-identical until today and have now deliberately diverged.

**FREE PLAY** — the piano with the game removed, and the fifth item on the wheel.

- The seven lemons become **do re mi fa sol la si** (C5 D5 E5 F5 G5 A5 B5): a
  fifth row in `keys[]`, so the existing `(level-1)*KEY_COUNT` offset addresses
  it with no new indexing. Deliberately not one of the level rows — those are
  puzzle alphabets (level 3 carries a C♯, level 4 skips B) and someone sitting
  down at a piano expects the notes they were taught at school.
- **The same lemon may be played over and over.** The game's `lastSoundedKey`
  lock — which exists because flaky fruit contact used to machine-gun both the
  buzzer and the guesses — is exactly backwards for an instrument, so free play
  takes a short path out of the press handler before it. `handleGuess()` returns
  immediately in free play as well, belt and braces: nothing is ever scored,
  nothing is ever wrong.
- The LED bar changes job: a **pitch meter** while a note sounds ((key+1)·10/7,
  so seven keys use all ten LEDs), and **only its two ends lit** when idle — a
  shape the game's left-filling bar cannot produce.
- It is **not a level**. The win path advances `level` against `LEVEL_COUNT`, so
  the cycle stays 1→2→3→4→1 and never lands here. It can only be chosen.

**THE MODE WHEEL** — the game-select switch V5 removed, back as a gesture.

- **Hold − for 3 s** to open. From 1 s the bar becomes a charge meter and a chirp
  climbs 3300→4700 Hz with it, so the gesture announces itself before it fires;
  release early and a bump says "cancelled".
- **+ / − turn it**, five items, **wrapping both ways**. Each stop previews the
  opening of its own theme (capped at 8 notes) and shows itself: **level n = n
  LEDs blinking**, free play = **one LED running**. Blinking is a question,
  steady is a score, motion is neither. A preview is aborted the instant the
  wheel turns again, so browsing runs at the speed of the hand.
- **Both buttons at once accepts**, in either order with any overlap.
- **Long-press either button, or wait 20 s, to leave** — the game keeps its level
  and its progress bar. The wheel opens on the mode being played, so opening and
  immediately accepting is a no-op.
- Three new non-Mario cues (`sfxMenuOpen`/`Close`/`Accept`) — open rises, close
  is the same three notes falling. Documented in `docs/MARIO-SOUNDS.md`.

**The overlaps, which is where this kind of thing normally goes wrong.** Three
gestures now share two buttons. The decision layer was lifted out of `main.cpp`
into **`firmware/include/ui_gestures.h`** — plain C++, no Arduino — and the four
collisions are named and resolved at the top of that file:

1. *− is both the knob and the menu key.* The sensitivity ramp is **capped at
   1 s**; past that the button is arming the menu. And if the hold becomes a
   menu-open, the margin is **restored to its value when the button went down** —
   reaching for the menu must not quietly desensitise the keyboard on the way in.
2. *Accept vs navigate.* In the menu, navigation fires **on release**, so the
   first of two buttons cannot step the wheel before the second lands. No
   coincidence window is needed at all.
3. *Accept vs smart adjust.* Different states; holding − for 3 s never fires
   smart adjust and holding both never opens the menu.
4. **The one that nearly shipped:** press +, then − on top of it, then release +.
   With "when − went down" as the clock, that release opened the menu on the
   spot — no meter, no warning, a two-finger fidget turning into a mode change.
   The clock is now *when − became the only button down*. Found by writing the
   state machine's own doc comment; it has its own test case.

**Verification** — the board is unplugged, so none of this was tested on it:

- `pio run` green on **all five envs** (a new `emulation-freeplay` env joins
  them): 537 B RAM (26.2 %), 15 450 B flash (50.3 %), up from 487 B / 12 142 B.
- **`firmware/test/run.sh` — 97 + 56 checks, 0 failed.** Two host binaries, plain
  `g++`, no Arduino and no Docker: `ui_gestures_test` drives the button state
  machine through every overlapping timeline (including the 7 ms loop rate the
  AVR really runs at, and contact bounce), and `piano_sim_test` compiles **the
  real `src/main.cpp`** against a fake board (`test/arduino/Arduino.h`) and plays
  it — note mapping, five repeats of one lemon, the wheel reaching every mode and
  wrapping, cancel changing nothing, accept starting clean, the margin restore,
  and winning never landing on free play.
- **Mutation-checked**, because a check that cannot fail proves nothing: breaking
  the free-play scale, deleting the margin restore, and moving the arming line
  each turn the suite red, on the assertion that names them.
- `emulation/piano-mode.yaml` written for free play (`-DSTART_IN_FREE_PLAY`,
  since the browser has no pins left for the two buttons) — **parses and
  compile-checks, but has never been run**: the Velxio harness is not installed
  on this machine. The mode wheel itself is **not emulatable here at all**, by
  the same pin exhaustion; that is why the host tests exist.

**Docs**: new printable instruction sheet `docs/USER-GUIDE.md` and its Spanish
translation `docs/GUIA-DE-USO.es.md` (the repo is English; what gets printed is
read by people who speak Spanish). V5.5's README grows the full gesture map and
an honest verification table; the root README's "how it plays" section was
**stale** — it still described the A7 game-select switch and the D7 restart
button, both gone since 2026-07-28 — and has been rewritten.

## 2026-09-06 — PCB v0.7.1: "Created with ♥ by Multitec." maker's mark (cosmetic)

- **Silk-only, PATCH bump** (v0.7.0 → v0.7.1, per the repo's version rule). No
  footprint, pad, net, hole or outline change: the placements, netlist,
  schematic and every physical gate are the v0.7.0 board unchanged — only
  F.SilkS grew (ADR-038).
- A large **"Created with ♥ by Multitec."** fills the ~39 × 16 mm free
  rectangle the USB-west flip left in the top-left / west block (below the
  C1/C3 caps, above D2/L1, left of the socket, across the left anchor the user
  said to use). Two left-justified lines at 3.5 mm height, 0.40 mm stroke:
  `Created with ♥` / `by Multitec.`.
- **The heart is a filled silk polygon, not a character.** KiCad 9's stroke
  font has no U+2665 glyph — a literal "♥" renders as a tofu box (verified by
  plotting F.Silkscreen with kicad-cli 9.0.9) — so it is drawn as a `gr_poly`
  from the classic heart parametric, scaled to the text height, y negated so
  the point sits at the bottom (KiCad's +Y is down).
- The **left anchor divider** is broken into two segments around the text band
  (y 113.5..124.0) so the line does not cross the letters; the right divider
  stays full.
- **Verification**: cloud `/drc` 0 errors / 0 warnings / 0 unconnected; ERC
  0/0; verify_placement 74 OK, verify_holes (+vision) PASS, geometry_gate
  30/30 — all identical to v0.7.0 because the board is; plus a numeric silk
  check (heart present, both lines on F.SilkS, zero silk over any pad). Release
  `pcb/releases/v0.7.1/lemon-piano-v0.7.1-fab.zip`; renders + 3D + INDEX.md
  regenerated.

## 2026-09-06 — PCB v0.7.0: Nano turned back to USB-WEST for enclosure cable access (LEDs north, keys south)

- **User request** (annotated screenshot of the v0.6.0 render): flip the
  Arduino so the mini-USB points the other way, away from the SENS buttons, so
  the cable can be plugged into the PC **with the Nano seated** once the
  enclosure exists; D2 and L1 moved to leave room for the cable; and, because
  the flip changes which pins face which edge, the LEDs go to the top and the
  key connector to the bottom.
- **What changed on the board** (`pcb/lemon-piano.yaml`, `pcb/tools/build_board.py`,
  `pcb/tools/geometry_gate.py`):
  - `U1`/`U2` trade places (ADR-035): U1 analog = SOUTH row, pin 1 D13 west
    (rot 90); U2 digital = NORTH row, pin 1 TX1 east (rot 270). D12/D13 flank
    the USB at x=132.22. Pin legends, refs and the B.SilkS title moved with it.
  - **LED bar → NORTH edge**, centred, **LED1 at the EAST** (D3 170.7 … D12
    129.3, rot 270 so anodes point south). **Keys header → SOUTH edge**,
    centred, **KEY1 at the WEST** (`KEYS 1 2 3 4 5 6 7 G`). Both follow the pin
    order for a non-crossing fan (ADR-036) — the mock-up had pasted the v0.6.0
    blocks unchanged; honouring that literally would have meant a 45-crossing
    via crossbar under the digital row. **Consequence: the VU meter fills
    right→left with the USB on the left.** Reversing it is a YAML change plus
    a re-route.
  - **USB-cable corridor reinstated** (x < 130.4, y 113..127 part-free on
    F.Cu — the ADR-024 band; supersedes ADR-030). `geometry_gate` checks it
    against real courtyard boxes. The v0.5.0–v0.6.0 "lift the Nano to flash
    it" cost is gone.
  - **Filter folded around the corridor** (ADR-037): C1 ‖ C3 stay adjacent on
    the north row; D2 → (107.0, 129.2) rot 180, J1 → D1 at y=136.0, L1 in its
    own column at (120.0, 134.5). Four parts do not fit one 27 mm row and L1
    cannot stack over D1 in the 13 mm strip, hence the column. `/VRAW` and
    `/+5V` now run ≈ 30 mm north–south side by side (accepted, same reasoning
    as ADR-031).
  - Both geometric-0805 groups inverted their pad numbers again (the ADR-029
    trap): builder assertions moved to KEY pad y=125.4 / cathode pad y=101.97,
    `ground-truth/components.yaml` re-derived from the built board.
- **Verification**: cloud `/drc` **0 errors / 0 warnings / 0 unconnected**;
  ERC 0/0; `verify_placement` 74 OK; `verify_holes` geometric + vision PASS
  (max LOO 0.022 mm); `geometry_gate` 30/30 incl. the new corridor check; an
  independent logical pass over the routed `.kicad_pcb` (pin→net map, both
  fans monotonic, zero pads in the corridor, copper on all 34 nets, legend
  order); all eight renders inspected (overlay photo lands USB-west, every
  part has a 3D body); `build_board` byte-identical across two runs. First
  pipeline run stopped at `verify_placement` exactly as AGENT_PROMPT step 2
  predicts (0805 pad inversion); no board change was needed to fix it.
- **Release**: `pcb/releases/v0.7.0/lemon-piano-v0.7.0-fab.zip` (14 files),
  produced by `/fab` on the very board the gates passed on (not a `--fab`
  re-route, so the release matches the committed `.kicad_pcb`). 3D models
  `pcb/3d/lemon-piano-v0.7.0.{glb,step}` committed. Renders under
  `pcb/renders/v0.7.0-*`, `renders/INDEX.md` regenerated.
- **Docs**: ADR-035/036/037 in `pcb/docs/DECISIONS.md`; `NETLIST.md` board
  frame + pin map (USB-east map kept as history); `DESIGN_STATE.md`;
  `pcb/README.md` (assembly notes: cable corridor, G pin now east, VU
  direction); `AGENT_PROMPT.md` board facts; `overlays/modules.yaml` and
  `ground-truth/holes.yaml` comments/version.
- **Toolchain note**: the local `eda-pcb-designer:latest` Docker image was
  absent on this machine and was rebuilt from the sibling repo (`docker build`,
  ≈5 GB) — no toolkit code changed.

## 2026-09-06 — V5.5: add the amplified speaker that was missing from the diagram

- **`build_v5_5()`** in `tools/wiring_diagrams.py` — the V5.5 wiring diagram was
  re-rendered (same file, `versions/v5.5-power-filter/images/wiring-v5.5.png`,
  **not** a new version) to include the **LM386 amplifier + 4 Ω 3 W speaker**
  that the schematic had left out. 64 nets, DRC 0 violations. The audio stage is
  wired exactly as in the V6 proposal: `D13 → 10 kΩ / 1 kΩ divider (≈ ÷11) →
  1 µF DC block → LM386 IN → 4 Ω speaker`, with the on-board piezo still in
  parallel on D13.
- **The amp is fed from the *unfiltered* 5 V (`vin`, before the TVS/Schottky)**,
  never the filtered rail: an LM386 into 4 Ω pulls hundreds of mA at audio rate,
  and that current crossing the CLC pi would modulate AVcc — the ADC reference
  the 3–4-count (15–20 mV) touch margin is measured against. Accordingly the
  `+5 V` rail now **stops short** of the amp (`v5_x1=2850` on `_board`) instead
  of spanning the canvas, and the canvas widened to 4200×2080 to hold the audio
  block on the right.
- `versions/v5.5-power-filter/README.md` — hardware-delta section, title and the
  verification net count (58 → 64) updated so the doc matches its own diagram.
- **Known doc debt, deliberately left for a `replantear` pass:** the V6 proposal
  still lists the LM386 + speaker as *its* delta vs V5.5, and V5.5's
  `HARDWARE.md` / `CLAUDE.md` / `versions/README.md` still describe V5.5 as
  "V5 + filter, buzzer unchanged". Reconciling the V5.5↔V6 boundary (e.g. V6 =
  battery-only) was explicitly out of scope for this change.

## 2026-08-05 — V6 proposal: battery power + amplified speaker (diagram only)

- **`versions/v6-battery-amp/`** — a **diagram-only proposal**, explicitly not a
  complete version directory: no `firmware/`, no `emulation/`, no `HARDWARE.md`,
  not in the version index tables, never built or measured. Its README lists
  exactly what is missing. The newest real board is still V5.5.
- **`build_v6()`** in `tools/wiring_diagrams.py` — 66 nets, DRC 0 violations.
  V5.5's board and filter unchanged, plus (a) a 1S LiPo 3.7 V / 10 000 mAh on an
  IP5356 power-bank driver module feeding the existing 2-pin `5V IN` header, and
  (b) an LM386 module + 4 Ω 3 W speaker in parallel with the buzzer on
  `/BUZZER`. **The PCB does not change**: ADR-025 already sized `5V IN` for a
  chopped USB-A pigtail and ADR-034 already put `SPK` (J5) across the buzzer, so
  moving the pigtail between the module's USB-A and a wall charger *is* the mode
  switch.
- **The amplifier hangs off the unfiltered 5 V (`vbus`), not the rail** — the
  one decision worth remembering. The CLC pi has fc ≈ 730 Hz, so at 1 kHz it
  only attenuates ~5 dB: audio-band current pulled *through* the filter would
  land almost unattenuated on the rail, which is AVcc, the reference the 3-4
  count (15-20 mV) touch margin is measured against. So the amp taps the module
  before the TVS and shares no series element with the keyboard. Consequently
  the drawing's `+5 V` rail **stops short** of the audio block (new `v5_x0` /
  `v5_x1` args on `_board`) rather than spanning the canvas — it is not there.
- D13 drives a 10 kΩ / 1 kΩ divider (≈ ÷11, 0.45 mA off the pin) and a 1 µF DC
  block, never the coil: a bare 4-8 Ω speaker on D13 would destroy the pin
  (PCB ADR-034). The on-board piezo stays in parallel on the same node.
- The README argues the case a battery makes beyond portability: the V5.5 filter
  is a **series** filter and cannot touch the common-mode path, so on battery the
  board floats *with* the player and that 50 Hz difference cancels in the ADC —
  plus the charger's Y-cap leakage stops flowing through the player's hand. It
  also records the five open risks to measure first, the worst being low-load
  auto-shutdown (the piano idles at 25-35 mA; IP5356-class modules cut out below
  ~45-75 mA, i.e. it would switch off during the silences).
- `keyboard_2019()` gains `dx=0` and `_board()` gains `v5_x0`/`v5_x1`, both
  backward-compatible: re-rendering V1-V5.5 leaves every committed PNG untouched.
  (V5.5's PNG does differ from its committed bytes on this machine, but the
  *pristine* engine and builder reproduce that same 113 885-pixel delta — it is
  pre-existing font-rendering drift, not this change, so it was left alone.)
- Engine side (`../eda-wirewright`): four new component factories — `battery`,
  `power_bank_module`, `amp_module`, `speaker` — plus `deco.panel` for the
  "three power modes" box, with tests. See its CHANGELOG.

