/* mario_sfx.h — Super Mario Bros sound effects (not the level themes).
 *
 * Every sound the piano makes that is NOT a played key note and NOT one of the
 * four level themes lives here (the themes — full transcriptions since
 * 2026-07-29 — are PROGMEM tables in main.cpp, alongside the 2019-era
 * marioNotes/underworldNotes they now match in treatment). The provenance of
 * each sound — sourced verbatim, transcribed from a letter-note tab, or
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

// ── KEY STUCK 🔨 reconstruction ────────────────────────────────────────────
// A locked lemon (same key pressed again before a different one unlocks it,
// see KEY_LOCK_COOLDOWN_MS in main.cpp) played AGAIN after the cooldown: a
// dull rattling triple-hit on one low, muted note — distinct from Bump's two
// separated low tones ("cannot go further") because this is a different
// situation ("this key specifically is locked, try another"), and pitched
// below every level's key range (lowest is level 2's A3) so it is never
// mistaken for a note.
const int sfxKeyStuck[] PROGMEM = {
  NOTE_D3, 45, NOTE_D3, 45, NOTE_D3, 90, 0, 0
};

// ── LEVEL CLEAR (flagpole fanfare) 🔨 reconstruction ───────────────────────
// Plays the moment a level's code is completed, before that level's own theme.
const int sfxLevelClear[] PROGMEM = {
  NOTE_C5, 110, NOTE_E5, 110, NOTE_G5, 110,
  NOTE_C6, 110, NOTE_E6, 110, NOTE_G6, 260, NOTE_E6, 260, 0, 0
};

// ── GAME COMPLETE (castle-clear fanfare, extended) 🔨 reconstruction ───────
// Played on a LOOP once all four levels are cleared (see playEndingLoop() in
// main.cpp), until the player holds both sensitivity buttons for 1 s — so it
// needed to be more than the original ~1.4 s jingle. SMB1's real castle/
// "World Clear" cue could not be found as a verbatim, note-by-note source
// (unlike the coin/1-up/fireball data above) — even dedicated video-game-music
// wikis describe it only as "a short fanfare in the same triumphant idiom as
// the flagpole cue" (the so-called "Mario Cadence": a whole-step-resolving
// arpeggio, shared with the power-up SFX at a slower speed). Extended 2026-07-29
// from the original short version (still phrase 1 below) into three phrases:
// the same call-and-response idiom repeated a third higher (raising the
// stakes), then a descending flourish that resolves back to the tonic and
// loops cleanly.
const int sfxEnding[] PROGMEM = {
  // Phrase 1: call-and-response, C major arpeggio (the original short jingle)
  NOTE_C5, 90, NOTE_E5, 90, NOTE_G5, 90,
  NOTE_C6, 90, NOTE_E6, 90, NOTE_G6, 180,
  0, 60,
  NOTE_G6, 90, NOTE_E6, 90, NOTE_C7, 90,
  NOTE_E6, 90, NOTE_G6, 90, NOTE_C7, 260,
  0, 120,
  // Phrase 2: the same call-and-response, a third higher — raises the stakes
  NOTE_E5, 90, NOTE_G5, 90, NOTE_C6, 90,
  NOTE_E6, 90, NOTE_G6, 90, NOTE_C7, 180,
  0, 60,
  NOTE_C7, 90, NOTE_G6, 90, NOTE_E7, 90,
  NOTE_G6, 90, NOTE_C7, 90, NOTE_E7, 260,
  0, 120,
  // Phrase 3: descending flourish back to the tonic, then a final chord-run —
  // the piece's own cadence, so looping it back to phrase 1 reads as a fresh
  // start rather than a cut-off.
  NOTE_G6, 130, NOTE_F6, 130, NOTE_E6, 130, NOTE_D6, 130, NOTE_C6, 200,
  0, 100,
  NOTE_C5, 70, NOTE_E5, 70, NOTE_G5, 70, NOTE_C6, 70, NOTE_E6, 70, NOTE_G6, 70,
  NOTE_C7, 420,
  0, 0
};

#endif  // MARIO_SFX_H
