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

### Mistake / wrong note 📐 excerpt of Death

The game's own "wrong note" cue, added 2026-07-29 to replace a plain low tone.
Just the opening `C5, G4` pair of the Death jingle above, clipped to two notes
so a miss doesn't stall play the way the fuller Death cue (used for a failed
smart adjust) can.

| # | Note | Hz | Duration |
|---|---|---|---|
| 1 | C5 | 523 | 90 ms |
| 2 | G4 | 392 | 90 ms |

### Key stuck 🔨 reconstruction

Added 2026-07-29: pressing an already-**locked** key (the same lemon again,
before a different one unlocks it — see `KEY_LOCK_COOLDOWN_MS` in main.cpp)
stayed completely silent before this. Now the first repeat within
`KEY_LOCK_COOLDOWN_MS` (500 ms) of release still says nothing — a quick
accidental double-tap shouldn't be scolded — but a press *after* that grace
window plays this: a dull rattling triple-hit on one low, muted note.

| # | Note | Hz | Duration |
|---|---|---|---|
| 1 | D3 | 147 | 45 ms |
| 2 | D3 | 147 | 45 ms |
| 3 | D3 | 147 | 90 ms |

⚠️ Ours. Deliberately unlike Bump's two separated low tones ("cannot go
further") — this is a different situation ("this key specifically is locked,
try another") — and pitched below every level's key range (lowest is level 2's
A3) so it is never mistaken for a note.

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

### Game complete / castle-clear fanfare 🔨 reconstruction

Used here as the **end-of-game** piece, played **on a loop** once all four
levels are cleared (`playEndingLoop()`) until the player holds both
sensitivity buttons for 1 s — see the Timing section below. First replaced
2026-07-29 (the original version was a generic game-over-style descent
unrelated to any specific SMB1 cue) and extended the same day from a ~1.4 s
jingle to the three-phrase piece below, since something meant to loop
needed to be more than one bar.

SMB1 plays a distinct, more triumphant fanfare after a **castle** level
(touching the axe, before "Thank you Mario! But our princess is in another
castle!") than after an ordinary flagpole — sometimes called the "World
Clear" fanfare on fan wikis. A verbatim note-by-note transcription of that
specific cue could not be sourced (unlike the coin/1-up/fireball data above,
which comes with hard numbers) — even dedicated video-game-music references
describe it only in general terms: the same triumphant "Mario Cadence" idiom
as the flagpole fanfare (a whole-step-resolving arpeggio, shared with the
power-up SFX at a slower speed), in C major.

Built to match that description rather than invent something unrelated:
phrase 1 is the original short jingle (the same arpeggio idiom as
`sfxLevelClear`, call-and-response resolving a step higher); phrase 2 repeats
it a third higher, raising the stakes; phrase 3 is a descending flourish back
to the tonic and a final chord-run, so the piece has its own cadence and
looping it back to phrase 1 reads as a fresh start rather than a cut-off.

| Phrase | Notes |
|---|---|
| 1 (original jingle) | C5, E5, G5, C6, E6, G6 (180 ms) · *rest* · G6, E6, C7, E6, G6, C7 (260 ms) |
| 2 (a third higher) | E5, G5, C6, E6, G6, C7 (180 ms) · *rest* · C7, G6, E7, G6, C7, E7 (260 ms) |
| 3 (descent + cadence) | G6, F6, E6, D6, C6 (200 ms) · *rest* · C5, E5, G5, C6, E6, G6, C7 (420 ms) |

Full note-by-note data: `sfxEnding` in `firmware/include/mario_sfx.h`
(44 {frequency, ms} pairs). ⚠️ Ours, built to match a cited description rather
than a verbatim source.

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

### 3 · Castle 🔨 reconstruction (replaced Underwater, 2026-07-29)

The dark, driving fortress-level theme — replaced Underwater the same day the
level themes were made "full", because Underwater's opening (a subtle
chromatic slide) turned out too hard for players to recognise as a distinct
level. Castle is one of SMB1's most recognisable pieces after the Overworld
theme itself. `castleNotes[] / castleTempo[]` in main.cpp, PROGMEM, same
treatment as the other three levels — win jingle from `CASTLE_VICTORY_FROM`,
level-start announce is the first `CASTLE_INTRO_LEN` notes.

- **G minor, 90 BPM, 2/2** ([Super Mario Wiki — Castle BGM](https://www.mariowiki.com/Castle_BGM_(Super_Mario_Bros.))).
  Unlike Overworld/Underworld/Underwater/Starman above, no letter-note tab for
  this piece could be found on pianoletternotes or elsewhere — the wiki
  describes its character (driving, syncopated, built around a bassline
  distinctive enough that even *The Super Mario Bros. Movie* quotes it on its
  own) but gives no note-by-note breakdown.
- Built to match that description rather than transcribed verbatim: a
  repeating four-note "alarm" pulse (`G3 G3 G3 G3`) answered by a descending
  phrase, in G minor, at a brisk tempo — played twice, then the same shape a
  whole step up (also twice — the victory tail starts here), then a driving
  descent to a final cadence that loops cleanly.
- ⚠️ Reconstruction, not a transcription — tagged 🔨 rather than 📐 because,
  unlike the other three themes, there was no tab to transcribe *from*.

### 4 · Starman / invincibility 📐 transcribed (full theme since 2026-07-29)

The frantic loop while you are invincible. `starmanNotes[] / starmanTempo[]`
in main.cpp, PROGMEM — win jingle from `STARMAN_VICTORY_FROM`, level-start
announce is the first `STARMAN_INTRO_LEN` notes.

- Source tab: [pianoletternotes — Starman Theme](https://pianoletternotes.blogspot.com/2019/10/starman-theme-super-mario.html),
  which (unlike the Underwater tab above) explicitly labels its rows `RH`
  (right hand / melody) and `LH` (left hand / bass) — the least ambiguous of
  the tabs this project draws on.
- Characteristic figure, right hand: a repeated `C–F` pair decorated with `D`
  (`c f · f · d f · f · d f d f`) that then steps down to `E`/`c e`, over a bass
  alternating `D`/`A`.
- The real theme is a short vamp that repeats for as long as invincibility
  lasts — there is no "rest of the song" to transcribe further. **"Full"
  here (2026-07-29) means the validated figure played through TWICE**
  (mirroring what the NES actually does — loop the vamp) before the closing
  descending phrase, rather than a longer excerpt of the same ~10 seconds of
  source material repeated under a different name.
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
| Key stuck / re-baselined (noise, not a finger) | **Fireball** | something odd happened, keep going |
| Wrong note in the game | **Mistake** (short Death excerpt) | the game's own "no" — unmistakably Mario, distinct from the UI chirps |
| Locked key pressed again, after 500 ms | **Key Stuck** (low rattle) | "this key specifically is locked, try another" — silent for the first 500 ms so a quick double-tap isn't scolded |
| **A level (re)starts** | the first few notes of **that level's own theme** | so the player recognises which of the four they landed on before touching a lemon |
| **Level complete** | the level's own theme, **then** the Level-clear fanfare | the theme is the pay-off; the fanfare punctuates it |
| **All levels complete** | fanfare, **then** the Game-complete/castle-clear piece **on a loop** | keeps celebrating until the player resets (both sensitivity buttons, 1 s) |

Pitch policy: UI sounds and SFX sit **above** the level note sets where possible so
a state chirp is never mistaken for a played note. The Mistake and Death jingles
are the deliberate exception — they borrow Death's C5/G4 register, which can sit
inside a level's own key range, but they only ever play *after* the game has
already told you the guess was wrong, so there is no ambiguity in context.

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
| `PHRASE_GAP_MS` | 350 ms | musical pause between phrases: level theme → fanfare → ending melody → the next level's own intro |

Two helpers decide *how* the buzzer is freed:

- **`silenceKeyNote()`** lets the note finish — its minimum length and the player's
  release, capped by `SUSTAIN_CAP_MS` — then pauses. Used by a win and a miss:
  they owe the note that caused them.
- **`hushBuzzer()`** stops it immediately, for the smart adjust, where the player
  must keep touching and the buzzer has to be quiet anyway (its current rides into
  the reading through the shared ground); also used by `playLevelIntro()` — safe
  even though nothing is normally sounding at a level start, same defensive pattern
  as `playVictory()`'s own `silenceKeyNote()` call.

**Level-start intro (2026-07-29):** `playLevelIntro()` plays the first few notes
of the *new* current level's theme — `MARIO_INTRO_LEN` / `UNDER_INTRO_LEN` /
`CASTLE_INTRO_LEN` / `STARMAN_INTRO_LEN` notes from index 0 of that level's
table — once at boot (level 1) and once every time a level begins (auto-advance
or the wrap after all four). It reuses the SAME full PROGMEM tables the win
jingle plays from — no extra flash for the intro, just a different pair of
offsets into a table already there. Because it is a blocking `playSong()` call
at the end of `setup()` and at the end of a win's `handleGuess()` branch, a key
touched during it is not queued — see the emulation specs' comments for the
measured delay this adds before free play actually starts.

**Locked-key cooldown (2026-07-29):** `KEY_LOCK_COOLDOWN_MS` (500 ms) gates the
Key Stuck cue above — `lastReleaseAt` records when the locked key was let go,
and a repeat press within the cooldown stays silent (unchanged from before this
feature), while one after it plays `sfxKeyStuck`.

**Ending loop + reset gesture (2026-07-29):** `playEndingLoop()` plays
`sfxEnding` repeatedly via `playSfx()`'s new optional `checkAbort` callback
parameter — polled after every single note (not just between repeats), so
holding both sensitivity buttons for `RECAL_HOLD_MS` (1 s, the same gesture and
duration as smart adjust) is honoured almost immediately rather than only at
the end of a whole loop. That gesture reaching `checkEndingReset()` instead of
`serviceButtons()`'s own handler means something different here: reset straight
to level 1 with **no recalibration** (the player may not be anywhere near the
fruit while the ending is playing). Emulation has no sensitivity buttons — every
digital pin is already a LED or a key — so there it just plays the piece once.

Articulation is taken **from** each note, not added to it, so the tempo written in a
table is the tempo you hear. Measured on the emulator after the fix: the winning
note plays out, 262 ms of silence, then the fanfare's seven notes at 92/242 ms with
18 ms between each, then 655 ms before the level theme.

Without articulation the seven-note fanfare measured as a **single 1071 ms tone** —
worth remembering if you add a melody with repeated pitches.

## Adding a sound

1. Add its notes + durations to `mario_sfx.h` in the version's `firmware/include/`
   (a short UI cue) — or to `main.cpp`'s melody section, alongside
   `marioNotes`/`underworldNotes`/`castleNotes`/`starmanNotes` (a full level
   theme with its own `*_VICTORY_FROM` and `*_INTRO_LEN` offsets).
2. Document it here with a provenance tag and a source link.
3. Map it to a moment in the table above.
4. Rebuild, run the version's emulation specs, and re-flash.

## Sources

- [bikeshedeffect.weebly.com — Arduino Piezo Sounds](https://bikeshedeffect.weebly.com/arduino-piezo-sounds.html) — coin, 1-up, fireball note/duration values
- [Los Doggies — Super Mario Melodies](https://www.losdoggies.com/archives/1302) — musical analysis of the SFX (intervals, chords, key)
- [robsoncouto/arduino-songs](https://github.com/robsoncouto/arduino-songs/blob/master/supermariobros/supermariobros.ino) — overworld theme, note+duration form
- [MuseScore score 2145](https://musescore.com/user/2123/scores/2145) — the score that transcription came from
- [pianoletternotes — Underwater](https://pianoletternotes.blogspot.com/2018/06/underwater-theme-super-mario.html) · [Starman](https://pianoletternotes.blogspot.com/2019/10/starman-theme-super-mario.html) · [Underground](https://pianoletternotes.blogspot.com/2021/07/super-mario-bros-underground-theme.html) · [Death](https://pianoletternotes.blogspot.com/2017/10/super-mario-death-theme.html) — letter-note tabs (Underwater is no longer used in this repo since the Castle swap, but the tab and its reasoning are kept here for reference)
- [Wikipedia — Super Mario Bros. theme](https://en.wikipedia.org/wiki/Super_Mario_Bros._theme) — key, tempo, and the "Mario Cadence" description used for the castle-clear reconstruction
- [Super Mario Wiki — Course Clear](https://www.mariowiki.com/Course_Clear_(Super_Mario_Bros.)) · [World Clear](https://www.mariowiki.com/World_Clear_(Super_Mario_Bros.)) — confirms SMB1 uses a *distinct* fanfare for castle levels, described but not transcribed note-for-note
- [Super Mario Wiki — Castle BGM](https://www.mariowiki.com/Castle_BGM_(Super_Mario_Bros.)) — key (G minor), tempo (90 BPM, 2/2) and character used for level 3's Castle theme reconstruction; no note-by-note tab found for this piece anywhere searched
- [Mario Universe SFX soundboard](https://www.mariouniverse.com/sfx-smb/) · [The Sounds Resource](https://sounds.spriters-resource.com/nes/supermariobros/asset/393915/) — original audio, for listening comparison
