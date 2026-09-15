/* LEMON PIANO V5.5 — filtered supply, four-level game, and a mode wheel
   Author : Yupipi93 (Sergio Conejero), 2019 · V5 rework 2026-07-14
   Board rebuilt 2026-07-28: GND-clip keyboard, two sensitivity buttons, no
   restart and no game-select switch.
   2026-09-09: FREE PLAY + the MODE WHEEL (see "THE MODE WHEEL" below).
   2026-09-15: free play OPENS WITH A SWEEP and then leaves the bar DARK, and
   TWO LEMONS AT ONCE sound as two notes (see "TWO LEMONS AT ONCE" below).

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

   THE MODE WHEEL (2026-09-09) — the game-select switch comes back, as a
   gesture. HOLD "−" FOR THREE SECONDS and the piano opens a wheel of five
   modes: the four levels, and FREE PLAY. "+" and "−" turn it (it wraps both
   ways), each stop previews itself in sound and light, and BOTH BUTTONS AT
   ONCE accepts. A long press on either button, or twenty seconds of silence,
   leaves it having changed nothing.

   FREE PLAY is the fifth item and is NOT a level: it cannot be reached by
   winning, only chosen here. The seven lemons become do-re-mi-fa-sol-la-si
   (C5..B5), there is no code to find, no wrong note, and — unlike the game —
   THE SAME LEMON MAY BE PLAYED OVER AND OVER, which is the whole point of an
   instrument and exactly what the game's repeat filter forbids.

   TWO LEMONS AT ONCE (2026-09-15) — in FREE PLAY only, a second lemon held
   together with the first is detected and both notes sound. One buzzer cannot
   hold two frequencies, so the chord is played the way every mono chip has
   played one since 1983: the two notes swap the buzzer every CHORD_SWAP_MS,
   fast enough that the ear fuses them into an interval. What is hard here is
   not the sound but the DECISION — the channels are coupled, so one finger
   already drags its neighbours down, and a false chord (one finger warbling
   like two) is far worse than a missed one. chordPartnerFor() is where that
   judgement lives and it is deliberately hard to please.

   Three gestures now share two buttons, so the decision layer was lifted out
   into include/ui_gestures.h — plain C++ with no Arduino in it, driven through
   every overlapping timeline by test/ui_gestures_test.cpp on the host. Read
   that header for the full gesture map and the four collisions it resolves;
   this file only turns its events into sound and light.

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
#ifndef VELXIO_EMULATION
#include "ui_gestures.h" // the two buttons, as one host-tested state machine
#endif

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
const unsigned long CAL_STEP_PAUSE_MS = 150;  // extra pause per key, after its
                                  // coin, purely so the progressive 10-LED
                                  // fill reads as a deliberate count-up rather
                                  // than a blur (2026-07-29). Doesn't touch
                                  // CAL_SAMPLES, so measurement quality is
                                  // unaffected — this is display pacing only.
const unsigned long BASELINE_EVERY_MS = 100;  // idle-drift tracking interval
const int BASELINE_DIVISOR = 8;               // baseline += (reading-baseline)/8
const unsigned long STUCK_MS = 5000;          // stuck-key re-baseline timeout

// ── Smart adjust (both buttons 1 s while touching a lemon) ──────────────────
const unsigned long RECAL_HOLD_MS = 1000;     // hold both this long to trigger it
const unsigned long LEARN_MS = 600;           // total sampling window
const uint8_t LEARN_BURSTS = 4;               // bursts, with a blip + LED between

// ── Buttons ─────────────────────────────────────────────────────────────────
const unsigned long BUTTON_DEBOUNCE_MS = 40;
// The sensitivity ramp is GONE (2026-09-13). Both buttons carry a hold gesture
// now, so a hold must be a mode change and nothing else: one tap, one step.
// Timings live in include/ui_gestures.h, which is what the host test compiles.

// ── Sound ───────────────────────────────────────────────────────────────────
const int NOTE_DURATION = 70;     // minimum key tone length; a held key sustains
const int WRONG_TONE_GAP_MS = 60; // silence between the played note and the
                                  // mistake cue, so the two never blur together
const unsigned long SUSTAIN_CAP_MS = 2000;  // how long the wrong tone waits for a
                                  // held key before sounding anyway (a stuck key
                                  // must not freeze the game)
// A RELEASE HAS TO BE CONFIRMED (2026-09-13). A finger resting on fruit is not
// a clean contact: through 1 MOhm the reading wanders, and a single scan that
// crosses back over the threshold used to end the note — and the next scan,
// finding the finger still there, started a NEW one. Held fruit machine-gunned
// itself, which is what Sergio reported: "si el usuario mantiene la fruta, solo
// tiene que sonar una vez".
//
// So the note survives any dropout shorter than this, and only silence that
// LASTS counts as letting go. It has to stay well under a deliberate re-tap
// (about 150 ms hand-to-hand at speed) or fast playing would lose notes.
const unsigned long RELEASE_CONFIRM_MS = 90;

// ── TWO LEMONS AT ONCE, in free play (2026-09-15) ───────────────────────────
// Sergio: "cuando el usuario toque dos notas a la vez... toque la nota
// correspondiente a ambas notas". Two problems, and the second is the hard one.
//
// SOUNDING two notes on one piezo: tone() owns Timer2 and Timer2 makes exactly
// one square wave, so a real chord is impossible. The two voices take turns
// instead, CHORD_SWAP_MS each. At 10 ms that is 50 handovers a second — above
// the ~20 Hz where the ear stops hearing two alternating notes and starts
// hearing one shimmering interval, and slow enough that C5 still gets five
// whole cycles to establish its pitch. It is the Game Boy's arpeggio trick, and
// tone() reprograms OCR2A in place rather than restarting the pin, so the
// handover costs no click.
//
// DECIDING that there are two fingers is the part that needs care. A touch is
// 3-4 counts on this keyboard and the channels are coupled: one finger already
// pulls its neighbours part of the way down (which is why strongestKey() exists
// at all). So a rule that just asks "are two channels below threshold?" would
// hear a chord every time anyone plays a single note — and a single note that
// warbles like two is much worse than a chord that occasionally fails to
// trigger. Hence THREE independent gates, all of which the second key must pass:
//
//   1. an ABSOLUTE floor stricter than a normal touch: touchMargin +
//      CHORD_EXTRA_COUNTS. Coupling crumbs sit just over the plain margin.
//   2. a RATIO against the finger already down: a second finger makes a dip of
//      the same ORDER as the first, while coupling is a fraction of it. This is
//      the gate that actually separates the two cases.
//   3. EXACTLY ONE key may pass 1 and 2. Three lemons' worth of signal is a
//      hand laid across the fruit, not a chord — refuse the lot.
//
// ...and then the survivor has to hold still for CHORD_CONFIRM_MS, so a spike
// on the way into a single touch cannot open a second voice for one scan.
//
// The two numbers are deliberately conservative and they are the tuning knobs:
// the free-play log prints the dip of BOTH voices every time a chord opens, so
// the real ratio on real fruit can be read off a serial monitor rather than
// guessed at a second time.
const int CHORD_EXTRA_COUNTS = 2;      // gate 1: deeper than a single touch needs
const int CHORD_MIN_RATIO_PCT = 70;    // gate 2: share of the first finger's dip
// ...and the ratio it takes to KEEP a voice, which is lower than the one it took
// to open it — a Schmitt trigger, for the same reason TOUCH_HYSTERESIS is one.
// This number is also how a chord is LET GO, and that is not a detail: while two
// lemons are held, lifting one does not return its channel to its resting level,
// because the finger still on the other one goes on shadowing it. An absolute
// threshold would therefore never see the release at all (it would hold a chord
// until BOTH fingers left). What collapses the moment a finger lifts is the
// RATIO between the two dips, so that is what is watched.
const int CHORD_HOLD_RATIO_PCT = 55;
const unsigned long CHORD_CONFIRM_MS = 24;   // ...held this long before it opens
const unsigned long CHORD_SWAP_MS = 10;      // buzzer handover, per voice

// ── Free play announces itself with a SWEEP (2026-09-15) ────────────────────
// It replaces the two-end idle shape that used to mark the mode: see
// showFreePlayIdle() for why that shape had to go, and playFreePlayEntrySweep()
// for what the animation says.
const int FREE_ENTRY_STEP_MS = 34;   // one frame of the entry animation
const int FREE_ENTRY_HOLD_MS = 90;   // the full bar's beat before it goes dark

// A repeated lemon SOUNDS and says "that did not count" with light instead.
// One LED from the right end to the left, at this pace, then the progress bar
// snaps back — see showRepeatSweep(). Faster than the menu's entry sweep on
// purpose: this one has to read as a flick, not as an announcement.
const int REPEAT_SWEEP_MS = 20;

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

// ── The mode wheel (2026-09-09) ─────────────────────────────────────────────
// Timings for the GESTURES are in include/ui_gestures.h (they are what the host
// test asserts). What lives here is only how the wheel LOOKS and SOUNDS.
const uint8_t MENU_PREVIEW_NOTES = 8;    // cap on a preview melody. The level
                                  // intros run 8-16 notes and playSong stretches
                                  // each one to 2.3x its written length, so the
                                  // Castle intro alone would be 4.6 s — long
                                  // enough that turning the wheel feels broken.
                                  // Eight notes is ~1.5-2.3 s: still the tune,
                                  // still recognisable, and interruptible.
const int MENU_BLINK_ON_MS = 420; // the selected item BLINKS. A steady bar is a
const int MENU_BLINK_OFF_MS = 180;// score; a blinking bar is a question. The
                                  // difference has to be visible from a metre
                                  // away with no legend to read.
const int MENU_ENTER_SWEEP_MS = 26;  // the "something happened" sweep on entry
const int ARM_TICK_LOW = 3300;    // the charge-up chirps while − is held: the
const int ARM_TICK_HIGH = 4700;   // pitch rises with the meter, so the gesture
const int ARM_TICK_MS = 18;       // is audible as well as visible

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

// SUPER MARIO BROS — CASTLE THEME (full, redesigned 2026-07-29 for
// recognisability — a player found the first version too generic). Victory
// from CASTLE_VICTORY_FROM, intro (level-start announce) is the first
// CASTLE_INTRO_LEN notes. G minor, brisk. No letter-note tab could be sourced
// for this one (unlike the others), so it is a 🔨 reconstruction — but this
// version is built around the two traits every description of the real piece
// agrees on: a fast alternating "pedal" hook (Super Mario Wiki's own trivia
// notes the opening famously echoes The Twilight Zone's repeated-note theme),
// answered by a chromatic descending run (the "danger" quality every analysis
// calls out) — see docs/MARIO-SOUNDS.md. The verse (hook + answer) repeats,
// then the same verse a fourth higher (victory tail starts here, a common
// Nintendo device for raising tension), then a descending coda that resolves
// hard on the tonic to loop cleanly.
const int castleNotes[] PROGMEM = {
  // verse in G minor, x2 — hook (fast alternating pedal) then chromatic answer
  NOTE_G3, NOTE_D4, NOTE_G3, NOTE_D4, NOTE_AS3, NOTE_D4, NOTE_AS3, NOTE_D4,
  NOTE_D4, NOTE_CS4, NOTE_C4, NOTE_B3, NOTE_AS3, NOTE_A3, NOTE_G3, NOTE_FS3,
                                                // index 0-15: intro cuts here
  NOTE_G3, NOTE_D4, NOTE_G3, NOTE_D4, NOTE_AS3, NOTE_D4, NOTE_AS3, NOTE_D4,
  NOTE_D4, NOTE_CS4, NOTE_C4, NOTE_B3, NOTE_AS3, NOTE_A3, NOTE_G3, NOTE_FS3,

  // the SAME verse a fourth higher (G -> C) — victory cut-in, x2
  NOTE_C4, NOTE_G4, NOTE_C4, NOTE_G4, NOTE_DS4, NOTE_G4, NOTE_DS4, NOTE_G4,
  NOTE_G4, NOTE_FS4, NOTE_F4, NOTE_E4, NOTE_DS4, NOTE_D4, NOTE_C4, NOTE_B3,
  NOTE_C4, NOTE_G4, NOTE_C4, NOTE_G4, NOTE_DS4, NOTE_G4, NOTE_DS4, NOTE_G4,
  NOTE_G4, NOTE_FS4, NOTE_F4, NOTE_E4, NOTE_DS4, NOTE_D4, NOTE_C4, NOTE_B3,

  // coda: descending run back to the tonic, resolved hard so the loop reads
  // as a fresh start rather than a cut-off
  NOTE_D4, NOTE_C4, NOTE_AS3, NOTE_G3, NOTE_F3, NOTE_DS3, NOTE_D3, NOTE_G3
};
const int castleTempo[] PROGMEM = {
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 4,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 4,

  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 4,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 4,

  8, 8, 8, 8, 8, 8, 8, 2
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
const uint8_t CASTLE_VICTORY_FROM = 32;      // full theme tail (see above)
const uint8_t STARMAN_VICTORY_FROM = 22;     // full theme tail (see above)

// Level-start "announce" — the first few notes of the level's OWN theme,
// played once when a level begins, so the player recognises which of the
// four they are on and can place the secret code from the theme alone.
const uint8_t MARIO_INTRO_LEN = 12;
const uint8_t UNDER_INTRO_LEN = 8;
const uint8_t CASTLE_INTRO_LEN = 16;
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

// FREE PLAY is the fifth entry in `keys`, but it is NOT a fifth level: nothing
// advances into it and `handleGuess()` refuses to score in it.
//
// AND IT IS NO LONGER ON THE WHEEL (2026-09-13). Sergio asked for it on its own
// button: holding + swaps between the instrument and the game, the way a piano
// has a switch rather than a menu entry. The wheel is now four levels and
// nothing else, so turning it can never take the instrument away by accident.
const int FREE_PLAY = LEVEL_COUNT + 1;
const uint8_t MENU_ITEM_COUNT = LEVEL_COUNT;   // the four levels. Free play is a hold on +.

const int keys[(LEVEL_COUNT + 1) * KEY_COUNT] = {
  // level 1 — Overworld (the 2019 set)
  NOTE_E6, NOTE_G6, NOTE_A6, NOTE_B6, NOTE_C7, NOTE_E7, NOTE_G7,
  // level 2 — Underworld (the 2019 set)
  NOTE_A3, NOTE_AS3, NOTE_C4, NOTE_A4, NOTE_AS4, NOTE_C5, NOTE_D5,
  // level 3 — Starman theme (moved here from level 4, same day): a plain C
  // major run (unchanged since the 2026-07-29 Underwater->Castle swap and the
  // later level 3/4 theme swap — only which theme plays here has ever changed)
  NOTE_C5, NOTE_CS5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5, NOTE_A5,
  // level 4 — Castle theme (moved here from level 3, same day: makes more
  // sense as the last level): a plain C major run, for the hammered figure
  NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5, NOTE_A5, NOTE_C6,
  // FREE PLAY — one octave of the plain white-key scale: do re mi fa sol la si
  // (C5..B5), left to right. Deliberately NOT one of the level rows above:
  // those are puzzle alphabets picked to make ten-note codes work (level 3
  // carries a C#, level 4 skips B for a top C), and someone who sits down to
  // play a piano expects the seven notes they were taught at school, in order.
  // The rule that a level's seven notes must be distinct does not apply here —
  // nothing is compared against anything — but they are anyway.
  NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5, NOTE_A5, NOTE_B5,
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
int  level = 1;                // 1..LEVEL_COUNT, or FREE_PLAY (the mode wheel)

// The single question everything else asks: is this an instrument or a game?
static inline bool freePlay() { return level == FREE_PLAY; }

// Which note lemon k plays in the mode we are in. Every row of `keys` is the
// same length, so the mode IS the offset.
static inline int noteForKey(int k) { return keys[(level - 1) * KEY_COUNT + k]; }

// Where holding + puts you back. Free play is a detour, not a destination: the
// level you were playing waits for you, and a player who never chose a level
// gets level 1, which is what "a switch between free play and level 1" means
// for everyone who never touched the wheel.
int  levelBeforeFreePlay = 1;

int  currentStep = 0;          // how many correct notes so far (index into the sequence)
                               // 0 = free play: any key just sounds its note
unsigned long keyToneMinEndsAt = 0;  // a key note never stops before this millis()
int  activeKey = -1;           // key whose note is sounding right now (-1 = silence)
unsigned long releaseSeenAt = 0;  // when activeKey first read clear — 0 = it did not
int  lastCountedKey = -1;      // last key the GAME accepted; pressing it again
                               // sounds, shows the repeat sweep, and scores
                               // nothing until a DIFFERENT key is played
int  pressedNote = 0;          // last note played

// ── the second voice (free play only) ──────────────────────────────────────
int  chordKey = -1;            // the lemon sharing the buzzer, or -1 for none
int  chordCandidate = -1;      // one that is passing the gates but not yet held
unsigned long chordCandidateSince = 0;
unsigned long chordReleaseSeenAt = 0;   // same confirmed-release rule as activeKey
unsigned long chordSwapAt = 0;          // last buzzer handover
bool chordOnSecond = false;             // which voice has the buzzer right now
int  chordLeaving = -1;                 // 0 = the first voice looks gone, 1 = the second

int  baseline[KEY_COUNT];      // each key's resting level (measured, then tracked)
int  noiseLevel[KEY_COUNT];    // peak-to-peak idle noise, from calibration
unsigned long touchedSince[KEY_COUNT];  // when this key started reading touched
unsigned long lastBaselineTick = 0;
unsigned long ledMeterUntil = 0;        // bar is showing the level until this ms

#ifndef VELXIO_EMULATION
// The two buttons are no longer read as two independent knobs: three gestures
// share them now, so one state machine owns the decision (include/ui_gestures.h)
// and this file only reacts to the events it emits. `buttons[]` and the manual
// both-held timer it used to need are gone with it.
UiGestures gestures;
// Both buttons nudge the knob on PRESS and can still turn out to be a hold, so
// each one carries a snapshot of the margin taken as it went down. The gesture
// that fires puts its own snapshot back — see collision 1 in ui_gestures.h.
int  marginBeforePlus = 0, marginBeforeMinus = 0;
bool plusWasDown = false, minusWasDown = false;
unsigned long menuAnimAt = 0;     // last menu animation frame
bool menuBlinkOn = true;
unsigned long endingHeldSince = 0;  // separate timer: the same both-buttons-1s
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
void enterMenu();
void serviceMenu();
void announceMenuItem();
void acceptMenuItem();
void cancelMenu();
void showMenuItem(uint8_t item, bool on);
void showArmBar(uint8_t pct);
void soundArmTick(uint8_t pct);
void playMenuPreview(uint8_t item);
static bool menuInterrupt();
#endif
void showMarginOnBar();
void showFreePlayIdle();
void showPitchBar(int key);
void showChordBar(int a, int b);
void playFreePlayEntrySweep();
int  keyDip(uint8_t i);
int  chordPartnerFor(int a);
void serviceChord();
void serviceChordTone();
void startChord(int partner);
void endChord();
void promoteChordVoice();
void clearChord();
void restoreIdleDisplay();
void playFreePlayFlourish(bool lights);
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
void showRepeatSweep();
void resetBoard();
void logGame();
void toggleFreePlay();
void handleGuess();
void playVictory();
void playLevelIntro();
void playEndingLoop();
bool playSfx(const int *table, bool lightShow = false, bool (*checkAbort)() = nullptr);
void hushBuzzer();
void silenceKeyNote();
void playSong(const int *notes, const int *tempos, uint8_t from, uint8_t length,
              uint16_t ledTotal = 0, uint16_t ledOffset = 0, bool lights = true,
              bool (*checkAbort)() = nullptr);
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
    Serial.println(F("Lemon Piano V5.5"));
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

#ifndef VELXIO_EMULATION
  gestures.begin(MENU_ITEM_COUNT);    // one owner for all three button gestures
#endif
#ifdef START_IN_FREE_PLAY
  // Emulation/bench build only: the browser has no pins left for the buttons,
  // so the wheel cannot be reached there. This boots straight into the piano so
  // free play itself can still be regression-tested (emulation/piano-mode.yaml).
  level = FREE_PLAY;
#endif

  autoCalibrate();                    // measures baselines + noise, sets the margin
  logGame();
  playLevelIntro();                   // announce the mode before play begins
}


//################################
//###########  LOOP ##############
//################################
void loop() {
#ifndef VELXIO_EMULATION
  serviceButtons();      // sensitivity knob, smart adjust, and the mode wheel

  // While the wheel is turning the LEMONS ARE DEAD. A preview melody and a
  // played note share one buzzer, and a piano that answers the fruit while it
  // is asking you a question is a piano nobody can read. Baselines keep
  // tracking, so a long browse does not leave the keyboard mis-calibrated.
  if (gestures.inMenu()) {
    serviceMenu();
    trackBaselines();
    return;
  }
#endif
  trackBaselines();      // follow the idle drift while keys are untouched

  // The bar belongs to the sensitivity meter for a moment after a button press;
  // restore whatever the current mode shows when idle once that moment passes.
  if (ledMeterUntil && (long) (ledMeterUntil - millis()) <= 0) {
    ledMeterUntil = 0;
    restoreIdleDisplay();
  }

  const int keyboardOffset = (level - 1) * KEY_COUNT;

//################################
//########  READ INPUT ###########
//################################
  // ONE key at a time. While a key is down nothing else can interrupt it — the
  // channels are coupled, so a single finger lifts several of them over their
  // thresholds and any "first index wins" scan would flip between them.
  if (activeKey >= 0) {
    // TWO LEMONS SOUNDING: the chord answers for both of them, including which
    // one was let go. It cannot be decided here, because keyStillDown() is an
    // absolute threshold and a lifted finger does not cross one while the other
    // finger is still shadowing its channel — see serviceChord().
    if (chordKey >= 0) { serviceChord(); return; }

    if (keyStillDown(activeKey)) {
      releaseSeenAt = 0;   // whatever that was, it was not letting go
      serviceChord();      // free play only: a SECOND lemon may join this note
      return;              // still held: the note is sounding, nothing to decide
    }
    // It read clear — but ONE scan is not a release (see RELEASE_CONFIRM_MS).
    // Hold the note and the key while the silence proves itself; a finger that
    // is still on the fruit comes back within a scan or two and nothing happened.
    if (releaseSeenAt == 0) releaseSeenAt = millis();
    if ((millis() - releaseSeenAt) < RELEASE_CONFIRM_MS) return;
    releaseSeenAt = 0;

    // RELEASE — let the note reach its minimum length (so a quick tap is still a
    // note), then go silent. The next loop is free to accept a new key.
    long remaining = (long) (keyToneMinEndsAt - millis());
    if (remaining > 0) delay(remaining);
    stopKeyTone();
    activeKey = -1;
    if (freePlay() && !ledMeterUntil) showFreePlayIdle();
    return;
  }

  releaseSeenAt = 0;                  // nothing is held; the clock starts fresh
  int justPressed = strongestKey();   // the clearest touch this scan, or -1
  if (justPressed >= 0) {
    pressedNote = keys[justPressed + keyboardOffset];
    activeKey = justPressed;

    // ── FREE PLAY: no lock, no guess, no penalty ───────────────────────────
    // Everything below this block exists to stop flaky fruit contact from
    // machine-gunning the GAME. An instrument wants the opposite: the same
    // lemon, again and again, as fast as you like. So free play takes the
    // short road — sound the note, show the pitch, and stop.
    if (freePlay()) {
      startKeyTone(pressedNote);
      showPitchBar(justPressed);
      ledMeterUntil = 0;             // the note owns the bar now, not the meter
      if (serialEnabled) {
        Serial.print(F("~ key ")); Serial.print(justPressed + 1);
        Serial.print(F("  ")); Serial.print(pressedNote); Serial.println(F(" Hz"));
      }
      return;
    }

    // A KEY ALWAYS SOUNDS (2026-09-13). This is a piano before it is a game, so
    // the note under the finger is never withheld — not even when the game has
    // nothing to do with it. A repeat used to stay silent and then scold, with
    // the sfxKeyStuck rattle; Sergio asked for both of those to go, because
    // pressing the same lemon twice is not a mistake, it is just not a move.
    startKeyTone(pressedNote);

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

    // ...but the GAME only sees the FIRST press of a key. A repeat still sounds;
    // it simply does not score, does not advance and — this is the part that
    // changed — does not COST anything either. It says so in light, not in noise.
    if (justPressed != lastCountedKey) {
      lastCountedKey = justPressed;
      handleGuess();
    } else {
      showRepeatSweep();
      if (serialEnabled) {
        Serial.print(F("    key ")); Serial.print(justPressed + 1);
        Serial.println(F(" again - sounds, scores nothing (play a different lemon)"));
      }
    }
  }
}


//################################
//#########  GAME LOGIC ##########
//################################

// Evaluate the note the player just pressed against the secret sequence and
// drive the ten-LED progress bar.
void handleGuess() {
  if (freePlay()) return;             // an instrument has nothing to be right about
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
      // LEVEL_COUNT, not FREE_PLAY: winning cycles 1->2->3->4->1 and never
      // lands on the wheel's fifth item. Free play is chosen, never earned.
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
//
// THE CHANNEL IS CONVERTED UNTIL IT STOPS MOVING (2026-09-13). The ATmega328P's
// sample-and-hold cap charges through whatever drives the pin, and it starts
// every conversion still holding the PREVIOUS channel's level. The datasheet's
// recommended ceiling is a 10 kOhm source; how much of the gap one conversion
// closes falls off fast above it:
//
//     220 Ohm   tau = 3 ns  against a 12 us sampling window -> all of it
//     1 MOhm    tau = 14 us against the same window         -> about 57 %
//
// So on the 1 MOhm keyboard the v0.7.1 board now carries, ONE conversion is not
// a reading, it is a step towards one -- and a single discarded conversion, this
// function's first fix, is only the first step. Worst case is the SENS - button:
// it is A7, an ADC channel on the SAME multiplexer, read at the top of every
// loop() and sitting at 0 V while it is held. That handed key 1 a cap holding
// zero and made the button play a note (Sergio, 2026-09-13; test 14).
//
// Converting until two readings agree is the only form of this that survives
// the resistors changing again: a settled 220 Ohm channel breaks out on the
// second conversion and costs 112 us, a 0 V -> 5 V worst case on 1 MOhm takes
// eight and costs 0.9 ms. Nothing here is tuned to a particular pull-up.
const uint8_t SETTLE_MAX = 12;   // hard stop, so a noisy channel cannot stall the scan
const int     SETTLE_EPS = 1;    // counts: settled once two in a row are this close

int readKey(uint8_t i) {
#ifdef VELXIO_EMULATION
  if (i == 6) {
    return digitalRead(KEY_PINS[6]) == LOW ? 0 : 1023;
  }
#endif
  // Select the channel and keep converting until it holds still. Every one of
  // these is thrown away: they are the S/H cap filling up, not a measurement.
  int prev = analogRead(i);
  for (uint8_t n = 0; n < SETTLE_MAX; n++) {
    int v = analogRead(i);
    int d = v - prev;
    if (d < 0) d = -d;
    prev = v;
    if (d <= SETTLE_EPS) break;
  }
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
    // Progressive fill across all TEN LEDs, not just the seven keys: LED count
    // scales with measurement progress ((i+1)*LED_COUNT/KEY_COUNT), so the bar
    // reaches all ten by the last key regardless of there being only seven.
    uint8_t lit = (uint8_t) (((uint16_t) (i + 1) * LED_COUNT) / KEY_COUNT);
    for (uint8_t l = 0; l < LED_COUNT; l++) {
      digitalWrite(LED_PINS[l], l < lit ? HIGH : LOW);
    }
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
    delay(CAL_STEP_PAUSE_MS);  // let the LED count-up read as deliberate
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
    if ((int) i == chordKey) continue;               // ...and so is this one
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

// Poll the two buttons once per loop() and act on whatever gesture the state
// machine says that was. Every timing decision — debounce, the ramp, the 3 s
// charge, the both-press — lives in ui_gestures.h and is asserted by the host
// test; this function is deliberately nothing but a switch.
void serviceButtons() {
  const unsigned long now = millis();
  const bool plusDown = sensUpDown();
  const bool minusDown = sensDownDown();

  // Snapshot the knob as each button goes down. The nudge fires on PRESS, so
  // every hold gesture begins by moving the sensitivity one step; if the press
  // turns out to be a hold, the gesture puts ITS OWN snapshot back. Someone
  // reaching for a mode change must not find the keyboard re-tuned afterwards.
  // (Releasing early KEEPS the step — that press really was the knob.)
  if (plusDown  && !plusWasDown)  marginBeforePlus  = touchMargin;
  if (minusDown && !minusWasDown) marginBeforeMinus = touchMargin;
  plusWasDown  = plusDown;
  minusWasDown = minusDown;

  switch (gestures.update(plusDown, minusDown, now)) {
    case UiGestures::EV_NUDGE_MORE:
      nudgeMargin(-1);
      break;
    case UiGestures::EV_NUDGE_LESS:
      nudgeMargin(+1);
      break;
    case UiGestures::EV_SMART_ADJUST:
      learnFromTouch();
      break;
    case UiGestures::EV_ARM_START:
      log(gestures.armingPlus() ? F("hold + ... keep holding to swap instrument/game")
                                : F("hold − ... keep holding for the level wheel"));
      ledMeterUntil = 0;              // the bar is the charge meter from here
      showArmBar(0);
      soundArmTick(0);
      break;
    case UiGestures::EV_ARM_TICK:
      soundArmTick(gestures.armPercent(now));
      break;
    case UiGestures::EV_ARM_CANCEL:
      // Let go (or reached for the other button) before the three seconds were
      // up. Nothing happened — say so with the same "that did not work" bump
      // the end stops use, and hand the bar back to the sensitivity meter.
      log(F("menu cancelled - nothing changed"));
      soundLimit();
      showMarginOnBar();
      break;
    case UiGestures::EV_TOGGLE_FREEPLAY:
      touchMargin = marginBeforePlus;  // see the snapshot above
      toggleFreePlay();
      break;
    case UiGestures::EV_MENU_OPEN:
      touchMargin = marginBeforeMinus; // see the snapshot above
      enterMenu();
      break;
    case UiGestures::EV_MENU_NEXT:
    case UiGestures::EV_MENU_PREV:
      announceMenuItem();
      break;
    case UiGestures::EV_MENU_ACCEPT:
      acceptMenuItem();
      break;
    case UiGestures::EV_MENU_CANCEL:
      cancelMenu();
      break;
    case UiGestures::EV_NONE:
    default:
      break;
  }

  // The charge meter is redrawn every loop while − is held past one second, so
  // it climbs smoothly rather than in 500 ms jumps with the ticks.
  if (gestures.arming()) showArmBar(gestures.armPercent(millis()));
}


//################################
//########  MODE WHEEL ###########
//################################
// Four items: level 1, 2, 3, 4. It wraps. It previews. It is opened by holding
// −, turned with + and −, accepted with both buttons, and left by a long press
// or by walking away. See the gesture map in include/ui_gestures.h.
//
// FREE PLAY IS NOT ON IT (2026-09-13) — it is a hold on +, see toggleFreePlay().

// True while ANY button is down — the abort hook that lets a preview melody be
// cut off the moment the player turns the wheel again. Without it the wheel
// runs at the speed of the tunes instead of the speed of the hand.
static bool menuInterrupt() { return sensUpDown() || sensDownDown(); }

// Entry has to be unmistakable: the piano was one thing a second ago and is now
// another. A fast full sweep of the bar (something no game state does), then a
// rising three-note cue, then the first item announces itself.
void enterMenu() {
  hushBuzzer();                       // a held lemon stops sounding, cleanly
  ledMeterUntil = 0;
  // Open ON the mode being played, so "open it, accept it" is a no-op. From
  // free play there is no such item any more, so it opens on the level free
  // play interrupted — accept that and you are back where the + hold found you.
  gestures.setItem((uint8_t) ((freePlay() ? levelBeforeFreePlay : level) - 1));
  log(F("== MODE MENU: + / - to choose, BOTH to accept, long press to leave"));
  for (uint8_t i = 0; i < LED_COUNT; i++) {  // sweep out...
    allLedsOff(); digitalWrite(LED_PINS[i], HIGH); delay(MENU_ENTER_SWEEP_MS);
  }
  for (int8_t i = LED_COUNT - 1; i >= 0; i--) {  // ...and back
    allLedsOff(); digitalWrite(LED_PINS[i], HIGH); delay(MENU_ENTER_SWEEP_MS);
  }
  allLedsOff();
  playSfx(sfxMenuOpen);
  announceMenuItem();
}

// Say where the wheel is now, in all three languages the piano speaks: serial,
// light, and — the one that matters with the lid closed — the mode's own tune.
void announceMenuItem() {
  const uint8_t it = gestures.item();
  if (serialEnabled) {
    Serial.print(F("  > Level ")); Serial.println(it + 1);
  }
  menuAnimAt = millis();
  menuBlinkOn = true;
  showMenuItem(it, true);
  playMenuPreview(it);
  gestures.noteActivity(millis());  // the music was not idleness
}

// The preview: the opening of that level's own theme, capped so the wheel never
// feels slow, and abortable so the next press cuts it off mid-phrase.
void playMenuPreview(uint8_t item) {
  const uint8_t cap = MENU_PREVIEW_NOTES;
  switch (item) {
    case 0: playSong(marioNotes, marioTempo, 0,
                     MARIO_INTRO_LEN < cap ? MARIO_INTRO_LEN : cap,
                     0, 0, false, menuInterrupt); break;
    case 1: playSong(underworldNotes, underworldTempo, 0,
                     UNDER_INTRO_LEN < cap ? UNDER_INTRO_LEN : cap,
                     0, 0, false, menuInterrupt); break;
    case 2: playSong(starmanNotes, starmanTempo, 0,
                     STARMAN_INTRO_LEN < cap ? STARMAN_INTRO_LEN : cap,
                     0, 0, false, menuInterrupt); break;
    default: playSong(castleNotes, castleTempo, 0,
                     CASTLE_INTRO_LEN < cap ? CASTLE_INTRO_LEN : cap,
                     0, 0, false, menuInterrupt); break;
  }
}

// ── FREE PLAY, on its own button (2026-09-13) ───────────────────────────────
// Sergio's ask: "mantener el botón más es un switch entre el modo libre y el
// nivel". Not a menu entry — a switch, the way an instrument has one. So the
// wheel lost its fifth item and + grew a three-second hold.
//
// Going IN remembers the level it interrupted; coming OUT goes back to it,
// restarted from zero, for the same reason accepting a level does: a mode you
// asked for should not hand you someone else's half-finished progress bar. A
// player who never touched the wheel is on level 1, so for them this is exactly
// the "free play <-> level 1" switch they asked for, with no special case.
//
// playLevelIntro() already says which of the two you landed in — the level's
// own theme, or free play's scale — so the switch announces itself in the one
// language that works with the lid closed.
void toggleFreePlay() {
  hushBuzzer();
  if (freePlay()) {
    level = levelBeforeFreePlay;
    if (level < 1 || level > LEVEL_COUNT) level = 1;   // nothing else is a level
  } else {
    levelBeforeFreePlay = level;
    level = FREE_PLAY;
  }
  playSfx(sfxMenuAccept);
  resetBoard();
  logGame();
  delay(PHRASE_GAP_MS);
  playLevelIntro();
  restoreIdleDisplay();
}

// The wheel stops here. Accepting a level RESTARTS it from zero — a mode you
// chose deliberately should not drop you into someone else's half-finished
// progress bar. Accepting from inside free play also LEAVES free play, which is
// the only other way out of it besides the + hold.
void acceptMenuItem() {
  const uint8_t it = gestures.item();
  hushBuzzer();
  playSfx(sfxMenuAccept);
  level = (int) it + 1;               // item 0..3 -> level 1..4, never FREE_PLAY
  resetBoard();
  logGame();
  delay(PHRASE_GAP_MS);
  playLevelIntro();
  restoreIdleDisplay();
}

// Left without choosing: the game is exactly where it was, progress bar and
// all. The closing cue is the opening one played backwards, so "in" and "out"
// are one thing to learn instead of two.
void cancelMenu() {
  log(F("== menu closed - nothing changed"));
  playSfx(sfxMenuClose);
  restoreIdleDisplay();
}

// Draw one wheel item. Levels are a COUNT: level n = n LEDs from the left, so
// the number you read is the level you get.
void showMenuItem(uint8_t item, bool on) {
  allLedsOff();
  if (!on) return;
  for (uint8_t i = 0; i <= item && i < LED_COUNT; i++) {
    digitalWrite(LED_PINS[i], HIGH);
  }
}

// The menu's heartbeat, called every loop while it is open. Levels blink: a
// blinking bar is a question, a steady one is a score. (The bouncing-LED
// animation that marked free play on the wheel went with free play, 2026-09-13.)
void serviceMenu() {
  const unsigned long now = millis();
  const uint8_t it = gestures.item();

  const unsigned long period = menuBlinkOn ? (unsigned long) MENU_BLINK_ON_MS
                                           : (unsigned long) MENU_BLINK_OFF_MS;
  if (now - menuAnimAt >= period) {
    menuAnimAt = now;
    menuBlinkOn = !menuBlinkOn;
    showMenuItem(it, menuBlinkOn);
  }
}

// The charge meter: how much of the three-second hold is done.
void showArmBar(uint8_t pct) {
  uint8_t lit = (uint8_t) (((uint16_t) pct * LED_COUNT) / 100);
  if (lit > LED_COUNT) lit = LED_COUNT;
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    digitalWrite(LED_PINS[i], i < lit ? HIGH : LOW);
  }
}

// ...and its sound: a chirp per half second, rising with the meter, so the
// gesture can be completed without looking at the box. Silent while a lemon is
// sounding, for the same reason the sensitivity tick is (chopping a held note
// in half to acknowledge a button is worse than saying nothing) — the meter
// still climbs, so the gesture is never invisible.
void soundArmTick(uint8_t pct) {
  if (!UI_SOUNDS || activeKey >= 0) return;
  const int freq = ARM_TICK_LOW +
                   (int) (((long) (ARM_TICK_HIGH - ARM_TICK_LOW) * pct) / 100);
  playTone(freq, ARM_TICK_MS);
}
#endif  // !VELXIO_EMULATION

// The ten-LED bar doubles as a SENSITIVITY METER: how many LEDs are lit shows
// where the knob sits across the useful range on fruit (a margin of MARGIN_MIN
// to 20), so it can be read across the room without a serial monitor.
//
// IT COUNTS SENSITIVITY, NOT MARGIN (2026-09-13). More LEDs = more sensitive =
// a SMALLER margin, so the + button adds light and − takes it away. It used to
// be the other way round and Sergio called it straight away: a button labelled
// "more" that puts out lights is a button that reads as "less". The number on
// the bar and the label on the button now move together, which is the only
// version of this that can be read with the lid closed.
void showMarginOnBar() {
  int lit = LED_COUNT - (int) (((long) touchMargin * LED_COUNT) / 20);
  if (lit < 1) lit = 1;
  if (lit > LED_COUNT) lit = LED_COUNT;
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    digitalWrite(LED_PINS[i], i < lit ? HIGH : LOW);
  }
  ledMeterUntil = millis() + LED_METER_MS;
}

// ── "that lemon does not count" ─────────────────────────────────────────────
// A repeated lemon still SOUNDS — this is a piano before it is a game — but the
// game ignores it, and the player has to be able to SEE that. It used to be
// told with the sfxKeyStuck rattle, and Sergio had it right on 2026-09-13: that
// noise reads as a mistake, and pressing the same lemon twice is not a mistake.
// It is simply not a move.
//
// So the bar runs BACKWARDS: one LED from the right end to the left, then the
// progress bar snaps back exactly as it was. Three things make that readable
// across the room, and each is deliberate:
//
//   - it MOVES, and this bar is otherwise perfectly steady while playing.
//     Motion is already this piano's word for "neither a score nor a question"
//     (it is what the wheel used to mark free play), so it cannot be read as
//     either of them.
//   - it runs RIGHT TO LEFT, against the direction progress fills, so it reads
//     as "that took you nowhere" and never as a step forward.
//   - it ENDS on the identical bar it started from. The score is visibly
//     untouched, which is the whole message. A wrong note, by contrast, blanks
//     the bar and LEAVES it blank — the two can never be confused.
//
// It is not the menu's entry sweep either: that one goes out AND back, is
// slower, and arrives with a three-note cue. This is one flick, under a note
// that is already sounding.
void showRepeatSweep() {
  for (int8_t i = LED_COUNT - 1; i >= 0; i--) {
    allLedsOff();
    digitalWrite(LED_PINS[i], HIGH);
    delay(REPEAT_SWEEP_MS);
  }
  restoreIdleDisplay();      // …and the score is exactly where it was
}

// ── What the bar means in FREE PLAY ─────────────────────────────────────────
// Idle: DARK — and that is the change of 2026-09-15. It used to be the two ENDS
// lit, a shape the game's left-filling bar can never make, so one glance said
// "this is the instrument". The trouble is that it never went away: press a
// lemon, the pitch bar climbs, and those two end LEDs are still sitting there
// inside the reading, pretending to be part of it. Sergio: "se queda un poco
// raro... que no se queden esos dos LEDs encendidos de los extremos".
//
// He is right, and the dark bar is the better instrument anyway: every LED that
// is lit was lit by a finger, and nothing else. What announces the mode instead
// is playFreePlayEntrySweep() — once, loudly, and then out of the way.
void showFreePlayIdle() {
  allLedsOff();
}

// Sounding: the bar is a PITCH meter. Key 1 lights one LED and key 7 lights all
// ten, spread proportionally ((key+1)*10/7 = 1,2,3,5,7,8,10) so seven keys use
// the whole bar and the light climbs with the note — the same trick calibration
// uses to fill ten LEDs with seven measurements.
void showPitchBar(int key) {
  uint8_t lit = (uint8_t) ((((uint16_t) key + 1) * LED_COUNT) / KEY_COUNT);
  if (lit < 1) lit = 1;
  if (lit > LED_COUNT) lit = LED_COUNT;
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    digitalWrite(LED_PINS[i], i < lit ? HIGH : LOW);
  }
}

// Where one key's light sits at the TOP of its pitch bar: key 1 -> LED 1, key 7
// -> LED 10, and the five in between spread over 0,1,3,4,6,7,9.
static uint8_t ledForKey(int key) {
  uint8_t lit = (uint8_t) ((((uint16_t) key + 1) * LED_COUNT) / KEY_COUNT);
  if (lit < 1) lit = 1;
  if (lit > LED_COUNT) lit = LED_COUNT;
  return (uint8_t) (lit - 1);
}

// TWO lemons sounding: two LONE LEDs, one at each note's place on the bar, and
// nothing else. A single note fills the bar from the left, so "two separated
// points" is a shape one finger can never draw — the light says "two" before
// the ear has finished deciding what it is hearing. It is also, on purpose,
// the shape the mode's idle bar used to wear: the two ends, keys 1 and 7, are
// simply the widest chord this keyboard has.
void showChordBar(int a, int b) {
  allLedsOff();
  digitalWrite(LED_PINS[ledForKey(a)], HIGH);
  digitalWrite(LED_PINS[ledForKey(b)], HIGH);
}

// ── free play, announcing itself in light (2026-09-15) ──────────────────────
// The mode used to be marked by a STANDING shape (both ends lit), which meant
// the announcement was still on the bar an hour later, underneath every note.
// This says the same thing as an EVENT instead, and an event can end:
//
//   1. two lights walk in from the ends and meet in the middle — the old idle
//      shape, collected up and carried away,
//   2. they open back out, filling the bar to all ten: "the whole keyboard is
//      yours now",
//   3. and the bar empties from the outside in, to nothing.
//
// It ends DARK, which is exactly where free play then lives, so the animation
// hands over to the mode instead of being switched off by it. Nothing sounds
// under it: the scale flourish plays straight afterwards and that is the mode's
// voice — two announcements on top of each other would be neither.
void playFreePlayEntrySweep() {
  const uint8_t half = (LED_COUNT + 1) / 2;
  allLedsOff();
  for (uint8_t i = 0; i < half; i++) {            // 1. in from both ends
    allLedsOff();
    digitalWrite(LED_PINS[i], HIGH);
    digitalWrite(LED_PINS[LED_COUNT - 1 - i], HIGH);
    delay(FREE_ENTRY_STEP_MS);
  }
  for (int8_t i = (int8_t) half - 1; i >= 0; i--) {  // 2. back out, filling
    digitalWrite(LED_PINS[i], HIGH);
    digitalWrite(LED_PINS[LED_COUNT - 1 - i], HIGH);
    delay(FREE_ENTRY_STEP_MS);
  }
  delay(FREE_ENTRY_HOLD_MS);
  for (uint8_t i = 0; i < half; i++) {            // 3. ...and away to nothing
    digitalWrite(LED_PINS[i], LOW);
    digitalWrite(LED_PINS[LED_COUNT - 1 - i], LOW);
    delay(FREE_ENTRY_STEP_MS);
  }
  allLedsOff();
}


//################################
//#####  TWO LEMONS AT ONCE ######
//################################
// The rules and the reasoning are at CHORD_EXTRA_COUNTS, up in the constants.
// This is only the machinery.

// How far below its own resting level this channel is sitting, in ADC counts.
// The dip, not the reading, is the only number worth comparing between keys:
// the baselines differ per key and drift all day, the dips do not.
int keyDip(uint8_t i) {
  int d = baseline[i] - readKey(i);
  return d < 0 ? 0 : d;
}

// Is exactly one OTHER lemon being held as deliberately as the one already
// sounding? Returns it, or -1 for "no, that is one finger and its shadows".
int chordPartnerFor(int a) {
  const int dipA = keyDip((uint8_t) a);
  if (dipA <= 0) return -1;
  const int floorCounts = touchMargin + CHORD_EXTRA_COUNTS;   // gate 1
  int best = -1, bestDip = 0, passed = 0;
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    if ((int) i == a) continue;
    const int d = keyDip(i);
    if (d < floorCounts) continue;                            // gate 1
    if ((long) d * 100 < (long) dipA * CHORD_MIN_RATIO_PCT) continue;   // gate 2
    passed++;
    if (d > bestDip) { bestDip = d; best = (int) i; }
  }
  if (passed != 1) return -1;                                 // gate 3
  return best;
}

// Called every loop while a lemon is held. Outside free play it does nothing at
// all: the game recognises a guess by comparing ONE frequency against the
// secret sequence, so a chord there is not a richer guess, it is an unanswerable
// question. Two notes are an instrument's idea, and free play is the instrument.
void serviceChord() {
  if (!freePlay()) return;
  if (activeKey < 0) return;

  if (chordKey >= 0) {
    // Already a chord, and now the only question is whether it still is. Both
    // voices are judged AGAINST EACH OTHER rather than against a threshold: a
    // lifted lemon keeps reading low while the other finger shadows it, so the
    // only thing that reliably changes is that the two dips stop being
    // comparable. Whichever one falls away is the one that left — and it can be
    // either of them, so a chord is as happy to lose its lower voice as its
    // upper one.
    const int dipA = keyDip((uint8_t) activeKey);
    const int dipB = keyDip((uint8_t) chordKey);
    int leaving = -1;
    if (dipB < touchMargin) leaving = 1;                 // gone outright
    else if (dipA < touchMargin) leaving = 0;
    else if ((long) dipB * 100 < (long) dipA * CHORD_HOLD_RATIO_PCT) leaving = 1;
    else if ((long) dipA * 100 < (long) dipB * CHORD_HOLD_RATIO_PCT) leaving = 0;

    if (leaving < 0) {                                   // both still there
      chordReleaseSeenAt = 0;
      chordLeaving = -1;
      serviceChordTone();
      return;
    }
    // ...and a release still has to prove itself, exactly as a single note's
    // does: through 1 MOhm a resting finger reads clear for a scan or two
    // without going anywhere, and a chord that flickers on that is worse than
    // no chord at all.
    if (chordReleaseSeenAt == 0 || leaving != chordLeaving) {
      chordReleaseSeenAt = millis();
      chordLeaving = leaving;
    }
    if ((millis() - chordReleaseSeenAt) < RELEASE_CONFIRM_MS) { serviceChordTone(); return; }
    if (leaving == 1) endChord(); else promoteChordVoice();
    return;
  }

  const int cand = chordPartnerFor(activeKey);
  if (cand < 0) { chordCandidate = -1; chordCandidateSince = 0; return; }
  if (cand != chordCandidate) {              // a new claim: start its clock
    chordCandidate = cand;
    chordCandidateSince = millis();
    return;
  }
  if ((millis() - chordCandidateSince) < CHORD_CONFIRM_MS) return;
  startChord(cand);
}

// The handover. tone() reprograms the running timer rather than stopping it, so
// this is a frequency change on a note that never breaks — which is why the
// alternation reads as one shimmering interval instead of two notes taking
// turns. Nothing here waits: the swap happens on whichever loop first crosses
// the deadline, and the loop while a key is held is a few hundred microseconds.
void serviceChordTone() {
  if (chordKey < 0 || activeKey < 0) return;
  if ((millis() - chordSwapAt) < CHORD_SWAP_MS) return;
  chordSwapAt = millis();
  chordOnSecond = !chordOnSecond;
  tone(BUZZER, noteForKey(chordOnSecond ? chordKey : activeKey));
}

void startChord(int partner) {
  chordKey = partner;
  chordCandidate = -1;
  chordCandidateSince = 0;
  chordReleaseSeenAt = 0;
  chordSwapAt = 0;              // ...so the first handover happens immediately
  chordOnSecond = false;
  showChordBar(activeKey, chordKey);
  ledMeterUntil = 0;            // the chord owns the bar now, not the meter
  if (serialEnabled) {
    // The dips are printed because they are the tuning evidence: gate 2 is a
    // ratio between these two numbers, and this is the only place the real one
    // on real fruit can be read off.
    Serial.print(F("~~ chord ")); Serial.print(activeKey + 1);
    Serial.print(F("+")); Serial.print(chordKey + 1);
    Serial.print(F("  ")); Serial.print(noteForKey(activeKey));
    Serial.print(F("+")); Serial.print(noteForKey(chordKey));
    Serial.print(F(" Hz  dips ")); Serial.print(keyDip((uint8_t) activeKey));
    Serial.print(F("/")); Serial.println(keyDip((uint8_t) chordKey));
  }
  serviceChordTone();
}

// The second finger left; the first is still down. Back to one voice, with the
// buzzer handed straight back rather than stopped — the remaining note was
// never released, so it must not be re-articulated either.
void endChord() {
  chordKey = -1;
  chordReleaseSeenAt = 0;
  chordLeaving = -1;
  chordOnSecond = false;
  releaseSeenAt = 0;            // the surviving note was never let go
  if (activeKey < 0) return;
  tone(BUZZER, noteForKey(activeKey));
  showPitchBar(activeKey);
  if (serialEnabled) {
    Serial.print(F("~~ chord ends - key ")); Serial.print(activeKey + 1);
    Serial.println(F(" alone"));
  }
}

// The FIRST finger left and the second is still down: the chord does not end,
// it narrows. The survivor becomes the note under the finger, keeping its own
// minimum length so a chord broken a millisecond after it opened still leaves a
// note you can hear.
void promoteChordVoice() {
  const int survivor = chordKey;
  clearChord();
  releaseSeenAt = 0;            // the surviving note was never let go
  activeKey = survivor;
  pressedNote = noteForKey(survivor);
  startKeyTone(pressedNote);
  showPitchBar(survivor);
  if (serialEnabled) {
    Serial.print(F("~~ chord ends - key ")); Serial.print(survivor + 1);
    Serial.println(F(" holds on"));
  }
}

void clearChord() {
  chordKey = -1;
  chordLeaving = -1;
  chordCandidate = -1;
  chordCandidateSince = 0;
  chordReleaseSeenAt = 0;
  chordSwapAt = 0;
  chordOnSecond = false;
}

// Whatever the bar should be showing when nothing is sounding and no meter is
// up. One place, because there are now two answers and three callers.
void restoreIdleDisplay() {
  if (freePlay()) { showFreePlayIdle(); return; }
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    digitalWrite(LED_PINS[i], i < currentStep ? HIGH : LOW);
  }
}

// FREE PLAY announcing itself: the seven lemons, left to right, as a rising
// scale with the pitch bar climbing under it, because the most honest
// description of "the keys are do re mi fa sol la si" is to play do re mi fa
// sol la si.
//
// IT PLAYS TO THE END NOW (2026-09-15). It used to give up the moment either
// button read down, from the days when it was also the wheel's preview for free
// play and a turn of the wheel had to cut it off. Free play left the wheel on
// 2026-09-13, so the only button that can still be down while this plays is the
// + that just asked for the mode — and cutting the announcement short because
// the player has not let go of the button yet is the opposite of announcing it.
// Sergio, 2026-09-15: "y como siempre toques la melodía".
void playFreePlayFlourish(bool lights) {
  const int base = (FREE_PLAY - 1) * KEY_COUNT;
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    if (lights) showPitchBar((int) i);
    playTone(keys[base + i], 85);
    delay(15);
  }
  delay(SFX_TAIL_MS);
}


//################################
// Clear the per-round state: progress, last note, edge latches, and the bar.
void resetBoard() {
  currentStep = 0;
  pressedNote = 0;
  clearChord();
  lastCountedKey = -1;   // a new round may legitimately open with the key that
                         // ended the last one
  stopKeyTone();
  activeKey = -1;
  allLedsOff();
}

void logGame() {
  if (serialEnabled) {
    if (freePlay()) {
      Serial.println(F("FREE PLAY - do re mi fa sol la si, no code, no penalty"));
      return;
    }
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
  clearChord();
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
  const int *notes; const int *tempo; uint8_t from, length;
  switch (level) {
    case 1: notes = marioNotes; tempo = marioTempo; from = MARIO_VICTORY_FROM; length = MARIO_LEN; break;
    case 2: notes = underworldNotes; tempo = underworldTempo; from = UNDER_VICTORY_FROM; length = UNDER_LEN; break;
    case 3: notes = starmanNotes; tempo = starmanTempo; from = STARMAN_VICTORY_FROM; length = STARMAN_LEN; break;
    default: notes = castleNotes; tempo = castleTempo; from = CASTLE_VICTORY_FROM; length = CASTLE_LEN; break;
  }
  // Progressive fill (2026-07-29): the bar counts up from empty to all ten
  // LEDs across the whole victory tail, instead of flashing the whole bar per
  // note — ledTotal = the tail's own note count, so the pace scales with
  // whichever level's theme is playing (26-50 notes -> ~230-580 ms per LED).
  playSong(notes, tempo, from, length, (uint16_t) (length - from), 0);
}

// Level-start announce: the first few notes of the CURRENT level's own theme,
// so the player hears which level they are on before touching a lemon. Plays
// once at boot and once every time a level begins (auto-advance or wrap).
void playLevelIntro() {
  hushBuzzer();               // silence + a beat, safe even if nothing was sounding
  if (freePlay()) {           // not a level: the scale IS the announcement
    playFreePlayEntrySweep();   // ...opened by a sweep that says the mode changed
    playFreePlayFlourish(true);
    delay(SFX_TAIL_MS);
    showFreePlayIdle();         // ...and then the bar is dark until a finger
    return;
  }
  switch (level) {
    case 1: playSong(marioNotes, marioTempo, 0, MARIO_INTRO_LEN); break;
    case 2: playSong(underworldNotes, underworldTempo, 0, UNDER_INTRO_LEN); break;
    case 3: playSong(starmanNotes, starmanTempo, 0, STARMAN_INTRO_LEN); break;
    default: playSong(castleNotes, castleTempo, 0, CASTLE_INTRO_LEN); break;
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
// game pauses while the theme plays. Two light-show modes:
//   - ledTotal == 0 (default): the WHOLE bar flashes to the beat — lit while
//     the note sounds, dark in the inter-note gap and during rests. Used by
//     the level-start intro.
//   - ledTotal > 0 (2026-07-29): PROGRESSIVE fill instead — the bar
//     accumulates from empty to all LED_COUNT lit (and stays lit, note to
//     note) across ledTotal notes total, so it reads as a count-up rather
//     than a flash. ledOffset lets a caller span the fill across more than
//     one playSong() call by telling this call how many of ledTotal's steps
//     already happened elsewhere. Used by the win theme (playVictory()).
//   - lights == false (2026-09-09): the bar is left ALONE. The mode wheel is
//     already using it to show which item you are on, and a preview melody that
//     flashed over that would hide the very thing it is previewing.
// checkAbort, if given, is polled after every note and stops the melody early —
// that is what lets a preview be cut off the instant the wheel turns again,
// so the menu runs at the speed of the hand rather than the speed of the tunes.
void playSong(const int *notes, const int *tempos, uint8_t from, uint8_t length,
              uint16_t ledTotal, uint16_t ledOffset, bool lights,
              bool (*checkAbort)()) {
  noTone(BUZZER);  // silence any lingering key tone before bit-banging the pin
  for (uint8_t i = from; i < length; i++) {
    int frequency = (int) pgm_read_word(&notes[i]);
    int tempo = (int) pgm_read_word(&tempos[i]);

    // note duration: one second / note type (quarter = 1000/4, eighth = 1000/8...)
    int noteDuration = 1000 / tempo;

    if (lights) {
      if (ledTotal > 0) {
        uint16_t step = ledOffset + (uint16_t) (i - from) + 1;
        uint8_t lit = (uint8_t) (((uint32_t) step * LED_COUNT) / ledTotal);
        if (lit > LED_COUNT) lit = LED_COUNT;
        for (uint8_t l = 0; l < LED_COUNT; l++) {
          digitalWrite(LED_PINS[l], l < lit ? HIGH : LOW);
        }
      } else if (frequency > 0) {
        allLedsOn();
      }
    }
    buzz(BUZZER, frequency, noteDuration);
    if (lights && ledTotal == 0) allLedsOff();

    // a gap of duration + 30% keeps consecutive notes distinct
    delay((unsigned long)(noteDuration * 1.30));
    buzz(BUZZER, 0, noteDuration);  // stop

    if (checkAbort && checkAbort()) {
      noTone(BUZZER);
      return;
    }
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
