/* LEMON PIANO V5 — ten-LED progress bar, GND clip, live sensitivity
   Author : Yupipi93 (Sergio Conejero), 2019 · V5 rework 2026-07-14
   Board rebuilt 2026-07-28: GND-clip keyboard, two sensitivity buttons, no
   restart and no game-select switch.

   CODE 1 (Mario Main Theme):  6,5,6,7,2,5,2,1,3,4
   CODE 2 (Mario Underworld) :  3,6,1,4,2,5,3,6,1,4

   Seven lemons are touch keys on A0..A6. Reproduce the secret 10-note melody. A
   row of TEN green LEDs is the progress bar: each correct note lights the next
   LED; a wrong note blanks the row and the sequence restarts. Light all ten and
   the theme plays, then the game AUTO-ADVANCES to the other one.

   THE KEYBOARD IS THE 2019 ONE AGAIN (2026-07-28). Every analog pin is pulled UP
   to +5 V through 220 Ohm and rests near 1023; the player holds a GND clip, so
   touching a lemon drags that pin DOWN. This replaced the floating +5 V-clip
   keyboard because floating pins could not be read at all: measured on the bench,
   one touch pushed ALL SEVEN channels to the rail, the highest and second-highest
   channel differed by a median of 5 counts, and the idle level wandered ~170
   counts on a ~25 s cycle. A 220 Ohm pull-up gives every pin a hard reference:
   idle 1022-1023, rock steady, and a touch dips it a few counts.

   Sensitivity is a MARGIN from each key's own resting level, not an absolute
   reading, and it is on two buttons so it can be tuned with a hand on the fruit:
     - D7 -> more sensitive (smaller margin)
     - A7 -> less sensitive (bigger margin)
     - both held 1 s WHILE TOUCHING a lemon -> smart adjust: the firmware learns
       the best margin from that real touch.
   At boot it auto-calibrates: it measures every key's baseline AND its idle
   noise, then derives the margin from the noise. The LED bar shows what it is
   doing, and every state change has its own sound.

   Gone in this rebuild: GAME SELECT (A7 is a button now) and RESTART (D7 is a
   button now). The game starts at 1 and auto-advances on every win, so both
   themes are reachable; recalibration is the smart-adjust gesture or a reset.
   The previous V5 board is in git history and CHANGELOG.md.

   Kept: edge-triggered input, one key at a time (strongest channel wins),
   sustained notes, PROGMEM melodies, the victory light show, and the compile-time
   Velxio emulation shim (see emulation/README.md).

   Pin map (rewired 2026-07-28 for buildability — one ascending run for the bar):
     A0..A6  keys 1..7      (220 Ohm pull-up each; player holds GND)
     D2..D11 LEDs 1..10     (LED n on pin n+1 — nothing to look up)
     D12     SENS + button  (to GND, internal pull-up)
     D13     buzzer         (the on-board LED blinks with the audio)
     A7      SENS - button  (to GND + external 10 kOhm pull-up)

   Requires an Arduino Nano (or Mini): A6 is a key and A7 is a button, and the
   classic Uno exposes neither.
*/

#include <Arduino.h>
#include "notes.h"
#include "mario_sfx.h"   // every non-key sound + the level 3/4 themes

//################################
//###########  PINS ##############
//################################
#ifdef VELXIO_EMULATION
#define BUZZER 11  // MUST be D11: Velxio ends a note only on a Timer2 duty->0
                   // event and noTone() leaves OCR2A set, so the note-off is an
                   // explicit `OCR2A = 0` — and OCR2A is Timer2/OC2A = D11. On any
                   // other pin the first note beeps forever (CHANGELOG 2026-07-13).
#else
// Pin map chosen so the board is easy to WIRE and easy to FOLLOW: the ten LEDs are
// one unbroken ascending run, LED n on pin n+1 (LED 1 = D2 ... LED 10 = D11), then
// the two odd men out take the last two digital pins.
#define BUZZER 13        // last pin; the Nano's on-board LED blinks along with the
                         // audio, which is harmless (and a free "sound" indicator)
#define SENS_UP 12       // button to GND (internal pull-up): MORE sensitive
#define SENS_DOWN A7     // button to GND + external 10k pull-up: LESS sensitive.
                         // A7 is analog-in only and has no internal pull-up, hence
                         // the external one; read with analogRead() < 512.
                         //
                         // Why the button is NOT on D13: on many Nanos the on-board
                         // LED hangs off D13 through ~1 kOhm, which fights a ~30 kOhm
                         // internal pull-up and can read as "permanently pressed".
                         // A buzzer does not care about that load, so D13 is its pin
                         // and the button gets D12.
#endif

const uint8_t KEY_COUNT = 7;    // A0..A6
const uint8_t LED_COUNT = 10;   // one green LED per note of the sequence

// The ten green progress LEDs, LED[0] = first note ... LED[9] = tenth note.
#ifdef VELXIO_EMULATION
// The browser has no pins for the sensitivity buttons (see the shim note below),
// so D7/D8 are free for LEDs here; key 7 is on D12 and the buzzer on D11.
const uint8_t LED_PINS[LED_COUNT] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 13};
#else
// Hardware: one contiguous run, LED n -> pin n+1. Wire the bar left to right and
// the pin numbers just count up with it — no gaps to remember.
const uint8_t LED_PINS[LED_COUNT] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
#endif

// ── Velxio emulation input shim (define VELXIO_EMULATION to enable) ─────────
// Good news since the keyboard went back to pull-ups: the browser's pushbutton +
// pull-up IS this polarity (idle ~1023, pressed ~0), so the sensing needs no shim
// at all any more. What still differs:
//   - key 7 moves to D12: avr8js exposes A0..A5 for analog injection, no A6/A7,
//   - buzzer on D11 (see above),
//   - NO sensitivity buttons: ten LEDs + buzzer + key 7 already use all twelve
//     digital lines. The buttons and the smart-adjust gesture are verified on the
//     V2.5 rig instead (../../v2.5-threshold-buttons/), which is the same front
//     end with the bar removed. Boot auto-calibration still runs here.
#ifdef VELXIO_EMULATION
const uint8_t KEY_PINS[KEY_COUNT] = {A0, A1, A2, A3, A4, A5, 12};
#endif

//################################
//#########  CONSTANTS ###########
//################################
const int SEQUENCE_LENGTH = 10;   // notes to guess correctly to win (== LED_COUNT)

// ── Sensitivity ─────────────────────────────────────────────────────────────
// A key counts as touched when its reading has dropped TOUCH_MARGIN counts below
// that key's own resting level. Measured on this rig (2026-07-27): baseline 1022,
// a fruit touch dips to 1018 — a 3-4 count signal, because ~1 MOhm of skin against
// a 220 Ohm pull-up is a lopsided divider. That is where the 2019 sketch's magic
// `<= 1019` came from, and why the knob works in SINGLE counts down here.
int  touchMargin = 4;             // the knob, in ADC counts (auto-set at boot)
const int MARGIN_MIN = 1;
const int MARGIN_MAX = 600;
const int MARGIN_STEP_FINE = 1;      // step at or below MARGIN_COARSE_ABOVE
const int MARGIN_STEP_COARSE = 5;    // step above it
const int MARGIN_COARSE_ABOVE = 20;
const int TOUCH_HYSTERESIS = 2;   // a key is released only when it comes back this
                                  // far ABOVE its threshold (Schmitt trigger), so a
                                  // reading on the line cannot chop up the note.
                                  // Small, because the whole signal is ~4 counts.

// ── Auto-calibration ────────────────────────────────────────────────────────
// The margin is derived from the MEASURED noise: a quiet rig gets a tight margin,
// a noisy one a wide margin, with no dial to turn. Floor 4 = the working point
// measured on the real board.
const uint8_t CAL_SAMPLES = 24;               // ~200 ms for all seven keys
const int NOISE_FACTOR = 2;                   // auto margin = 2 x worst noise
const int AUTO_MARGIN_MIN = 4;                // ...never tighter than this
const unsigned long BASELINE_EVERY_MS = 100;  // idle-drift tracking interval
const int BASELINE_DIVISOR = 8;               // baseline += (reading-baseline)/8
const unsigned long STUCK_MS = 5000;          // stuck-key re-baseline timeout

// ── Smart adjust (both buttons 1 s while touching a lemon) ──────────────────
const unsigned long RECAL_HOLD_MS = 1000;     // hold both this long to trigger it
const unsigned long LEARN_MS = 600;           // total sampling window
const uint8_t LEARN_BURSTS = 4;               // bursts, with a blip + LED between

// ── Buttons ─────────────────────────────────────────────────────────────────
const unsigned long BUTTON_DEBOUNCE_MS = 40;
const unsigned long REPEAT_DELAY_MS = 400;    // hold this long to start ramping
const unsigned long REPEAT_EVERY_MS = 120;    // then one step this often

// ── Sound ───────────────────────────────────────────────────────────────────
const int NOTE_DURATION = 70;     // minimum key tone length; a held key sustains
const int WRONG_TONE_GAP_MS = 60; // silence between the played note and the
                                  // mistake cue, so the two never blur together
const unsigned long SUSTAIN_CAP_MS = 2000;  // how long the wrong tone waits for a
                                  // held key before sounding anyway (a stuck key
                                  // must not freeze the game)
const unsigned long KEY_LOCK_COOLDOWN_MS = 500;  // a locked key (pressed again
                                  // before a different one unlocks it) stays
                                  // silent for this long after release — a
                                  // quick accidental double-tap gets no cue.
                                  // Press it again after the cooldown and it
                                  // plays sfxKeyStuck instead of staying silent.
// UI chirps sit ABOVE every game note (game 1 reaches G7 = 3136 Hz, game 2 runs
// 220..587 Hz), so a state sound can never be mistaken for the piano. 3.3-4.8 kHz
// is also where a piezo is loudest.
const bool UI_SOUNDS = true;
const int UI_TICK_MS = 14;

// ── Silence policy (2026-07-29) ─────────────────────────────────────────────
// A key note SUSTAINS: tone() runs until the lemon is released. So anything the
// piano wants to say afterwards must wait for that note to finish, or it simply
// preempts it mid-sound — which is what made the win fanfare step on the tenth
// note. Three deliberate silences fix it:
const int SFX_GAP_MS = 120;      // hush -> silence -> effect: separates an effect
                                 // from whatever was sounding before it
const int SFX_TAIL_MS = 60;      // silence AFTER every effect, so two effects in a
                                 // row (seven calibration coins) never blur
const int PHRASE_GAP_MS = 350;   // musical pause between phrases: level theme ->
                                 // fanfare -> ending melody
const int SFX_ARTICULATION_MS = 18;  // silence carved out of the END of each note in
                                 // a table. Without it the notes run together and a
                                 // repeated pitch (Starman's F5 F5) sounds like one
                                 // long note — measured: the 7-note fanfare came out
                                 // as a single 1071 ms tone. Taken FROM the note, so
                                 // the tempo written in the table is what you hear.
const int UI_LOW = 3300;
const int UI_MID = 4000;
const int UI_HIGH = 4700;

const int LED_METER_MS = 700;     // how long the bar shows the sensitivity level

const bool serialEnabled = true;  // debug log at 9600 baud

//################################
//#########  MELODIES ############
//################################
// Stored in flash (PROGMEM): read with pgm_read_word. The victory tunes that
// play on completing the game are just the tail of the full themes, so we keep
// ONE copy of each and play it from an offset.

// SUPER MARIO BROS — MAIN THEME (full). Victory plays from MARIO_VICTORY_FROM.
const int marioNotes[] PROGMEM = {
  NOTE_E7, NOTE_E7, 0, NOTE_E7,
  0, NOTE_C7, NOTE_E7, 0,
  NOTE_G7, 0, 0, 0,
  NOTE_G6, 0, 0, 0,

  NOTE_C7, 0, 0, NOTE_G6,
  0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6,
  0, NOTE_AS6, NOTE_A6, 0,   // index 28 = victory cut-in

  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7,
  NOTE_D7, NOTE_B6, 0, 0,

  NOTE_C7, 0, 0, NOTE_G6,
  0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6,
  0, NOTE_AS6, NOTE_A6, 0,

  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7,
  NOTE_D7, NOTE_B6, 0, 0
};
const int marioTempo[] PROGMEM = {
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12
};

// SUPER MARIO BROS — UNDERWORLD THEME (full). Victory from UNDER_VICTORY_FROM.
const int underworldNotes[] PROGMEM = {
  NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
  NOTE_AS3, NOTE_AS4, 0,
  0,
  NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
  NOTE_AS3, NOTE_AS4, 0,      // index 12 = victory cut-in
  0,
  NOTE_F3, NOTE_F4, NOTE_D3, NOTE_D4,
  NOTE_DS3, NOTE_DS4, 0,
  0,
  NOTE_F3, NOTE_F4, NOTE_D3, NOTE_D4,
  NOTE_DS3, NOTE_DS4, 0,
  0, NOTE_DS4, NOTE_CS4, NOTE_D4,
  NOTE_CS4, NOTE_DS4,
  NOTE_DS4, NOTE_GS3,
  NOTE_G3, NOTE_CS4,
  NOTE_C4, NOTE_FS4, NOTE_F4, NOTE_E3, NOTE_AS4, NOTE_A4,
  NOTE_GS4, NOTE_DS4, NOTE_B3,
  NOTE_AS3, NOTE_A3, NOTE_GS3,
  0, 0, 0
};
const int underworldTempo[] PROGMEM = {
  12, 12, 12, 12,
  12, 12, 6,
  3,
  12, 12, 12, 12,
  12, 12, 6,
  3,
  12, 12, 12, 12,
  12, 12, 6,
  3,
  12, 12, 12, 12,
  12, 12, 6,
  6, 18, 18, 18,
  6, 6,
  6, 6,
  6, 6,
  18, 18, 18, 18, 18, 18,
  10, 10, 10,
  10, 10, 10,
  3, 3, 3
};

// SUPER MARIO BROS — CASTLE THEME (full, 2026-07-29, replaces Underwater —
// too hard to recognise). Victory from CASTLE_VICTORY_FROM, intro (level-start
// announce) is the first CASTLE_INTRO_LEN notes. G minor, brisk 2/2 — no
// letter-note tab could be sourced for this one (unlike the others; the
// pianoletternotes site this project otherwise draws on has no Castle entry),
// so it is a 🔨 reconstruction built to match the piece's well-documented key,
// tempo and driving/syncopated character, not a note-for-note transcription —
// see docs/MARIO-SOUNDS.md. A repeating four-pulse "alarm" figure answered by
// a descending phrase (x2), the same shape a step up (x2, victory tail starts
// here), then a driving descent to a final cadence that loops cleanly.
const int castleNotes[] PROGMEM = {
  NOTE_G3, NOTE_G3, NOTE_G3, NOTE_G3,
  NOTE_AS3, NOTE_AS3, NOTE_AS3, NOTE_D4,
  NOTE_C4, NOTE_AS3, NOTE_G3, NOTE_F3,
  NOTE_G3, 0,                                  // index 0-13: intro cuts here
  NOTE_G3, NOTE_G3, NOTE_G3, NOTE_G3,
  NOTE_AS3, NOTE_AS3, NOTE_AS3, NOTE_D4,
  NOTE_C4, NOTE_AS3, NOTE_G3, NOTE_F3,
  NOTE_G3, 0,

  NOTE_A3, NOTE_A3, NOTE_A3, NOTE_A3,          // index 28 = victory cut-in
  NOTE_C4, NOTE_C4, NOTE_C4, NOTE_DS4,
  NOTE_D4, NOTE_C4, NOTE_A3, NOTE_G3,
  NOTE_A3, 0,
  NOTE_A3, NOTE_A3, NOTE_A3, NOTE_A3,
  NOTE_C4, NOTE_C4, NOTE_C4, NOTE_DS4,
  NOTE_D4, NOTE_C4, NOTE_A3, NOTE_G3,
  NOTE_A3, 0,

  NOTE_D4, NOTE_C4, NOTE_AS3, NOTE_G3,
  NOTE_F3, NOTE_D3, NOTE_G3, NOTE_AS3,
  NOTE_D4, NOTE_G4, 0
};
const int castleTempo[] PROGMEM = {
  6, 6, 6, 6,
  6, 6, 6, 3,
  6, 6, 6, 6,
  3, 6,
  6, 6, 6, 6,
  6, 6, 6, 3,
  6, 6, 6, 6,
  3, 6,

  6, 6, 6, 6,
  6, 6, 6, 3,
  6, 6, 6, 6,
  3, 6,
  6, 6, 6, 6,
  6, 6, 6, 3,
  6, 6, 6, 6,
  3, 6,

  6, 6, 6, 6, 6, 6, 6, 6, 6, 2, 3
};

// SUPER MARIO BROS — STARMAN / INVINCIBILITY THEME (full, 2026-07-29). Victory
// from STARMAN_VICTORY_FROM, intro is the first STARMAN_INTRO_LEN notes. The
// real theme is a short vamp repeated for as long as invincibility lasts, so
// "full" here means the validated 2026-07-29 excerpt played through TWICE
// before the closing phrase, rather than inventing new melodic material.
const int starmanNotes[] PROGMEM = {
  NOTE_C6, NOTE_F5, NOTE_F5, NOTE_D5,
  NOTE_F5, NOTE_F5, NOTE_D5, NOTE_F5,
  NOTE_D5, NOTE_F5,                            // index 0-9: intro cuts here
  NOTE_C6, NOTE_F5, NOTE_F5, NOTE_D5,
  NOTE_E5, NOTE_E5, NOTE_C5, NOTE_E5,
  NOTE_E5, NOTE_C5, NOTE_E5, NOTE_C5,

  NOTE_C6, NOTE_F5, NOTE_F5, NOTE_D5,          // index 22 = victory cut-in
  NOTE_F5, NOTE_F5, NOTE_D5, NOTE_F5,
  NOTE_D5, NOTE_F5,
  NOTE_C6, NOTE_F5, NOTE_F5, NOTE_D5,
  NOTE_E5, NOTE_E5, NOTE_C5, NOTE_E5,
  NOTE_E5, NOTE_C5, NOTE_E5, NOTE_C5,

  0,
  NOTE_B5, NOTE_A5, NOTE_G5
};
const int starmanTempo[] PROGMEM = {
  11, 11, 11, 11,
  11, 11, 11, 11,
  11, 11,
  11, 11, 11, 11,
  11, 11, 11, 11,
  11, 11, 11, 11,

  11, 11, 11, 11,
  11, 11, 11, 11,
  11, 11,
  11, 11, 11, 11,
  11, 11, 11, 11,
  11, 11, 11, 11,

  11,
  8, 8, 4
};

#define MARIO_LEN          (sizeof(marioNotes) / sizeof(marioNotes[0]))
#define UNDER_LEN          (sizeof(underworldNotes) / sizeof(underworldNotes[0]))
#define CASTLE_LEN         (sizeof(castleNotes) / sizeof(castleNotes[0]))
#define STARMAN_LEN        (sizeof(starmanNotes) / sizeof(starmanNotes[0]))
const uint8_t MARIO_VICTORY_FROM = 28;   // full theme is 78 notes; cut = tail 50
const uint8_t UNDER_VICTORY_FROM = 12;   // full theme is 56 notes; cut = tail 44
const uint8_t CASTLE_VICTORY_FROM = 28;      // full theme tail (see above)
const uint8_t STARMAN_VICTORY_FROM = 22;     // full theme tail (see above)

// Level-start "announce" — the first few notes of the level's OWN theme,
// played once when a level begins, so the player recognises which of the
// four they are on and can place the secret code from the theme alone.
const uint8_t MARIO_INTRO_LEN = 12;
const uint8_t UNDER_INTRO_LEN = 8;
const uint8_t CASTLE_INTRO_LEN = 14;
const uint8_t STARMAN_INTRO_LEN = 10;

//################################
//#####  SECRET SEQUENCES ########
//################################
// FOUR levels since 2026-07-29. Each level has its own seven key notes and its
// own 10-note secret code. The seven notes of a level MUST be distinct: the game
// recognises a guess by comparing frequencies, so two keys sharing a note would
// be indistinguishable. And no code may repeat a note back-to-back, because a
// repeated press of the same key is filtered as flaky contact.
const uint8_t LEVEL_COUNT = 4;

const int keys[LEVEL_COUNT * KEY_COUNT] = {
  // level 1 — Overworld (the 2019 set)
  NOTE_E6, NOTE_G6, NOTE_A6, NOTE_B6, NOTE_C7, NOTE_E7, NOTE_G7,
  // level 2 — Underworld (the 2019 set)
  NOTE_A3, NOTE_AS3, NOTE_C4, NOTE_A4, NOTE_AS4, NOTE_C5, NOTE_D5,
  // level 3 — Castle: a plain C major run (unchanged since the 2026-07-29
  // theme swap from Underwater — only the win jingle changed, not the keys)
  NOTE_C5, NOTE_CS5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5, NOTE_A5,
  // level 4 — Starman: a plain C major run, for the hammered figure
  NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5, NOTE_A5, NOTE_C6,
};

// Codes, as key numbers 1..7:
//   level 1: 6,5,6,7,2,5,2,1,3,4      (Mario Main Theme — since 2019)
//   level 2: 3,6,1,4,2,5,3,6,1,4      (Underworld — since 2019)
//   level 3: 2,4,6,1,5,3,7,4,2,6      (new)
//   level 4: 5,1,3,7,2,6,4,1,5,3      (new)
const int sequence_1[SEQUENCE_LENGTH] = {NOTE_E7, NOTE_C7, NOTE_E7, NOTE_G7, NOTE_G6, NOTE_C7, NOTE_G6, NOTE_E6, NOTE_A6, NOTE_B6};
const int sequence_2[SEQUENCE_LENGTH] = {NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4, NOTE_AS3, NOTE_AS4, NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4};
const int sequence_3[SEQUENCE_LENGTH] = {NOTE_CS5, NOTE_E5, NOTE_G5, NOTE_C5, NOTE_F5, NOTE_D5, NOTE_A5, NOTE_E5, NOTE_CS5, NOTE_G5};
const int sequence_4[SEQUENCE_LENGTH] = {NOTE_G5, NOTE_C5, NOTE_E5, NOTE_C6, NOTE_D5, NOTE_A5, NOTE_F5, NOTE_C5, NOTE_G5, NOTE_E5};

//################################
//#########  STATE ###############
//################################
int  level = 1;                // 1..LEVEL_COUNT (see the themes above)
int  currentStep = 0;          // how many correct notes so far (index into the sequence)
                               // 0 = free play: any key just sounds its note
unsigned long keyToneMinEndsAt = 0;  // a key note never stops before this millis()
int  activeKey = -1;           // key whose note is sounding right now (-1 = silence)
int  lastCountedKey = -1;      // last key the GAME accepted; pressing it again is
                               // ignored until a different key is pressed
int  lastSoundedKey = -1;      // ...and it does not sound again either
int  pressedNote = 0;          // last note played
unsigned long lastReleaseAt = 0;  // when the locked key (lastSoundedKey) was let
                                  // go — starts the KEY_LOCK_COOLDOWN_MS clock

int  baseline[KEY_COUNT];      // each key's resting level (measured, then tracked)
int  noiseLevel[KEY_COUNT];    // peak-to-peak idle noise, from calibration
unsigned long touchedSince[KEY_COUNT];  // when this key started reading touched
unsigned long lastBaselineTick = 0;
unsigned long ledMeterUntil = 0;        // bar is showing the level until this ms

struct Button {
  uint8_t pin;
  int direction;               // -1 = more sensitive, +1 = less sensitive
  bool pressed;
  unsigned long changedAt;
  unsigned long nextRepeat;
};
#ifndef VELXIO_EMULATION
Button buttons[2] = {
  {SENS_UP,   -1, false, 0, 0},
  {SENS_DOWN, +1, false, 0, 0},
};
unsigned long bothHeldSince = 0;
unsigned long endingHeldSince = 0;  // separate timer: same both-buttons-1s
                                     // gesture, but read only by playEndingLoop()
#endif

//################################
//#######  PROTOTYPES ############
//################################
int  readKey(uint8_t i);
int  thresholdFor(uint8_t i);
bool keyTouched(uint8_t i);
bool keyStillDown(int i);
int  strongestKey();
void autoCalibrate();
void learnFromTouch();
void trackBaselines();
#ifndef VELXIO_EMULATION
void serviceButtons();
void nudgeMargin(int direction);
int  stepSize();
#endif
void showMarginOnBar();
void playTone(int freq, int ms);
void soundCalStart();
void soundCalStep();
void soundCalDone();
void soundTick();
void soundLimit();
void soundLearnBlip();
void soundLearnOk();
void soundLearnFail();
void soundStuck();
void soundKeyStuck();
void resetBoard();
void logGame();
void handleGuess();
void playVictory();
void playLevelIntro();
void playEndingLoop();
bool playSfx(const int *table, bool lightShow = false, bool (*checkAbort)() = nullptr);
void hushBuzzer();
void silenceKeyNote();
void playSong(const int *notes, const int *tempos, uint8_t from, uint8_t length);
void wrongTone();
void startKeyTone(int note);
void stopKeyTone();
void waitKeyRelease(int key);
void allLedsOff();
void allLedsOn();
void buzz(int targetPin, long frequency, long length);
void log(const __FlashStringHelper *msg);


//################################
//###########  SETUP #############
//################################
void setup() {
  if (serialEnabled) {
    Serial.begin(9600);
    Serial.println(F("Lemon Piano V5"));
  }

  for (uint8_t i = 0; i < LED_COUNT; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
  }
  pinMode(BUZZER, OUTPUT);
#ifndef VELXIO_EMULATION
  pinMode(SENS_UP, INPUT_PULLUP);     // button to GND
  // SENS_DOWN is A7: analog-in only, no internal pull-up, so the board carries an
  // external 10k to +5 V and the button pulls it to GND.
#endif
  // A0..A6 need no pinMode for analogRead.

  autoCalibrate();                    // measures baselines + noise, sets the margin
  logGame();
  playLevelIntro();                   // announce level 1 before free play begins
}


//################################
//###########  LOOP ##############
//################################
void loop() {
#ifndef VELXIO_EMULATION
  serviceButtons();      // sensitivity knob + the smart-adjust gesture
#endif
  trackBaselines();      // follow the idle drift while keys are untouched

  // The bar belongs to the sensitivity meter for a moment after a button press;
  // restore the game's progress display when that moment passes.
  if (ledMeterUntil && (long) (ledMeterUntil - millis()) <= 0) {
    ledMeterUntil = 0;
    for (uint8_t i = 0; i < LED_COUNT; i++) {
      digitalWrite(LED_PINS[i], i < currentStep ? HIGH : LOW);
    }
  }

  const int keyboardOffset = (level - 1) * KEY_COUNT;

//################################
//########  READ INPUT ###########
//################################
  // ONE key at a time. While a key is down nothing else can interrupt it — the
  // channels are coupled, so a single finger lifts several of them over their
  // thresholds and any "first index wins" scan would flip between them.
  if (activeKey >= 0) {
    if (keyStillDown(activeKey)) {
      return;              // still held: the note is sounding, nothing to decide
    }
    // RELEASE — let the note reach its minimum length (so a quick tap is still a
    // note), then go silent. The next loop is free to accept a new key.
    long remaining = (long) (keyToneMinEndsAt - millis());
    if (remaining > 0) delay(remaining);
    stopKeyTone();
    lastReleaseAt = millis();  // starts the locked key's KEY_LOCK_COOLDOWN_MS clock
    activeKey = -1;
    return;
  }

  int justPressed = strongestKey();   // the clearest touch this scan, or -1
  if (justPressed >= 0) {
    pressedNote = keys[justPressed + keyboardOffset];
    activeKey = justPressed;
    if (justPressed != lastSoundedKey) {
      startKeyTone(pressedNote); // it's a piano — a fresh key sounds its note and
                                 // keeps sounding while the lemon is touched
    } else {
      // Locked: this key already reached the game and won't again until a
      // different one is played. Stay silent for the first KEY_LOCK_COOLDOWN_MS
      // (a quick accidental double-tap gets no cue) — but a press after that
      // grace window means the player is genuinely stuck, so say so.
      bool stillCoolingDown = (millis() - lastReleaseAt) < KEY_LOCK_COOLDOWN_MS;
      if (!stillCoolingDown) {
        soundKeyStuck();
      }
      if (serialEnabled) {
        Serial.print(F("    key ")); Serial.print(justPressed + 1);
        Serial.println(stillCoolingDown
          ? F(" again - locked (play another key to unlock it)")
          : F(" again - STUCK (play another key to unlock it)"));
      }
    }
#ifdef DEBUG_TOUCH
    Serial.print(F("press key ")); Serial.print(justPressed + 1);
    Serial.print(F("  readings:"));
    for (uint8_t i = 0; i < KEY_COUNT; i++) {
      Serial.print(' ');
      Serial.print(keyTouched(i) ? '*' : ' ');
#ifndef VELXIO_EMULATION
      Serial.print(analogRead(i));
#endif
    }
    Serial.println();
#endif

    // ...but the GAME only sees the first press of a key, and a repeat does not
    // even sound: holding a lemon or tapping it again is the same single event
    // until a DIFFERENT key is played. Flaky fruit contact used to machine-gun
    // both the buzzer and the guesses.
    if (justPressed != lastCountedKey) {
      lastCountedKey = justPressed;
      lastSoundedKey = justPressed;
      handleGuess();
    }
  }
}


//################################
//#########  GAME LOGIC ##########
//################################

// Evaluate the note the player just pressed against the secret sequence and
// drive the ten-LED progress bar.
void handleGuess() {
  const int wonWith = activeKey;      // the key under the finger right now
  const int *sequence = (level == 1) ? sequence_1
                      : (level == 2) ? sequence_2
                      : (level == 3) ? sequence_3
                                     : sequence_4;

  if (pressedNote == sequence[currentStep]) {
    // CORRECT — light this step's LED and advance.
    digitalWrite(LED_PINS[currentStep], HIGH);
    currentStep++;
    if (serialEnabled) {
      Serial.print(F("OK ")); Serial.print(currentStep);
      Serial.print(F("/")); Serial.println(SEQUENCE_LENGTH);
    }

    if (currentStep >= SEQUENCE_LENGTH) {
      // VICTORY — this level's own theme plays out in full first (bar flashing
      // to the beat), THEN the flagpole fanfare, then on to the next level,
      // announced by that level's own intro so the player knows where they
      // landed. Clearing the LAST level plays the ending melody after the
      // fanfare too, then wraps back to level 1 (also announced).
      log(F("WIN"));
      // The tenth note is still sounding under the player's finger. Let it play
      // out, pause, and only then start the theme — otherwise the theme cuts
      // the winning note off mid-sound.
      silenceKeyNote();
      playVictory();
      delay(PHRASE_GAP_MS);
      playSfx(sfxLevelClear, true);
      level++;
      if (level > LEVEL_COUNT) {
        log(F("ALL LEVELS CLEAR"));
        delay(PHRASE_GAP_MS);
        playEndingLoop();   // loops until the reset gesture (hardware) or once
                            // (emulation — no sensitivity buttons to test it)
        level = 1;
      }
      resetBoard();
      // If the player never let go (the release wait is capped so a stuck key
      // cannot hang the game), the still-held lemon must NOT count as the first
      // guess of the next level. Leave it marked as already used: releasing and
      // pressing again is what unlocks it, exactly like any other repeat.
      lastCountedKey = wonWith;
      lastSoundedKey = wonWith;
      logGame();
      delay(PHRASE_GAP_MS);
      playLevelIntro();          // announce the new level before free play resumes
    }
  } else if (currentStep > 0) {
    // WRONG — but only once the sequence has actually started (see below).
    // Let the note the player just pressed FINISH first: the low tone is
    // feedback about that note, so cutting it off hides which key was wrong.
    // With sustain, "finished" means the lemon was let go (capped, so a stuck
    // key cannot freeze the game).
    log(F("WRONG"));
    waitKeyRelease(activeKey);
    delay(WRONG_TONE_GAP_MS);
    allLedsOff();
    wrongTone();
    currentStep = 0;
  }
  // else: currentStep == 0 -> FREE PLAY. The bar is empty and the player is
  // just noodling on the lemons; the note already sounded in readInput() and
  // nothing else happens. No penalty tone until they have found the first note
  // of the sequence, so the piano never scolds you for exploring it.
}

void allLedsOff() {
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    digitalWrite(LED_PINS[i], LOW);
  }
}

void allLedsOn() {
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    digitalWrite(LED_PINS[i], HIGH);
  }
}

// Is key i currently touched/pressed? The single point where the two sensing
// models meet: real hardware = analog rise above the calibrated baseline;
// Velxio emulation = active-low keys (analog-low via pull-ups on A0..A5,
// digital-low on D12 for key 7).
// Four-sample average, as the 2019 rig did: cheap noise rejection on a signal
// only a few counts wide. Key 7 is a digital button in the browser build.
int readKey(uint8_t i) {
#ifdef VELXIO_EMULATION
  if (i == 6) {
    return digitalRead(KEY_PINS[6]) == LOW ? 0 : 1023;
  }
#endif
  long sum = 0;
  for (uint8_t n = 0; n < 4; n++) {
    sum += analogRead(i);
  }
  return (int) (sum / 4);
}

// Where this key's trigger point sits right now: its own resting level, minus the
// margin — the pins are pulled UP, so a touch drags them DOWN.
int thresholdFor(uint8_t i) {
  int v = baseline[i] - touchMargin;
  if (v < 0) v = 0;
  return v;
}

bool keyTouched(uint8_t i) {
  return readKey(i) <= thresholdFor(i);
}

// Is key i STILL down? Release needs a slightly higher bar than press
// (TOUCH_HYSTERESIS), so a reading sitting on the line cannot chop a sustained
// note into pieces or re-trigger guesses.
bool keyStillDown(int i) {
  if (i < 0) return false;
  return readKey((uint8_t) i) <= thresholdFor((uint8_t) i) + TOUCH_HYSTERESIS;
}

// Which key is being touched *most clearly*? The channel that has dropped
// furthest below its own threshold wins, so the lemon under the finger beats any
// neighbour that merely grazed its threshold. -1 if none is below.
int strongestKey() {
  int best = -1;
  long bestDepth = 0;
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    long depth = (long) thresholdFor(i) - readKey(i);
    if (depth >= 0 && depth + 1 > bestDepth) {
      bestDepth = depth + 1;
      best = i;
    }
  }
  return best;
}


//################################
//######  CALIBRATION ############
//################################

// Fast, smart, automatic: measure every key's resting level AND its idle noise,
// then derive the margin from the noise itself. The LED BAR is the progress
// display — one LED per key as it is measured — so the player can see that the
// piano is busy and keep their hands off the fruit.
void autoCalibrate() {
#ifdef VELXIO_EMULATION
  // BOOT GUARD (2026-07-13 fix, and doubly important now): Velxio's first SPICE
  // solve takes a moment, and until it drives our nets every input reads 0/LOW —
  // which with this polarity means "every key touched", so calibration would
  // measure a baseline of 0 and the piano would never work.
  log(F("Emulation build: waiting for circuit solve (inputs idle-high)..."));
  while (readKey(0) < 512 || readKey(6) < 512) {
    delay(10);
  }
  log(F("Inputs idle. Ready to play."));
#endif
  log(F("Auto-calibrating - LEDs running = HANDS OFF THE FRUIT..."));
  soundCalStart();
  allLedsOff();

  int worstNoise = 0;
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    digitalWrite(LED_PINS[i], HIGH);        // key i is being measured
    long sum = 0;
    int lo = 1023, hi = 0;
    for (uint8_t n = 0; n < CAL_SAMPLES; n++) {
      int v = readKey(i);
      sum += v;
      if (v < lo) lo = v;
      if (v > hi) hi = v;
      delay(1);
    }
    baseline[i] = (int) (sum / CAL_SAMPLES);
    noiseLevel[i] = hi - lo;
    if (noiseLevel[i] > worstNoise) worstNoise = noiseLevel[i];
    touchedSince[i] = 0;
    soundCalStep();          // a coin per key: audible progress, hands still off
  }

  int autoMargin = worstNoise * NOISE_FACTOR;
  if (autoMargin < AUTO_MARGIN_MIN) autoMargin = AUTO_MARGIN_MIN;
  if (autoMargin > MARGIN_MAX) autoMargin = MARGIN_MAX;
  touchMargin = autoMargin;

  if (serialEnabled) {
    for (uint8_t i = 0; i < KEY_COUNT; i++) {
      Serial.print(F("  key ")); Serial.print(i + 1);
      Serial.print(F(" baseline=")); Serial.print(baseline[i]);
      Serial.print(F(" noise=")); Serial.print(noiseLevel[i]);
      Serial.print(F(" -> threshold=")); Serial.println(thresholdFor(i));
    }
    Serial.print(F("auto margin=")); Serial.print(touchMargin);
    Serial.print(F("  (worst noise ")); Serial.print(worstNoise);
    Serial.print(F(" x ")); Serial.print(NOISE_FACTOR);
    Serial.print(F(", floor ")); Serial.print(AUTO_MARGIN_MIN);
    Serial.println(F(")"));
  }

  resetBoard();
  soundCalDone();          // all clear: you may touch the fruit
  showMarginOnBar();       // ...and here is the sensitivity it settled on
}

// Follow the idle level slowly while a key is NOT touched, so drift cannot make
// the margin lie. A key stuck "touched" for STUCK_MS is noise, not a finger — it
// is re-baselined so the piano recovers by itself, and says so.
void trackBaselines() {
  unsigned long now = millis();
  if (now - lastBaselineTick < BASELINE_EVERY_MS) return;
  lastBaselineTick = now;

  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    if ((int) i == activeKey) continue;              // sounding: leave it alone
    int v = readKey(i);
    if (v > thresholdFor(i)) {
      touchedSince[i] = 0;
      baseline[i] += (v - baseline[i]) / BASELINE_DIVISOR;
    } else if (touchedSince[i] == 0) {
      touchedSince[i] = now;
    } else if (now - touchedSince[i] > STUCK_MS) {
      if (serialEnabled) {
        Serial.print(F("!!! key ")); Serial.print(i + 1);
        Serial.println(F(" stuck touched - re-baselining it (noise, not a finger?)"));
      }
      baseline[i] = v;
      touchedSince[i] = 0;
      soundStuck();
    }
  }
}

#ifndef VELXIO_EMULATION
// ── Smart adjust: learn the margin from a REAL touch ────────────────────────
// Hold both buttons for 1 s WHILE TOUCHING A LEMON. It watches every channel for
// LEARN_MS, works out which key the finger is on (the biggest drop below that
// key's baseline), measures how far the other channels wander meanwhile (the
// noise floor), and puts the margin halfway between the two — the cleanest
// separation this fruit can currently give. The LED bar fills as it samples. If
// the touch is not clearly above the noise it says so and changes nothing.
void learnFromTouch() {
  log(F("SMART ADJUST - KEEP TOUCHING THE LEMON while the bar fills..."));
  hushBuzzer();      // silent while sampling: the buzzer's own current would ride
                     // into the readings we are about to learn from
  allLedsOff();

  int lo[KEY_COUNT];
  for (uint8_t i = 0; i < KEY_COUNT; i++) lo[i] = 1023;

  for (uint8_t burst = 0; burst < LEARN_BURSTS; burst++) {
    // Sample in bursts with the blip BETWEEN them, never during: the buzzer
    // draws current through the same ground as the analog front end, so a tone
    // playing while we measure would pollute the reading we are learning from.
    unsigned long until = millis() + LEARN_MS / LEARN_BURSTS;
    while ((long) (until - millis()) > 0) {
      for (uint8_t i = 0; i < KEY_COUNT; i++) {
        int v = readKey(i);
        if (v < lo[i]) lo[i] = v;
      }
    }
    for (uint8_t l = 0; l <= burst * 2 && l < LED_COUNT; l++) {
      digitalWrite(LED_PINS[l], HIGH);          // progress on the bar
    }
    soundLearnBlip();
  }

  int dropped[KEY_COUNT];
  int best = -1, bestDepth = 0;
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    dropped[i] = baseline[i] - lo[i];
    if (dropped[i] < 0) dropped[i] = 0;
    if (dropped[i] > bestDepth) { bestDepth = dropped[i]; best = i; }
  }
  int floorNoise = 0;
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    if ((int) i != best && dropped[i] > floorNoise) floorNoise = dropped[i];
  }

  if (serialEnabled) {
    Serial.print(F("  strongest: key ")); Serial.print(best + 1);
    Serial.print(F("  depth=")); Serial.print(bestDepth);
    Serial.print(F("  noise floor (other keys)=")); Serial.println(floorNoise);
  }

  if (best < 0 || bestDepth < floorNoise + 2) {
    log(F("  !! touch not separable from noise - margin unchanged."));
    log(F("     Were you touching a lemon? Is the GND clip in your hand?"));
    soundLearnFail();
  } else {
    int m = (floorNoise + bestDepth) / 2;
    if (m < MARGIN_MIN) m = MARGIN_MIN;
    if (m > MARGIN_MAX) m = MARGIN_MAX;
    touchMargin = m;
    if (serialEnabled) {
      Serial.print(F("  learned margin=")); Serial.print(touchMargin);
      Serial.print(F("  (midway between ")); Serial.print(floorNoise);
      Serial.print(F(" and ")); Serial.print(bestDepth);
      Serial.print(F(")  key ")); Serial.print(best + 1);
      Serial.print(F(" threshold=")); Serial.println(thresholdFor(best));
    }
    soundLearnOk();
  }

  resetBoard();
  showMarginOnBar();
}


//################################
//#########  BUTTONS #############
//################################
int stepSize() {
  return touchMargin > MARGIN_COARSE_ABOVE ? MARGIN_STEP_COARSE : MARGIN_STEP_FINE;
}

void nudgeMargin(int direction) {
  int before = touchMargin;
  touchMargin += (direction < 0 ? -stepSize() : stepSize());
  if (touchMargin < MARGIN_MIN) touchMargin = MARGIN_MIN;
  if (touchMargin > MARGIN_MAX) touchMargin = MARGIN_MAX;

  if (touchMargin == before) {                    // against an end stop
    if (serialEnabled) {
      Serial.print(F("!! margin already at the "));
      Serial.print(before <= MARGIN_MIN ? F("MINIMUM (most sensitive)")
                                        : F("MAXIMUM (least sensitive)"));
      Serial.print(F(" = ")); Serial.println(before);
    }
    soundLimit();
    return;
  }
  soundTick();
  if (serialEnabled) {
    Serial.print(F(">>> margin=")); Serial.print(touchMargin);
    Serial.print(direction < 0 ? F("  (more sensitive)") : F("  (less sensitive)"));
    Serial.print(F("  thresholds now ")); Serial.print(thresholdFor(0));
    Serial.print(F("..")); Serial.println(thresholdFor(KEY_COUNT - 1));
  }
  showMarginOnBar();
}

// SENS_UP is a plain digital pin; SENS_DOWN is A7, analog-in only, so it is read
// with analogRead against its external pull-up.
static bool sensUpDown()   { return digitalRead(SENS_UP) == LOW; }
static bool sensDownDown() { return analogRead(SENS_DOWN) < 512; }

// True once both sensitivity buttons have been held continuously for
// RECAL_HOLD_MS — the SAME gesture and duration as smart adjust, but this
// function is polled only from inside playEndingLoop(), where it means
// something different: "stop celebrating, back to level 1" — no calibration,
// since the player may not be anywhere near the fruit right now.
static bool checkEndingReset() {
  if (sensUpDown() && sensDownDown()) {
    unsigned long now = millis();
    if (endingHeldSince == 0) endingHeldSince = now;
    return (now - endingHeldSince) >= RECAL_HOLD_MS;
  }
  endingHeldSince = 0;
  return false;
}

void serviceButtons() {
  unsigned long now = millis();

  // Both buttons held together = learn the margin from the lemon you are holding.
  if (sensUpDown() && sensDownDown()) {
    if (bothHeldSince == 0) {
      bothHeldSince = now;
    } else if (now - bothHeldSince > RECAL_HOLD_MS) {
      bothHeldSince = 0;
      learnFromTouch();
      for (uint8_t b = 0; b < 2; b++) {           // swallow this press
        buttons[b].pressed = true;
        buttons[b].nextRepeat = now + REPEAT_DELAY_MS * 4;
      }
    }
    return;
  }
  bothHeldSince = 0;

  for (uint8_t b = 0; b < 2; b++) {
    Button &btn = buttons[b];
    bool down = (b == 0) ? sensUpDown() : sensDownDown();

    if (down != btn.pressed) {
      if (now - btn.changedAt < BUTTON_DEBOUNCE_MS) continue;    // bounce
      btn.pressed = down;
      btn.changedAt = now;
      if (down) {
        nudgeMargin(btn.direction);
        btn.nextRepeat = now + REPEAT_DELAY_MS;
      }
    } else if (down && now >= btn.nextRepeat) {
      nudgeMargin(btn.direction);
      btn.nextRepeat = now + REPEAT_EVERY_MS;
    }
  }
}
#endif  // !VELXIO_EMULATION

// The ten-LED bar doubles as a SENSITIVITY METER: how many LEDs are lit shows
// where the margin sits between MARGIN_MIN and 20 (the useful range on fruit),
// so the knob can be read across the room without a serial monitor.
void showMarginOnBar() {
  int lit = ((long) touchMargin * LED_COUNT) / 20;
  if (lit < 1) lit = 1;
  if (lit > LED_COUNT) lit = LED_COUNT;
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    digitalWrite(LED_PINS[i], i < lit ? HIGH : LOW);
  }
  ledMeterUntil = millis() + LED_METER_MS;
}


//################################
// Clear the per-round state: progress, last note, edge latches, and the bar.
void resetBoard() {
  currentStep = 0;
  pressedNote = 0;
  lastCountedKey = -1;   // a new round may legitimately open with the key that
                         // ended the last one
  lastSoundedKey = -1;
  stopKeyTone();
  activeKey = -1;
  allLedsOff();
}

void logGame() {
  if (serialEnabled) {
    Serial.print(F("Level "));
    Serial.println(level);
  }
}


//################################
//#######  SOUND (UI) ############
//################################
// Every non-key sound is a Super Mario Bros effect now (2026-07-29): coins while
// calibrating, a power-up when it is ready, 1-up when the smart adjust learns
// something, the death rattle when it cannot. The tables and their provenance
// live in include/mario_sfx.h and docs/MARIO-SOUNDS.md.

// One tone, both builds. The browser needs the note-off written by hand (Velxio
// ends a note only on a Timer2 duty->0 event and noTone() leaves OCR2A set).
void playTone(int freq, int ms) {
  if (ms <= 0) return;
  if (freq <= 0) {            // a rest: silence for the stated time
    delay(ms);
    return;
  }
  tone(BUZZER, freq);
  delay(ms);
  noTone(BUZZER);
#ifdef VELXIO_EMULATION
  OCR2A = 0;
#endif
}

// Stop a sounding key note RIGHT NOW, without waiting for the lemon to be let go.
// For moments where the player must keep touching (the smart adjust samples while
// they hold a key) and the buzzer has to be quiet anyway — a tone playing during a
// measurement rides into the reading through the shared ground.
void hushBuzzer() {
  stopKeyTone();
  activeKey = -1;
  delay(SFX_GAP_MS);
}

// Let a sounding key note FINISH first — its minimum length, and the player's
// release (capped by SUSTAIN_CAP_MS so a stuck key cannot hang the game) — then a
// short silence. This is what a win or a miss owes the note that caused it.
void silenceKeyNote() {
  if (activeKey >= 0) {
    waitKeyRelease(activeKey);     // sustains, then stops it
  } else {
    stopKeyTone();
  }
  delay(SFX_GAP_MS);
}

// Play a PROGMEM {frequency, milliseconds} table until its {0,0} terminator.
// With lightShow the LED bar steps along with the notes, so a win is visible as
// well as audible. Always leaves SFX_TAIL_MS of silence behind it (unless
// aborted). checkAbort, if given, is polled after every note; the moment it
// returns true, playSfx stops early and returns false — used by playEndingLoop()
// to react to the reset gesture mid-song rather than only between repeats.
// Returns true if the table played to its natural end without being aborted.
bool playSfx(const int *table, bool lightShow, bool (*checkAbort)()) {
  if (!UI_SOUNDS) return true;
  uint8_t led = 0;
  for (uint8_t i = 0; i < 96; i += 2) {
    int freq = (int) pgm_read_word(&table[i]);
    int ms   = (int) pgm_read_word(&table[i + 1]);
    if (freq == 0 && ms == 0) break;         // terminator
    if (lightShow) {
      allLedsOff();
      digitalWrite(LED_PINS[led % LED_COUNT], HIGH);
      led++;
    }
    // Articulate: sound most of the note, then a sliver of silence, so adjacent
    // notes — especially two of the same pitch — are heard as two notes.
    int sounding = ms - SFX_ARTICULATION_MS;
    if (freq == 0 || sounding < 20) {
      playTone(freq, ms);                 // rests and very short blips stay as-is
    } else {
      playTone(freq, sounding);
      delay(ms - sounding);
    }
    if (checkAbort && checkAbort()) {
      if (lightShow) allLedsOff();
      return false;
    }
  }
  if (lightShow) allLedsOff();
  delay(SFX_TAIL_MS);              // never butt two effects up against each other
  return true;
}

// ── the UI vocabulary, in Mario ────────────────────────────────────────────
void soundCalStart()   { playSfx(sfxFireball); }   // "starting, hands off"
void soundCalStep()    { playSfx(sfxCoin); }       // one coin per key measured
void soundCalDone()    { playSfx(sfxPowerUp); }    // "ready" — the mushroom sweep
void soundLimit()      { playSfx(sfxBump); }       // head on a block: end stop
void soundLearnBlip()  { playSfx(sfxCoin); }       // smart adjust, still listening
void soundLearnOk()    { playSfx(sfxOneUp); }      // a genuine gain: 1-up
void soundLearnFail()  { playSfx(sfxDeath); }      // it did not work
void soundStuck()      { playSfx(sfxFireball); }   // odd, but carry on
void soundKeyStuck()   { playSfx(sfxKeyStuck); }   // a locked lemon, pressed again

// The sensitivity tick is the coin's grace note alone — the shortest sound in the
// set — with its PITCH TRACKING THE MARGIN, so holding a button sweeps a
// glissando and you can hear where the setting sits.
void soundTick() {
  if (!UI_SOUNDS) return;
  // Turning the sensitivity down while holding a lemon is normal; chopping that
  // lemon's note in half to acknowledge the press is not. The LED meter still
  // shows the change, so stay quiet instead.
  if (activeKey >= 0) return;
  int m = touchMargin > 60 ? 60 : touchMargin;
  int freq = NOTE_B5 + (long) (60 - m) * (NOTE_E7 - NOTE_B5) / (60 - MARGIN_MIN);
  playTone(freq, UI_TICK_MS);
}


//################################
//#########  AUDIO ###############
//################################

// Start the pressed key's note and LEAVE IT SOUNDING: no duration argument, so
// Timer2 keeps driving the buzzer until stopKeyTone(). The note is guaranteed to
// last at least NOTE_DURATION even if the touch was a fleeting tap.
void startKeyTone(int note) {
  keyToneMinEndsAt = millis() + NOTE_DURATION;
  tone(BUZZER, note);
}

// Silence the sustained note. Velxio's buzzer part only ends a WebAudio note on
// a Timer2 duty->0 event, and noTone() leaves OCR2A set — hence the extra clear
// in the emulation build (same reason emuTone() does it).
void stopKeyTone() {
#ifdef VELXIO_EMULATION
  noTone(BUZZER);
  OCR2A = 0;
#else
  noTone(BUZZER);
#endif
}

// Wait for a held key to be let go, keeping its note sounding, then silence it.
// Capped by SUSTAIN_CAP_MS so a stuck or ghosting key cannot hang the game.
void waitKeyRelease(int key) {
  if (key >= 0) {
    unsigned long giveUpAt = millis() + SUSTAIN_CAP_MS;
    while (keyStillDown(key) && (long) (giveUpAt - millis()) > 0) {
      // hold the note; the buzzer is already sounding it
    }
  }
  long remaining = (long) (keyToneMinEndsAt - millis());
  if (remaining > 0) delay(remaining);
  stopKeyTone();
  activeKey = -1;
}

// "You missed" cue: the short Mario death excerpt (sfxMistake), so a wrong
// note reads unmistakably as "the game says no" rather than as a state beep.
void wrongTone() {
  playSfx(sfxMistake);
}

void playVictory() {
  silenceKeyNote();          // in case we got here without the win path's hush
  switch (level) {
    case 1: playSong(marioNotes, marioTempo, MARIO_VICTORY_FROM, MARIO_LEN); break;
    case 2: playSong(underworldNotes, underworldTempo, UNDER_VICTORY_FROM, UNDER_LEN); break;
    case 3: playSong(castleNotes, castleTempo, CASTLE_VICTORY_FROM, CASTLE_LEN); break;
    default: playSong(starmanNotes, starmanTempo, STARMAN_VICTORY_FROM, STARMAN_LEN); break;
  }
}

// Level-start announce: the first few notes of the CURRENT level's own theme,
// so the player hears which level they are on before touching a lemon. Plays
// once at boot and once every time a level begins (auto-advance or wrap).
void playLevelIntro() {
  hushBuzzer();               // silence + a beat, safe even if nothing was sounding
  switch (level) {
    case 1: playSong(marioNotes, marioTempo, 0, MARIO_INTRO_LEN); break;
    case 2: playSong(underworldNotes, underworldTempo, 0, UNDER_INTRO_LEN); break;
    case 3: playSong(castleNotes, castleTempo, 0, CASTLE_INTRO_LEN); break;
    default: playSong(starmanNotes, starmanTempo, 0, STARMAN_INTRO_LEN); break;
  }
  delay(SFX_TAIL_MS);         // breathing room before free play begins
}

// All four levels cleared: play the game-complete piece (sfxEnding) on a LOOP
// until the player holds both sensitivity buttons for RECAL_HOLD_MS (the same
// gesture/duration as smart adjust) — then returns, so the caller can reset
// straight back to level 1 WITHOUT recalibrating. checkEndingReset() is polled
// between every note of every repeat, so a 1 s hold is honoured almost
// immediately rather than only at the end of a whole loop. Emulation has no
// sensitivity buttons to test that gesture (every digital pin is already a
// LED or a key — see the pin map note above KEY_PINS), so it loops forever
// there too rather than returning — playSfx() with no checkAbort always
// completes normally, so the while below never exits on its own. "Reset" in
// the browser means stopping and re-running the simulation, same as pulling
// power on real hardware; the point of an ending that loops is that it
// actually keeps celebrating, not that it plays once and quietly resets.
void playEndingLoop() {
#ifdef VELXIO_EMULATION
  while (playSfx(sfxEnding, true)) {
    // no reset gesture available here — loop forever, matching hardware
  }
#else
  endingHeldSince = 0;
  while (playSfx(sfxEnding, true, checkEndingReset)) {
    // completed one full loop without the reset firing — play it again
  }
#endif
}

// Play a melody from PROGMEM, notes[from..length). Blocking on purpose — the
// game pauses (with all ten LEDs lit) while the winning theme plays.
void playSong(const int *notes, const int *tempos, uint8_t from, uint8_t length) {
  noTone(BUZZER);  // silence any lingering key tone before bit-banging the pin
  for (uint8_t i = from; i < length; i++) {
    int frequency = (int) pgm_read_word(&notes[i]);
    int tempo = (int) pgm_read_word(&tempos[i]);

    // note duration: one second / note type (quarter = 1000/4, eighth = 1000/8...)
    int noteDuration = 1000 / tempo;

    // Light show: the whole LED bar flashes to the beat — lit while the note
    // sounds (buzz blocks for its duration), dark in the inter-note gap and
    // during rests.
    if (frequency > 0) {
      allLedsOn();
    }
    buzz(BUZZER, frequency, noteDuration);
    allLedsOff();

    // a gap of duration + 30% keeps consecutive notes distinct
    delay((unsigned long)(noteDuration * 1.30));
    buzz(BUZZER, 0, noteDuration);  // stop
  }
}

// Bit-banged square wave used by playSong.
void buzz(int targetPin, long frequency, long length) {
  if (frequency <= 0) {
    return;  // rest / stop — nothing to toggle (and avoids a divide-by-zero)
  }
#ifdef VELXIO_EMULATION
  // Bit-banged toggling is invisible to Velxio's buzzer part (it listens to
  // Timer2 duty, not raw edges) — route through playTone instead.
  (void) targetPin;
  playTone((int) frequency, (int) length);
  return;
#endif
  long delayValue = 1000000 / frequency / 2;    // half-period in microseconds
  long numCycles = frequency * length / 1000;   // cycles for the requested length
  for (long i = 0; i < numCycles; i++) {
    digitalWrite(targetPin, HIGH);   // push the diaphragm out
    delayMicroseconds(delayValue);
    digitalWrite(targetPin, LOW);    // pull it back
    delayMicroseconds(delayValue);
  }
}

#ifdef VELXIO_EMULATION
// Blocking tone that Velxio's buzzer part can both hear and STOP. The part
// starts a WebAudio note when Timer2 duty goes >0 and stops it ONLY on a
// duty->0 event — but noTone() leaves OCR2A set, so without the explicit clear
// the note plays forever (even after the sim stops). frequency <= 0 = rest.
void emuTone(long frequency, long durationMs) {
  if (frequency > 0) {
    tone(BUZZER, frequency);
  }
  delay(durationMs);
  noTone(BUZZER);
  OCR2A = 0;   // duty->0: the buzzer part's only note-off trigger
}
#endif

// Serial log helper — a single guard point for all debug output.
void log(const __FlashStringHelper *msg) {
  if (serialEnabled) {
    Serial.println(msg);
  }
}
