# Super Mario Bros (NES) — sound reference

All the Mario music and sound effects this project plays, in one place: what each
sound *is*, the notes it is made of, and **where the data came from**. Music by
Koji Kondo, Nintendo, 1985.

Every entry is tagged with its provenance, because the accuracy genuinely varies:

| Tag | Meaning |
|---|---|
| ✅ **sourced** | note values and durations taken verbatim from the cited source |
| 📐 **transcribed** | derived here from a cited letter-note tab or score — melody voice only, rhythm simplified for a one-voice buzzer |
| 🔨 **reconstruction** | built here to match a cited *description* of the sound; recognisable, not authoritative |

A single-voice piezo cannot play the NES's chords, arpeggios or pitch bends, so
every entry below is the **melody line only**. Where the original is polyphonic,
that is a real loss of fidelity, not a bug.

Frequencies are the standard Arduino `pitches.h` values (A4 = 440 Hz); this repo's
copy is [../versions/v5-led-bar/firmware/include/notes.h](../versions/v5-led-bar/firmware/include/notes.h).

---

## Sound effects

### Coin ✅ sourced

The "bling" of a collected coin: a grace note resolving up a perfect fourth.

| # | Note | Hz | Duration |
|---|---|---|---|
| 1 | B5 | 988 | 100 ms |
| 2 | E6 | 1319 | 850 ms |

Source: [bikeshedeffect.weebly.com](https://bikeshedeffect.weebly.com/arduino-piezo-sounds.html).
Musically it is an appoggiatura — "B acts as an ornament to the E", a perfect
fourth ([Los Doggies](https://www.losdoggies.com/archives/1302)).
**In this project we shorten note 2 to ~120 ms** so a coin can fire once per
calibrated key without the calibration taking eight seconds.

### 1-Up (extra life) ✅ sourced

A rising C-major-add-9 arpeggio — "twinkles like a newborn baby in your soul".

| # | Note | Hz | Duration |
|---|---|---|---|
| 1 | E6 | 1319 | 125 ms |
| 2 | G6 | 1568 | 125 ms |
| 3 | E7 | 2637 | 125 ms |
| 4 | C7 | 2093 | 125 ms |
| 5 | D7 | 2349 | 125 ms |
| 6 | G7 | 3136 | 125 ms |

Source: [bikeshedeffect.weebly.com](https://bikeshedeffect.weebly.com/arduino-piezo-sounds.html).

### Fireball ✅ sourced

A fast glissando "through three G tones", dominant to the C root.

| # | Note | Hz | Duration |
|---|---|---|---|
| 1 | G4 | 392 | 35 ms |
| 2 | G5 | 784 | 35 ms |
| 3 | G6 | 1568 | 35 ms |

Source: [bikeshedeffect.weebly.com](https://bikeshedeffect.weebly.com/arduino-piezo-sounds.html),
character described by [Los Doggies](https://www.losdoggies.com/archives/1302).

### Mushroom power-up 🔨 reconstruction

The original sweeps three chords — **Ab, Bb, then C** — inside about a second
([Los Doggies](https://www.losdoggies.com/archives/1302)). A monophonic buzzer
cannot play chords, so this is rendered as a fast run over those roots:

| # | Note | Hz | Duration |
|---|---|---|---|
| 1 | GS5 | 831 | 45 ms |
| 2 | AS5 | 932 | 45 ms |
| 3 | C6 | 1047 | 45 ms |
| 4 | GS6 | 1661 | 45 ms |
| 5 | AS6 | 1865 | 45 ms |
| 6 | C7 | 2093 | 90 ms |

⚠️ Chord roots are from the source; the octave doubling and the rhythm are ours.

### Death / "you died" 📐 transcribed

Already in this repo since the 2019 rescue: V4's `deathTune`, ported from score
sheets — see [../versions/v4-water-pump/firmware/src/main.cpp](../versions/v4-water-pump/firmware/src/main.cpp).

```
C4(32) CS4(32) D4(16) rest(4) rest(2)
B3(8) F4(8) rest(8) F4(8) F4(6) E4(6) D4(6) C4(8) E3(8) rest(8) E3(8) C3(8)
```

(Numbers are note *types*: 8 = eighth, 16 = sixteenth, as in the firmware's
`playSong()`.) Cross-check of the shape — a perfect fourth "death rattle" between
C and F — from [Los Doggies](https://www.losdoggies.com/archives/1302); letter-note
version at [pianoletternotes](https://pianoletternotes.blogspot.com/2017/10/super-mario-death-theme.html).

### Level clear / flagpole fanfare 🔨 reconstruction

Plays when a course is completed. Rendered here as the classic rising figure that
resolves on the tonic:

| # | Note | Hz | Duration |
|---|---|---|---|
| 1 | C5 | 523 | 110 ms |
| 2 | E5 | 659 | 110 ms |
| 3 | G5 | 784 | 110 ms |
| 4 | C6 | 1047 | 110 ms |
| 5 | E6 | 1319 | 110 ms |
| 6 | G6 | 1568 | 260 ms |
| 7 | E6 | 1319 | 260 ms |

⚠️ Ours. The original fanfare is polyphonic with a distinctive bass line; this is
the top voice's contour only.

### Game over / all levels finished 🔨 reconstruction

Used here as the **end-of-game** melody after the last level:

| # | Note | Hz | Duration |
|---|---|---|---|
| 1 | C6 | 1047 | 150 ms |
| 2 | G5 | 784 | 150 ms |
| 3 | E5 | 659 | 150 ms |
| 4 | A5 | 880 | 150 ms |
| 5 | B5 | 988 | 150 ms |
| 6 | A5 | 880 | 150 ms |
| 7 | GS5 | 831 | 200 ms |
| 8 | AS5 | 932 | 200 ms |
| 9 | GS5 | 831 | 200 ms |
| 10 | G5 | 784 | 400 ms |

⚠️ Ours, in the spirit of the game-over cadence.

---

## Themes

### 1 · Overworld / Main theme ✅ sourced (in repo since 2019)

The one everybody hums. C major, ~100 bpm
([Wikipedia](https://en.wikipedia.org/wiki/Super_Mario_Bros._theme)).

- **In this repo**: `marioNotes[] / marioTempo[]` in
  [../versions/v5-led-bar/firmware/src/main.cpp](../versions/v5-led-bar/firmware/src/main.cpp)
  (PROGMEM). Ported in 2019 from score sheets; the win jingle is the tail from
  `MARIO_VICTORY_FROM`.
- An independent note+duration transcription (from
  [MuseScore score 2145](https://musescore.com/user/2123/scores/2145)) is in
  [robsoncouto/arduino-songs](https://github.com/robsoncouto/arduino-songs/blob/master/supermariobros/supermariobros.ino),
  which encodes dotted notes as negative durations. Useful cross-check:

```
E5,8 E5,8 rest,8 E5,8 rest,8 C5,8 E5,8    G5,4 rest,4 G4,4 rest,4
C5,-4 G4,8 rest,4 E4,-4    A4,4 B4,4 AS4,8 A4,4
```

### 2 · Underworld / Underground ✅ sourced (in repo since 2019)

- **In this repo**: `underworldNotes[] / underworldTempo[]`, same file, PROGMEM.
  Ported 2019 from score sheets; win jingle from `UNDER_VICTORY_FROM`.
- Opening figure: `C4 C5 A3 A4 AS3 AS4 —` repeated, then the same a fourth down
  (`F3 F4 D3 D4 DS3 DS4 —`). Letter-note cross-check:
  [pianoletternotes](https://pianoletternotes.blogspot.com/2021/07/super-mario-bros-underground-theme.html).

### 3 · Underwater 📐 transcribed

The waltzing 3/4 water level theme.

- Source tab: [pianoletternotes — Underwater Theme](https://pianoletternotes.blogspot.com/2018/06/underwater-theme-super-mario.html)
  (letter-note tablature; **uppercase = black key**, lowercase = white).
- The tab's opening bars, top voice: `d C c g c C d d d e f g g`, i.e.
  **D, C♯, C, G, C, C♯, D D D, E, F, G, G** — a chromatic slide into a rising
  arpeggio. The excerpt this project plays is the first phrase only.
- ⚠️ Melody voice only; the original's swung 3/4 accompaniment is dropped.

### 4 · Starman / invincibility 📐 transcribed

The frantic loop while you are invincible.

- Source tab: [pianoletternotes — Starman Theme](https://pianoletternotes.blogspot.com/2019/10/starman-theme-super-mario.html).
- Characteristic figure, right hand: a repeated `C–F` pair decorated with `D`
  (`c f · f · d f · f · d f d f`) that then steps down to `E`/`c e`, over a bass
  alternating `D`/`A`. The excerpt here keeps the repeated-pair contour.
- ⚠️ Melody voice only; the original's driving bass is what makes it recognisable,
  so a single-voice rendering is a caricature of it.

---

## How this project uses them

| Moment | Sound | Why |
|---|---|---|
| Calibration starts | **Fireball** | short whoosh = "starting, hands off" |
| Each key measured | **Coin** (shortened) | seven coins = seven keys, audible progress |
| Calibration finished | **Power-up** | the classic "you are ready" sweep |
| Sensitivity button | **Coin tick** (first note only) | tiny, non-intrusive, pitch tracks the margin |
| Knob at its end stop | **Bump** (low double) | "cannot go further" |
| Smart adjust listening | **Coin** per sampling burst | progress, and it stays out of the measurement windows |
| Smart adjust learned | **1-Up** | a genuine gain |
| Smart adjust failed | **Death** (short) | it did not work, and nothing changed |
| Key stuck / re-baselined | **Fireball** | something odd happened, keep going |
| Wrong note in the game | low C2 tone | the game's own "no", deliberately unlike the UI chirps |
| **Level complete** | **Level-clear fanfare** | then the level's own theme plays |
| **All levels complete** | **Game-over/ending melody** | then it wraps back to level 1 |

Pitch policy: UI sounds and SFX sit **above** the level note sets where possible so
a state chirp is never mistaken for a played note. The exceptions are deliberate:
the wrong-note tone (C2) and the death jingle sit *below* everything.

## Timing: why the silences are deliberate

A key note **sustains** — `tone()` runs until the lemon is released — so anything
the piano says afterwards has to wait for it, or it preempts the note mid-sound.
That is exactly what made the win fanfare cut off the tenth note. Four constants
in the firmware carry the policy:

| Constant | Value | Purpose |
|---|---|---|
| `SFX_GAP_MS` | 120 ms | between whatever was sounding and the start of an effect |
| `SFX_ARTICULATION_MS` | 18 ms | carved from the END of every note in a table, so adjacent notes — especially two of the same pitch — are heard as two notes |
| `SFX_TAIL_MS` | 60 ms | after every effect, so two in a row (seven calibration coins) never blur |
| `PHRASE_GAP_MS` | 350 ms | musical pause between phrases: fanfare → level theme → ending melody |

Two helpers decide *how* the buzzer is freed:

- **`silenceKeyNote()`** lets the note finish — its minimum length and the player's
  release, capped by `SUSTAIN_CAP_MS` — then pauses. Used by a win and a miss:
  they owe the note that caused them.
- **`hushBuzzer()`** stops it immediately, for the smart adjust, where the player
  must keep touching and the buzzer has to be quiet anyway (its current rides into
  the reading through the shared ground).

Articulation is taken **from** each note, not added to it, so the tempo written in a
table is the tempo you hear. Measured on the emulator after the fix: the winning
note plays out, 262 ms of silence, then the fanfare's seven notes at 92/242 ms with
18 ms between each, then 655 ms before the level theme.

Without articulation the seven-note fanfare measured as a **single 1071 ms tone** —
worth remembering if you add a melody with repeated pitches.

## Adding a sound

1. Add its notes + durations to `mario_sfx.h` in the version's `firmware/include/`.
2. Document it here with a provenance tag and a source link.
3. Map it to a moment in the table above.
4. Rebuild, run the version's emulation specs, and re-flash.

## Sources

- [bikeshedeffect.weebly.com — Arduino Piezo Sounds](https://bikeshedeffect.weebly.com/arduino-piezo-sounds.html) — coin, 1-up, fireball note/duration values
- [Los Doggies — Super Mario Melodies](https://www.losdoggies.com/archives/1302) — musical analysis of the SFX (intervals, chords, key)
- [robsoncouto/arduino-songs](https://github.com/robsoncouto/arduino-songs/blob/master/supermariobros/supermariobros.ino) — overworld theme, note+duration form
- [MuseScore score 2145](https://musescore.com/user/2123/scores/2145) — the score that transcription came from
- [pianoletternotes — Underwater](https://pianoletternotes.blogspot.com/2018/06/underwater-theme-super-mario.html) · [Starman](https://pianoletternotes.blogspot.com/2019/10/starman-theme-super-mario.html) · [Underground](https://pianoletternotes.blogspot.com/2021/07/super-mario-bros-underground-theme.html) · [Death](https://pianoletternotes.blogspot.com/2017/10/super-mario-death-theme.html) — letter-note tabs
- [Wikipedia — Super Mario Bros. theme](https://en.wikipedia.org/wiki/Super_Mario_Bros._theme) — key and tempo
- [Mario Universe SFX soundboard](https://www.mariouniverse.com/sfx-smb/) · [The Sounds Resource](https://sounds.spriters-resource.com/nes/supermariobros/asset/393915/) — original audio, for listening comparison
