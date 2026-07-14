/* LEMON PIANO V5 — 10-LED progress bar
   Author : Yupipi93 (Sergio Conejero), 2019 · V5 rework 2026-07-14

   CODE 1 (Mario Main Theme):  6,5,6,7,2,5,2,1,3,4
   CODE 2 (Mario Underworld) :  3,6,1,4,2,5,3,6,1,4

   Seven lemons are touch keys on A0..A6 (the player's body closes each key to
   5 V). Reproduce the secret 10-note melody. A row of TEN green LEDs is the
   progress bar: each correct note lights the next LED; a wrong note blanks the
   whole row and the sequence restarts from the first note. Light all ten and
   the theme plays — then the game AUTO-ADVANCES to the other theme (win the
   Mario Main Theme and it flips to the Underworld, and back), so both games
   cycle from a single starting point.

   What changed from V4 (see archive/lemon-piano-v4/ for the frozen original):
     - No relays / no water pump, and no red LED — the ten-LED bar is the whole
       feedback surface (all-on = win in progress, all-off = you missed).
     - The fail-counter / death-tune game-over is gone with the pump it was
       tied to; a wrong note just resets the bar. A short low "wrong" tone
       still plays for feedback.
     - GAME SELECT moved from D4 to A7 so all 12 digital pins are free to drive
       the ten LEDs. A7 is analog-in only on the Nano: tie it HIGH (5 V, game 1)
       or LOW (GND, game 2) with an SPDT switch (or a switch to 5 V + 10k pull-
       down). Read at boot and on RESTART.

   Kept from V4: edge-triggered input (one step per fresh touch), auto-
   calibrated analog touch, PROGMEM melodies, both games, and the compile-time
   Velxio emulation shim (see emulation/README.md).

   Requires an Arduino Nano (or Mini) — V5 uses A6 (key 7) and A7 (game select),
   which the classic Uno does not expose.
*/

#include <Arduino.h>
#include "notes.h"

//################################
//###########  PINS ##############
//################################
#ifdef VELXIO_EMULATION
#define BUZZER 11  // Velxio only polls PWM duty on D3/5/6/9/10/11; the buzzer
                   // part's note-off fires ONLY on duty->0, so on D8 the first
                   // tone would play forever (see emuTone below)
#else
#define BUZZER 8
#define GAME_SELECT A7  // analog-in only: HIGH(5V)=game 1, LOW(GND)=game 2
#define RESTART 7       // press (active-HIGH) to restart / re-read game select
#endif

const uint8_t KEY_COUNT = 7;    // A0..A6
const uint8_t LED_COUNT = 10;   // one green LED per note of the sequence

// The ten green progress LEDs, LED[0] = first note ... LED[9] = tenth note.
#ifdef VELXIO_EMULATION
// Emulation reuses D7/D8 (no restart/HW-buzzer pins here) and keeps key 7 on
// D12 + buzzer on D11, leaving exactly these ten pins for the LEDs.
const uint8_t LED_PINS[LED_COUNT] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 13};
#else
// Hardware: D7 = RESTART, D8 = BUZZER, so the LEDs take every other free pin.
const uint8_t LED_PINS[LED_COUNT] = {2, 3, 4, 5, 6, 9, 10, 11, 12, 13};
#endif

// ── Velxio emulation input shim (define VELXIO_EMULATION to enable) ─────────
// The browser emulator can't reproduce the analog lemon divider or A6/A7, and
// with ten LEDs every I/O line is spoken for. So the emulation build:
//   - keys 1-6: pushbuttons + 10k pull-ups on A0..A5, analogRead, active-low
//     (idle ~1023, pressed ~0; threshold 512),
//   - key 7:    pushbutton + pull-up on D12, digitalRead (A6 has no digital pin),
//   - drops GAME SELECT and RESTART (no pins left): the browser starts at game 1
//     and each win auto-advances to the other theme, so both games are reachable
//     with no select switch and no restart button,
//   - buzzer on D11 via emuTone (Timer2-duty audio with an explicit note-off).
// Hardware builds (macro undefined) are unaffected.
#ifdef VELXIO_EMULATION
const uint8_t KEY_PINS[KEY_COUNT] = {A0, A1, A2, A3, A4, A5, 12};
#endif

//################################
//#########  CONSTANTS ###########
//################################
const int SEQUENCE_LENGTH = 10;   // notes to guess correctly to win (== LED_COUNT)
const int TOUCH_MARGIN = 100;     // touch threshold = per-key idle baseline + this
                                  // (auto-calibrated at boot; replaces the old fixed
                                  //  SENSITIVITY 100/170-per-power-supply constant)
const int NOTE_DURATION = 70;     // key tone length in ms; the lower, the snappier the feel
const int WRONG_TONE_MS = 200;    // low "you missed" tone

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

#define MARIO_LEN          (sizeof(marioNotes) / sizeof(marioNotes[0]))
#define UNDER_LEN          (sizeof(underworldNotes) / sizeof(underworldNotes[0]))
const uint8_t MARIO_VICTORY_FROM = 28;   // full theme is 78 notes; cut = tail 50
const uint8_t UNDER_VICTORY_FROM = 12;   // full theme is 56 notes; cut = tail 44

//################################
//#####  SECRET SEQUENCES ########
//################################
// Keys 0..6 for game 1, 7..13 for game 2 (keyboardOffset selects the half).
const int keys[14] = {
  NOTE_E6, NOTE_G6, NOTE_A6, NOTE_B6, NOTE_C7, NOTE_E7, NOTE_G7,   // game 1
  NOTE_A3, NOTE_AS3, NOTE_C4, NOTE_A4, NOTE_AS4, NOTE_C5, NOTE_D5  // game 2
};
const int sequence_1[SEQUENCE_LENGTH] = {NOTE_E7, NOTE_C7, NOTE_E7, NOTE_G7, NOTE_G6, NOTE_C7, NOTE_G6, NOTE_E6, NOTE_A6, NOTE_B6};
const int sequence_2[SEQUENCE_LENGTH] = {NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4, NOTE_AS3, NOTE_AS4, NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4};

//################################
//#########  STATE ###############
//################################
int  game = 1;                 // 1 = Main Theme, 2 = Underworld
bool started = false;          // false triggers (re)selection of the game
int  currentStep = 0;          // how many correct notes so far (index into the sequence)
int  pressedNote = 0;          // last note played

int  keyThreshold[KEY_COUNT];  // per-key touch threshold (auto-calibrated)
bool keyHeld[KEY_COUNT];       // previous touched-state, for rising-edge detection

//################################
//#######  PROTOTYPES ############
//################################
bool keyTouched(uint8_t i);
#ifdef VELXIO_EMULATION
void emuTone(long frequency, long durationMs);
#endif
void calibrate();
void selectGame();
void resetBoard();
void logGame();
void handleGuess();
void playVictory();
void playSong(const int *notes, const int *tempos, uint8_t from, uint8_t length);
void wrongTone();
void keyTone(int note);
void allLedsOff();
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
  pinMode(RESTART, INPUT);
  // GAME_SELECT is A7 (analog-in only) — no pinMode / no internal pull-up; the
  // external switch must drive it to 5 V or GND.
#endif
  // A0..A6 need no pinMode for analogRead.

  calibrate();
}


//################################
//###########  LOOP ##############
//################################
void loop() {
#ifndef VELXIO_EMULATION
  // Restart at any time (re-reads GAME SELECT, blanks the bar).
  if (digitalRead(RESTART) == HIGH) {
    started = false;
  }
#endif

  if (!started) {
    selectGame();
    started = true;
  }

  const int keyboardOffset = (game == 1) ? 0 : 7;

//################################
//########  READ INPUT ###########
//################################
  // Scan all keys, update held-state, and act on the FIRST fresh press this
  // loop (rising edge only — debounced by hysteresis on the threshold).
  int justPressed = -1;
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    bool touched = keyTouched(i);
    if (touched && !keyHeld[i] && justPressed < 0) {
      justPressed = i;
    }
    keyHeld[i] = touched;
  }

  if (justPressed >= 0) {
    pressedNote = keys[justPressed + keyboardOffset];
    keyTone(pressedNote);   // it's a piano — every press sounds its note
    handleGuess();
  }
}


//################################
//#########  GAME LOGIC ##########
//################################

// Evaluate the note the player just pressed against the secret sequence and
// drive the ten-LED progress bar.
void handleGuess() {
  const int *sequence = (game == 1) ? sequence_1 : sequence_2;

  if (pressedNote == sequence[currentStep]) {
    // CORRECT — light this step's LED and advance.
    digitalWrite(LED_PINS[currentStep], HIGH);
    currentStep++;
    if (serialEnabled) {
      Serial.print(F("OK ")); Serial.print(currentStep);
      Serial.print(F("/")); Serial.println(SEQUENCE_LENGTH);
    }

    if (currentStep >= SEQUENCE_LENGTH) {
      // VICTORY — all ten lit; hold them through the theme, then AUTO-ADVANCE to
      // the other game and blank the bar for the next round.
      log(F("WIN"));
      playVictory();
      game = (game == 1) ? 2 : 1;
      resetBoard();
      logGame();
    }
  } else {
    // WRONG — blank the whole bar, sound the low tone, back to the start.
    log(F("WRONG"));
    allLedsOff();
    wrongTone();
    currentStep = 0;
  }
}

void allLedsOff() {
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    digitalWrite(LED_PINS[i], LOW);
  }
}

// Is key i currently touched/pressed? The single point where the two sensing
// models meet: real hardware = analog rise above the calibrated baseline;
// Velxio emulation = active-low keys (analog-low via pull-ups on A0..A5,
// digital-low on D12 for key 7).
bool keyTouched(uint8_t i) {
#ifdef VELXIO_EMULATION
  if (i < 6) {
    return analogRead(i) < 512;               // pull-up idle ~1023, pressed ~0
  }
  return digitalRead(KEY_PINS[i]) == LOW;     // key 7 on D12
#else
  return analogRead(i) > keyThreshold[i];
#endif
}

// Sample each key's floating idle level and set its touch threshold above it.
// Auto-handles the "raise SENSITIVITY on a laptop charger" note from 2019 by
// measuring the actual baseline instead of hardcoding it. Hands OFF at boot.
void calibrate() {
#ifdef VELXIO_EMULATION
  // Active-low keys against external pull-ups — no analog baseline. Arm key 7's
  // digital input, then WAIT for the electrical solver: Velxio's first SPICE
  // solve takes a moment, and until it drives our nets every input reads
  // 0/LOW (= "everything pressed"), which would spam notes. Idle-high on keys
  // 1 and 7 means the solve landed.
  pinMode(KEY_PINS[6], INPUT_PULLUP);
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    keyHeld[i] = false;
  }
  log(F("Emulation build: waiting for circuit solve (inputs idle-high)..."));
  while (keyTouched(0) || keyTouched(6)) {
    delay(10);
  }
  log(F("Inputs idle. Ready to play."));
  return;
#endif
  log(F("Calibrating (hands off)..."));
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    long sum = 0;
    for (uint8_t s = 0; s < 32; s++) {
      sum += analogRead(i);
      delay(1);
    }
    int baseline = sum / 32;
    keyThreshold[i] = baseline + TOUCH_MARGIN;
    keyHeld[i] = false;
    if (serialEnabled) {
      Serial.print(F("  A")); Serial.print(i);
      Serial.print(F(" baseline=")); Serial.print(baseline);
      Serial.print(F(" threshold=")); Serial.println(keyThreshold[i]);
    }
  }
}

// Pick the STARTING game (boot / restart). Wins after this auto-advance between
// the two games, so this only sets where the cycle begins.
void selectGame() {
#ifdef VELXIO_EMULATION
  game = 1;   // no select pin in the browser build — start at the Main Theme
#else
  game = (analogRead(GAME_SELECT) > 512) ? 1 : 2;   // A7: HIGH=game 1, LOW=game 2
#endif
  resetBoard();
  logGame();
}

// Clear the per-round state: progress, last note, edge latches, and the bar.
void resetBoard() {
  currentStep = 0;
  pressedNote = 0;
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    keyHeld[i] = false;
  }
  allLedsOff();
}

void logGame() {
  if (serialEnabled) {
    Serial.print(F("Game "));
    Serial.println(game);
  }
}


//################################
//#########  AUDIO ###############
//################################

// The pressed key's note (non-blocking on hardware).
void keyTone(int note) {
#ifdef VELXIO_EMULATION
  emuTone(note, NOTE_DURATION);
#else
  tone(BUZZER, note, NOTE_DURATION);
#endif
}

// Short low "you missed" tone.
void wrongTone() {
#ifdef VELXIO_EMULATION
  emuTone(NOTE_C2, WRONG_TONE_MS);
#else
  tone(BUZZER, NOTE_C2, WRONG_TONE_MS);
  delay(WRONG_TONE_MS + 20);
  noTone(BUZZER);
#endif
}

void playVictory() {
  if (game == 1) {
    playSong(marioNotes, marioTempo, MARIO_VICTORY_FROM, MARIO_LEN);
  } else {
    playSong(underworldNotes, underworldTempo, UNDER_VICTORY_FROM, UNDER_LEN);
  }
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
    buzz(BUZZER, frequency, noteDuration);

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
  // Timer2 duty, not raw edges) — route through emuTone instead.
  (void) targetPin;
  emuTone(frequency, length);
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
