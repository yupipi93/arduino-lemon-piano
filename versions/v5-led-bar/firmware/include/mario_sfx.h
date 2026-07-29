/* mario_sfx.h — Super Mario Bros sound effects and level jingles.
 *
 * Every sound the piano makes that is NOT a played key note lives here, plus the
 * two level themes added in 2026-07-29 (underwater and starman). The provenance
 * of each one — sourced verbatim, transcribed from a letter-note tab, or
 * reconstructed from a description — is documented in docs/MARIO-SOUNDS.md.
 * Please keep that document in step with this file.
 *
 * Format: {frequency, milliseconds} pairs, terminated by {0, 0}. A frequency of
 * 0 is a rest of the stated length. Everything is PROGMEM: these tables live in
 * flash and are read with pgm_read_word, so they cost no RAM.
 *
 * One voice only. The NES played chords, arpeggios and pitch bends across four
 * channels; a piezo on one pin plays the melody line and that is all.
 */
#ifndef MARIO_SFX_H
#define MARIO_SFX_H

#include <Arduino.h>
#include "notes.h"

// ── COIN ✅ sourced ─────────────────────────────────────────────────────────
// B5 grace note resolving up a perfect fourth to E6. The original holds the E for
// ~850 ms; here it is clipped to 120 ms so one coin can mark one calibrated key
// without calibration taking eight seconds.
const int sfxCoin[] PROGMEM = {
  NOTE_B5, 60, NOTE_E6, 120, 0, 0
};

// ── 1-UP ✅ sourced ────────────────────────────────────────────────────────
// Rising C-major-add-9 arpeggio: the sound of gaining something.
const int sfxOneUp[] PROGMEM = {
  NOTE_E6, 125, NOTE_G6, 125, NOTE_E7, 125,
  NOTE_C7, 125, NOTE_D7, 125, NOTE_G7, 125, 0, 0
};

// ── FIREBALL ✅ sourced ────────────────────────────────────────────────────
// Three G tones in 105 ms — a whoosh, used here for "something happened, carry on".
const int sfxFireball[] PROGMEM = {
  NOTE_G4, 35, NOTE_G5, 35, NOTE_G6, 35, 0, 0
};

// ── MUSHROOM POWER-UP 🔨 reconstruction ────────────────────────────────────
// The original sweeps Ab -> Bb -> C chords in about a second. One voice cannot do
// chords, so this runs their roots up two octaves instead.
const int sfxPowerUp[] PROGMEM = {
  NOTE_GS5, 45, NOTE_AS5, 45, NOTE_C6, 45,
  NOTE_GS6, 45, NOTE_AS6, 45, NOTE_C7, 90, 0, 0
};

// ── DEATH 📐 transcribed ───────────────────────────────────────────────────
// The "you died" rattle: a perfect fourth C-F, then the falling figure. Shortened
// from V4's full deathTune (see ../../v4-water-pump/firmware/src/main.cpp).
const int sfxDeath[] PROGMEM = {
  NOTE_C5, 110, NOTE_G4, 110, 0, 60,
  NOTE_F4, 110, NOTE_E4, 110, NOTE_D4, 140, NOTE_C4, 260, 0, 0
};

// ── MISTAKE (short death) 📐 excerpt of sfxDeath ───────────────────────────
// The game's own "wrong note" cue: just the opening C5->G4 "death rattle"
// fourth from sfxDeath above, clipped to two notes so a miss stays snappy and
// does not hold up play the way the fuller sfxDeath (smart-adjust failure) can.
const int sfxMistake[] PROGMEM = {
  NOTE_C5, 90, NOTE_G4, 90, 0, 0
};

// ── BUMP 🔨 reconstruction ─────────────────────────────────────────────────
// Head-on-a-block: the knob is against its end stop and will not move.
const int sfxBump[] PROGMEM = {
  NOTE_A2, 70, 0, 40, NOTE_A2, 70, 0, 0
};

// ── LEVEL CLEAR (flagpole fanfare) 🔨 reconstruction ───────────────────────
// Plays the moment a level's code is completed, before that level's own theme.
const int sfxLevelClear[] PROGMEM = {
  NOTE_C5, 110, NOTE_E5, 110, NOTE_G5, 110,
  NOTE_C6, 110, NOTE_E6, 110, NOTE_G6, 260, NOTE_E6, 260, 0, 0
};

// ── ENDING / GAME OVER 🔨 reconstruction ───────────────────────────────────
// Played once all four levels are cleared, before wrapping back to level 1.
const int sfxEnding[] PROGMEM = {
  NOTE_C6, 150, NOTE_G5, 150, NOTE_E5, 150,
  NOTE_A5, 150, NOTE_B5, 150, NOTE_A5, 150,
  NOTE_GS5, 200, NOTE_AS5, 200, NOTE_GS5, 200,
  NOTE_G5, 400, 0, 0
};


/* ── LEVEL THEMES 3 and 4 ───────────────────────────────────────────────────
 * Levels 1 and 2 (Overworld, Underworld) have full PROGMEM themes in main.cpp,
 * kept from the 2019 rescue. These two are new and use the {freq, ms} format
 * above, as recognisable excerpts rather than full arrangements.
 */

// ── LEVEL 3 · UNDERWATER 📐 transcribed ────────────────────────────────────
// The waltzing water theme. From the letter-note tab's opening bars, top voice:
// D C# C G C C# D D D E F G G — a chromatic slide into a rising arpeggio.
const int themeUnderwater[] PROGMEM = {
  NOTE_D5, 200, NOTE_CS5, 200, NOTE_C5, 200,
  NOTE_G4, 200, NOTE_C5, 200, NOTE_CS5, 200,
  NOTE_D5, 180, NOTE_D5, 180, NOTE_D5, 180,
  NOTE_E5, 200, NOTE_F5, 200, NOTE_G5, 320, NOTE_G5, 320,
  0, 120,
  NOTE_E5, 200, NOTE_F5, 200, NOTE_A5, 200, NOTE_AS5, 200,
  NOTE_B5, 180, NOTE_B5, 180, NOTE_B5, 360, 0, 0
};

// ── LEVEL 4 · STARMAN 📐 transcribed ───────────────────────────────────────
// The invincibility loop: a hammered C-F pair decorated with D, stepping down to
// E. Fast and repetitive — that repetition IS the tune.
const int themeStarman[] PROGMEM = {
  NOTE_C6, 90, NOTE_F5, 90, NOTE_F5, 90, NOTE_D5, 90,
  NOTE_F5, 90, NOTE_F5, 90, NOTE_D5, 90, NOTE_F5, 90,
  NOTE_D5, 90, NOTE_F5, 90,
  NOTE_C6, 90, NOTE_F5, 90, NOTE_F5, 90, NOTE_D5, 90,
  NOTE_E5, 90, NOTE_E5, 90, NOTE_C5, 90, NOTE_E5, 90,
  NOTE_E5, 90, NOTE_C5, 90, NOTE_E5, 90, NOTE_C5, 90,
  0, 90,                                   // a breath before the closing phrase
  NOTE_B5, 120, NOTE_A5, 120, NOTE_G5, 240, 0, 0
};

#endif  // MARIO_SFX_H
