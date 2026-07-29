# V5 — ten-LED progress bar (2026) — newest board

No relays, no pump, no red LED, no fail counter: a row of **ten green LEDs** is
the entire feedback surface. Each correct note lights the next LED, a wrong note
blanks all ten, and lighting the tenth wins — the theme plays with the bar
flashing to the beat, then the game **auto-advances** to the other tune.

> **Rebuilt on 2026-07-28.** This board replaces the original V5 board *in place*,
> at the owner's request, rather than becoming a new version. The previous V5
> (floating +5 V-clip keyboard, A7 game-select switch, D7 restart button) is
> recoverable from git history and described in [../../CHANGELOG.md](../../CHANGELOG.md).

**Hardware delta vs [V4.5](../v4.5-margin-buttons/):**

- **the keyboard returns to the 2019 arrangement**: each key is pulled **up** to
  +5 V through 220 Ω and the player holds a **GND** clip, so a touch drags the
  reading **down**. This is what made the keyboard readable at all — the
  measurements are in [HARDWARE.md](HARDWARE.md);
- **off** the board: the red LED, the two MARGIN buttons, the **game-select
  switch** and the **restart button** (relays and pump left with V4.5);
- **on** the board: **ten green LEDs in one ascending run, D2 → D11** (LED n on
  pin n+1, so the bar wires left to right with nothing to look up) and **two
  sensitivity buttons** — **SENS + on D12**, **SENS − on A7**. The buzzer moves to
  **D13**;
- board: **Arduino Nano only** — A6 is key 7 and A7 is a button, and a classic Uno
  exposes neither.

<div align="center">
<img src="images/wiring-v5.png" alt="V5 wiring diagram" width="92%"/>
</div>

## How it plays

1. **Power on → auto-calibration.** It measures every key's resting level *and*
   its idle noise, **counting up all ten LEDs progressively** as it goes
   (**LEDs filling = hands off the fruit**) — one coin per key measured, seven
   keys spread proportionally across all ten so the bar always finishes full
   regardless of there being only seven keys — then derives the touch margin
   from the measured noise, chirps when done, and shows the chosen sensitivity
   on the bar. The game starts at **game 1** and auto-advances on each win, so
   all four themes are reachable with no switch. Then it **announces the
   level**: the opening notes of that level's own theme play once, so you know
   which of the four you landed on before touching a lemon.
2. **Touch lemons.** Hold the **GND** clip in one hand and touch a lemon with the
   other — your body drags that pin down and the note plays on the buzzer.
3. **Play freely.** While the bar is empty, every key just sounds its note —
   no wrong tone, no penalty. It is a piano; noodle as long as you like.
   **Hold a lemon and its note keeps sounding** for as long as you touch it (a
   quick tap still gets a full 70 ms note), and **pressing the same lemon again
   never counts twice** — only the first press of a key reaches the game, until
   you play a different one. Flaky fruit contact can no longer machine-gun
   guesses. It stays silent for the first 500 ms after you let go (a quick
   accidental double-tap gets no cue), but press the SAME locked key again
   after that and it plays a low **"stuck key" rattle** — a clear "try another
   one" instead of nothing at all.
4. **The puzzle starts when you hit the code's first note** (LED 1 lights). From
   there each correct note lights the next green LED (the bar climbs 1 → 10),
   and a wrong note **blanks all ten** and drops you back to free play. The low
   "wrong" tone plays **after** the note you pressed has finished — you always
   hear which key was wrong, then the buzz.
5. **Victory + auto-advance.** Light all ten → that level's own theme plays in
   full first, **the bar counting back up from empty to all ten** across the
   whole theme (a progressive fill, not a flash — the same "count up" feel as
   calibration), then the **flagpole fanfare**, then the **next level's own
   intro announces it**, before the game moves to the **next level**
   (1 → 2 → 3 → 4 → 1 …). Clearing level 4 plays the **game-complete piece on
   a loop**, after the flagpole fanfare — it keeps celebrating until you hold
   both sensitivity buttons for 1 s, which resets straight back to level 1
   **without recalibrating** (also announced).
6. **Tune sensitivity while you play**: **D12** = more sensitive, **A7** = less
   sensitive, 1-count steps (5 above margin 20), auto-repeat while held. The bar
   shows the level for a moment after each press, and the tick's pitch tracks the
   setting — so you can see *and* hear where it is.
7. **Smart adjust**: hold **both buttons for 1 s while touching a lemon**. It
   works out which lemon your finger is on, measures how far the other channels
   wander meanwhile, and sets the margin midway between the two — the cleanest
   separation your fruit can currently give. The bar fills as it samples; a rising
   triad means it learned, a falling pair means the touch could not be told from
   noise and **nothing changed**.
8. **No restart button.** Recalibrate with the smart-adjust gesture or a reset.
   The SAME both-buttons-1s gesture means something different while the
   game-complete piece is looping: it stops the music and resets to level 1
   without recalibrating (see step 5) rather than learning a new margin.

Every state has its own sound, all of them **above** the game notes (3.3–4.7 kHz,
where a piezo is loudest) so a chirp is never mistaken for a note: calibration
start/finish, button ticks, end-stop, smart-adjust progress/success/failure, and a
warble if a key gets stuck reading touched and is re-baselined.

### Four levels, four themes (spoilers)

Each level has its own seven key notes, its own 10-note code, and its own theme —
played in full (opening announce + win jingle draw from the SAME table) rather
than a short excerpt. Clear level 4 and the **game-complete fanfare** plays
before it wraps back to level 1.

| Level | Theme | Code |
|---|---|---|
| 1 | Overworld / Main Theme | `6, 5, 6, 7, 2, 5, 2, 1, 3, 4` |
| 2 | Underworld | `3, 6, 1, 4, 2, 5, 3, 6, 1, 4` |
| 3 | Starman *(moved here from level 4)* | `2, 4, 6, 1, 5, 3, 7, 4, 2, 6` |
| 4 | Castle *(was Underwater, then moved here from level 3 — makes more sense as the finale)* | `5, 1, 3, 7, 2, 6, 4, 1, 5, 3` |

| Key | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|---|---|---|---|---|---|---|---|
| Level 1 | E6 | G6 | A6 | B6 | C7 | E7 | G7 |
| Level 2 | A3 | A#3 | C4 | A4 | A#4 | C5 | D5 |
| Level 3 | C5 | C#5 | D5 | E5 | F5 | G5 | A5 |
| Level 4 | C5 | D5 | E5 | F5 | G5 | A5 | C6 |

A level's seven notes are always distinct (the game recognises a guess by
frequency), and no code repeats a note back-to-back (a repeated press of the same
key is filtered as flaky contact). Each level's key notes and code have stayed
put through both theme changes — only which theme plays for which level moved.

### The sounds are Mario's

Every non-key sound is a Super Mario Bros effect — see
[../../docs/MARIO-SOUNDS.md](../../docs/MARIO-SOUNDS.md) for the note data and
where each one came from:

| Moment | Sound |
|---|---|
| Calibration starts | fireball whoosh |
| Each key measured | **a coin** (seven coins = seven keys) |
| Calibration finished | **mushroom power-up** |
| Sensitivity button | coin grace-note tick, pitch tracks the margin |
| Knob at its end stop | bump (head on a block) |
| Smart adjust listening | a coin per sampling burst |
| Smart adjust learned | **1-up** |
| Smart adjust failed | **death rattle** |
| Wrong note | **short death rattle** (2 notes) |
| Locked key, pressed again after 500 ms | **"stuck key" rattle** (low, distinct from Bump) |
| A level (re)starts | opening notes of **that level's own theme** |
| Level complete | that level's theme (full), then the **flagpole fanfare** |
| All four levels clear | fanfare, then the **game-complete piece, looping** until reset |

Touch sensing is the V2.5 front end: `threshold = baseline − margin`, with the
baseline measured at boot and tracked while each key is untouched, and the margin
auto-derived as `max(4, 2 × worst measured noise)`. On this rig that lands on
**margin 4 → threshold 1018**, which is the working point measured by hand.

## Firmware

```bash
cd firmware
pio run                          # nanoatmega328 (default, old bootloader)
pio run -e nanoatmega328new      # Nano with the new bootloader
pio run -e emulation             # compile-check the VELXIO_EMULATION shim
pio run -t upload
pio device monitor               # 9600 baud — logs Game/OK n/10/WIN
```

There is no `uno` env: V5 needs A6 (key 7) + A7 (SENS −).

### Pin map at a glance

```
A0..A6   keys 1..7      220 Ω pull-up each; player holds the GND clip
D2..D11  LEDs 1..10     LED n on pin n+1 — one run, in order
D12      SENS +         button to GND (internal pull-up)
D13      buzzer         on-board LED blinks along with the audio
A7       SENS −         button to GND + external 10 kΩ pull-up
```

## Emulation — ✅ verify green

```bash
PIPE=../../../velxio-multi-board-emulator/harness/.venv/bin/velxio-pipeline
$PIPE stack up                    # then import emulation/lemon-piano.vlx at http://localhost:3080/editor
$PIPE run --mode verify --spec emulation/lemon-piano.yaml --out emulation/runs
```

Clickable lemons (or press `1`–`7` on your keyboard), audible buzzer, the ten-LED
bar — **coloured like a VU meter in the browser** (LEDs 1-3 green, 4-6 yellow,
7-8 orange, 9-10 red, 2026-07-29) so the progressive fill reads at a glance;
the real board's ten LEDs are all green (see the BOM), this is an emulation-only
touch. Because ten LEDs use every free browser pin there is **no game-select
switch and no restart button**: it starts at game 1 and auto-advances on each win.
There are **four** headless specs, all green (2026-07-27, +1 on 2026-07-29,
re-timed three times the same day — for the level-intro feature below, for
Underwater→Castle at level 3, then again when Castle moved to level 4 and was
redesigned for recognisability):

| Spec | What it proves |
|---|---|
| `emulation/lemon-piano.yaml` | free play → first note → a miss (`WRONG`) → the full code (with the first note pressed twice, to show the repeat filter doesn't eat real guesses) → `WIN` → auto-advance to `Level 2`, plus the victory light show |
| `emulation/free-play.yaml` | five keys that are *not* the code's first note: the buzzer sings, but `WRONG` and `OK` never appear and LED 1 never lights |
| `emulation/hold-and-repeat.yaml` | one key held 600 ms then tapped three more times: the note sustains with the touch, and four touches produce exactly one `OK 1/10` — no `OK 2/10`, no `WRONG` |
| `emulation/all-levels-win.yaml` | a scripted "virtual button" that plays all **four** levels' secret codes back to back — the only test that exercises levels 3 and 4. Asserts `WIN` -> `Level 2` -> `WIN` -> `Level 3` -> `WIN` -> `Level 4` -> `WIN` -> `ALL LEVELS CLEAR` -> wraps to `Level 1`, zero `WRONG`s |

Every input in every spec above is timed around a blocking call: since
2026-07-29 each level opens with its own **intro announce** (`playLevelIntro()`
— the level's own theme, first few notes, played once) in addition to the
existing victory theme + fanfare, so a key pressed too early is simply never
polled, not queued. See the comments above each spec's `inputs:` for the
measured timings this depends on.

There's also a **live, on-demand** version for manual testing —
`emulation/autoplayer.yaml`: a second Arduino (Uno) sharing the canvas. Type a
level number (1-4) into *that board's own Serial Monitor tab* to arm it (its
4 LEDs show which), then `p`, and it fires that level's code onto the lemons'
own key nodes — as many times as you like, matching whichever level piano is
actually on. Your own lemon clicks still work at the same time, in the same
session — verified live (autoplay a win, then click a lemon by hand right
after: both register correctly, 9/9 repeated runs). (An earlier pass tried
physical PLAY/LEVEL SELECT buttons instead of serial commands; verified live
with a headless-Playwright probe that this Velxio version binds every
pushbutton's click handler to a single global simulator rather than one per
board, so a button on the second board can't reliably drive its own pins —
serial I/O doesn't have that limitation, hence typing instead of clicking.
A later pass fixed manual clicks going dead once the finger wiring correctly
reached piano's pins — a bare wire was a hard short that always beat a real
click, fixed with a 220Ω series resistor on each finger wire. Details in
`emulation/README.md`.) Multi-board specs only run in `--mode interactive` in
this harness (no headless verify/document for them), so open it in the
browser:

```bash
$PIPE run --mode interactive --spec emulation/autoplayer.yaml --out emulation/runs --open
```

Details, pin map and the two boards' full sketches:
[emulation/README.md](emulation/README.md#files).

```bash
$PIPE run --mode verify --spec emulation/free-play.yaml --out emulation/runs
$PIPE run --mode verify --spec emulation/hold-and-repeat.yaml --out emulation/runs
$PIPE run --mode verify --spec emulation/all-levels-win.yaml --out emulation/runs
```

> **A code can never repeat a note back-to-back** now that same-key repeats are
> filtered. Neither of the two codes does (`6,5,6,7,2,5,2,1,3,4` and
> `3,6,1,4,2,5,3,6,1,4`) — check it if you ever add a third.

Full details, key mapping and the pin-map diff:
[emulation/README.md](emulation/README.md).

## Files

| Path | What |
|---|---|
| [firmware/src/main.cpp](firmware/src/main.cpp) | the V5 game (10-LED bar, auto-advance) + Velxio shim |
| [firmware/include/notes.h](firmware/include/notes.h) | note frequency table |
| [emulation/lemon-piano.yaml](emulation/lemon-piano.yaml) | circuit spec (source of truth) |
| [emulation/lemon-piano.vlx](emulation/lemon-piano.vlx) | generated project — import into Velxio and Run |
| [HARDWARE.md](HARDWARE.md) | pin map, BOM, wiring detail |
| [images/wiring-v5.png](images/wiring-v5.png) | wirewright-rendered wiring |

**Next revision:** none yet — this is the newest board. Modify the hardware and
you start V6: the recipe is in
[../../docs/VERSIONING.md](../../docs/VERSIONING.md).
