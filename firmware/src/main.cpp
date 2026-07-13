/* LEMON PIANO V4 — fixed & improved
   Author : Yupipi93 (Sergio Conejero), 2019
   Rescued, translated and refactored : 2026-07-12

   CODE 1 (Mario Main Theme):  6,5,6,7,2,5,2,1,3,4
   CODE 2 (Mario Underworld) :  3,6,1,4,2,5,3,6,1,4

   Seven lemons are touch keys on A0..A6 (the player's body closes each key to
   5 V). Reproduce the secret 10-note melody: green LED = right, red = wrong.
   Miss from note 7 onward and a relay fires the water pump. Ten penalties and
   the Mario death tune plays until you press RESTART.

   This file WAS a 1:1 translation of the 2019 sketch; TODO.md items 1–12 are
   now applied. See CHANGELOG.md for what changed and why. The two behavioural
   fixes worth flagging:
     - The sequence engine is now edge-triggered (one step per fresh touch), so
       melodies with repeated consecutive notes are playable and a held finger
       no longer machine-guns the sequence.
     - Victory/death/penalty playback is still intentionally blocking (TODO #9):
       the game is meant to pause while a song or the spray plays. Idle key
       input is fully responsive.
*/

#include <Arduino.h>
#include "notes.h"

//################################
//###########  PINS ##############
//################################
#define BUZZER 8
#define RED_LED 2
#define GREEN_LED 3
#define GAME_SELECT 4   // held HIGH at boot/restart -> game 1, else game 2
#define RELAY_1 5       // water-pump relay pair
#define RELAY_2 6
#define RESTART 7       // press to restart (re-runs game selection)
#define BUZZ_LED 13     // onboard LED, lit while a melody note is bit-banged

const uint8_t KEY_COUNT = 7;   // A0..A6

//################################
//#########  CONSTANTS ###########
//################################
const int SEQUENCE_LENGTH = 10;   // notes to guess correctly to win
const int PUMP_FROM_STEP = 7;     // failing at/after this step fires the pump
const int MAX_FAILS = 10;         // penalties before game over (spares the water bottle)

const int TOUCH_MARGIN = 100;     // touch threshold = per-key idle baseline + this
                                  // (auto-calibrated at boot; replaces the old fixed
                                  //  SENSITIVITY 100/170-per-power-supply constant)
const int NOTE_DURATION = 50;     // key tone length in ms; the lower, the snappier the feel
                                  // [reconstructed — the 2019 line was corrupted]
const unsigned long LED_FEEDBACK_MS = 600;  // how long the right/wrong LED stays lit
const unsigned long PUMP_MS = 1000;         // pump ON time on a late-game miss

const bool serialEnabled = true;  // debug log at 9600 baud

//################################
//#########  MELODIES ############
//################################
// Stored in flash (PROGMEM): read with pgm_read_word. The victory tunes that
// play on completing the game are just the tail of the full themes, so we keep
// ONE copy of each and play it from an offset (TODO #10 + #11).

// GAME OVER tune — index [0] holds the note count, then (note, 1/duration) pairs.
const int deathTune[] PROGMEM = {
  17, NOTE_C4, 32, NOTE_CS4, 32, NOTE_D4, 16, NOTE_REST, 4, NOTE_REST, 2,
  NOTE_B3, 8, NOTE_F4, 8, NOTE_REST, 8, NOTE_F4, 8, NOTE_F4, 6, NOTE_E4, 6,
  NOTE_D4, 6, NOTE_C4, 8, NOTE_E3, 8, NOTE_REST, 8, NOTE_E3, 8, NOTE_C3, 8
};

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
bool dead = false;             // true after MAX_FAILS: locked until RESTART
int  currentStep = 0;          // how many correct notes so far (index into the sequence)
int  fails = 0;                // penalties applied this game
int  pressedNote = 0;          // last note played

int  keyThreshold[KEY_COUNT];  // per-key touch threshold (auto-calibrated)
bool keyHeld[KEY_COUNT];       // previous touched-state, for rising-edge detection

bool ledActive = false;        // a feedback LED is currently lit
unsigned long ledOnAt = 0;     // millis() when it was lit

//################################
//#######  PROTOTYPES ############
//################################
void calibrate();
void selectGame();
void handleGuess();
void playSong(const int *notes, const int *tempos, uint8_t from, uint8_t length);
void playDeath();
void firePump();
void pumpOff();
void buzz(int targetPin, long frequency, long length);
void log(const __FlashStringHelper *msg);


//################################
//###########  SETUP #############
//################################
void setup() {
  if (serialEnabled) {
    Serial.begin(9600);
    Serial.println(F("Lemon Piano V4"));
  }

  // Set the pump to a defined OFF state BEFORE the pins become outputs, so an
  // active-LOW relay module can't chatter the pump at boot (TODO #2).
  digitalWrite(RELAY_1, LOW);
  digitalWrite(RELAY_2, HIGH);

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(GAME_SELECT, INPUT);
  pinMode(RESTART, INPUT);
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(BUZZ_LED, OUTPUT);
  // A0..A6 need no pinMode for analogRead. (The 2019 pinMode(0..7, INPUT) block
  // is gone — it clobbered the D0/D1 UART pins for no benefit; TODO #4.)

  pumpOff();
  calibrate();
}


//################################
//###########  LOOP ##############
//################################
void loop() {
  // Restart at any time.
  if (digitalRead(RESTART) == HIGH) {
    started = false;
  }

  // (Re)select the game. selectGame() clears fails/step/dead, so a restart
  // after death starts truly fresh (TODO #3).
  if (!started) {
    selectGame();
    started = true;
  }

  const int keyboardOffset = (game == 1) ? 0 : 7;

//################################
//########  READ INPUT ###########
//################################
  // Scan all keys, update held-state, and act on the FIRST fresh press this
  // loop (rising edge only — debounced by hysteresis on the threshold; TODO #8).
  int justPressed = -1;
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    bool touched = analogRead(i) > keyThreshold[i];
    if (touched && !keyHeld[i] && justPressed < 0) {
      justPressed = i;
    }
    keyHeld[i] = touched;
  }

  if (justPressed >= 0) {
    pressedNote = keys[justPressed + keyboardOffset];
    if (dead) {
      // Locked out until RESTART: any touch replays the game-over tune.
      playDeath();
    } else {
      tone(BUZZER, pressedNote, NOTE_DURATION);
      handleGuess();
    }
  }

  // Turn the feedback LEDs off after LED_FEEDBACK_MS (real time via millis(),
  // not a loop counter that drifts with loop speed; TODO #7).
  if (ledActive && (millis() - ledOnAt >= LED_FEEDBACK_MS)) {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);
    ledActive = false;
  }
}


//################################
//#########  GAME LOGIC ##########
//################################

// Evaluate the note the player just pressed against the secret sequence.
void handleGuess() {
  const int *sequence = (game == 1) ? sequence_1 : sequence_2;
  const int expected = sequence[currentStep];

  if (pressedNote == expected) {
    // CORRECT
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    ledActive = true;
    ledOnAt = millis();
    currentStep++;

    if (currentStep >= SEQUENCE_LENGTH) {
      // VICTORY — play the theme's tail, then reset to a clean slate.
      log(F("WIN"));
      if (game == 1) {
        playSong(marioNotes, marioTempo, MARIO_VICTORY_FROM, MARIO_LEN);
      } else {
        playSong(underworldNotes, underworldTempo, UNDER_VICTORY_FROM, UNDER_LEN);
      }
      currentStep = 0;
      fails = 0;
      pressedNote = 0;   // clear the last key so we don't instantly "fail" (TODO #1)
      digitalWrite(GREEN_LED, LOW);
      ledActive = false;
    }
  } else {
    // WRONG
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    ledActive = true;
    ledOnAt = millis();
    log(F("WRONG"));

    // Failing late in the sequence triggers the water pump.
    if (currentStep >= PUMP_FROM_STEP) {
      firePump();
      fails++;
      if (fails >= MAX_FAILS) {
        dead = true;
        log(F("GAME OVER"));
        playDeath();
      }
    }
    currentStep = 0;   // wrong note -> back to the start
  }
}

// Sample each key's floating idle level and set its touch threshold above it.
// Auto-handles the "raise SENSITIVITY on a laptop charger" note from 2019 by
// measuring the actual baseline instead of hardcoding it (TODO #12).
// Keep hands OFF the lemons during boot.
void calibrate() {
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

// Pick the game from the select button and reset all per-game state.
void selectGame() {
  game = (digitalRead(GAME_SELECT) == HIGH) ? 1 : 2;
  currentStep = 0;
  fails = 0;
  dead = false;
  pressedNote = 0;
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    keyHeld[i] = false;
  }
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  ledActive = false;
  if (serialEnabled) {
    Serial.print(F("Game "));
    Serial.println(game);
  }
}


//################################
//#########  ACTUATORS ###########
//################################

void pumpOff() {
  digitalWrite(RELAY_1, LOW);
  digitalWrite(RELAY_2, HIGH);
}

// Fire the water pump for PUMP_MS with a low warning groan, then switch off.
void firePump() {
  digitalWrite(RELAY_1, HIGH);
  digitalWrite(RELAY_2, LOW);
  tone(BUZZER, NOTE_D1);
  delay(PUMP_MS);
  noTone(BUZZER);
  pumpOff();
}


//################################
//#########  AUDIO ###############
//################################

// Play a melody from PROGMEM, notes[from..length). Blocking (TODO #9: this is
// intentional — the game pauses while the tune plays).
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

// Play the game-over tune (note, 1/duration pairs from PROGMEM).
void playDeath() {
  int count = (int) pgm_read_word(&deathTune[0]);
  for (int i = 1; i < count * 2 + 1; i += 2) {
    int frequency = (int) pgm_read_word(&deathTune[i]);
    int noteType = (int) pgm_read_word(&deathTune[i + 1]);
    int noteDuration = 1000 / noteType;
    tone(BUZZER, frequency, noteDuration);
    delay((unsigned long)(noteDuration * 1.30));
    noTone(BUZZER);
  }
}

// Bit-banged square wave used by playSong (lights BUZZ_LED per note).
void buzz(int targetPin, long frequency, long length) {
  if (frequency <= 0) {
    return;  // rest / stop — nothing to toggle (and avoids a divide-by-zero; TODO #5)
  }
  digitalWrite(BUZZ_LED, HIGH);
  long delayValue = 1000000 / frequency / 2;    // half-period in microseconds
  long numCycles = frequency * length / 1000;   // cycles for the requested length
  for (long i = 0; i < numCycles; i++) {
    digitalWrite(targetPin, HIGH);   // push the diaphragm out
    delayMicroseconds(delayValue);
    digitalWrite(targetPin, LOW);    // pull it back
    delayMicroseconds(delayValue);
  }
  digitalWrite(BUZZ_LED, LOW);
}

// Serial log helper — a single guard point for all debug output (TODO #4).
void log(const __FlashStringHelper *msg) {
  if (serialEnabled) {
    Serial.println(msg);
  }
}
