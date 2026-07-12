/* LEMON PIANO V4
   Author : Yupipi93
   Date : 02/2019

   CODE 1: 6,5,6,7,2,5,2,1,3,4,X
   CODE 2: 3,6,1,4,2,5,3,6,1,4,X

   English translation of the rescued Piano_Limones_v4.ino (2026-07-12).
   Behavior is intentionally identical to the original, bugs included —
   see TODO.md for the fix roadmap. The only functional reconstruction is
   NOTE_DURATION: the original line was corrupted by accidental keystrokes
   ("= 5çkp`ñ´sca...0;") and has been restored to 50 ms.
*/

#include <Arduino.h>
#include "notes.h"

//################################
//#########  MELODIES ############
//################################

// GAME OVER MELODY (Mario death tune; index 0 holds the note count)
const int death[] = {17, NOTE_C4, 32, NOTE_CS4, 32, NOTE_D4, 16, NOTE_REST, 4, NOTE_REST, 2, NOTE_B3, 8, NOTE_F4, 8, NOTE_REST, 8, NOTE_F4, 8, NOTE_F4, 6, NOTE_E4, 6, NOTE_D4, 6, NOTE_C4, 8, NOTE_E3, 8, NOTE_REST, 8, NOTE_E3, 8, NOTE_C3, 8};



// MAIN THEME NOTES (FULL MELODY)
const int melody[] = {
  NOTE_E7, NOTE_E7, 0, NOTE_E7,
  0, NOTE_C7, NOTE_E7, 0,
  NOTE_G7, 0, 0,  0,
  NOTE_G6, 0, 0, 0,

  NOTE_C7, 0, 0, NOTE_G6,
  0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6,
  0, NOTE_AS6, NOTE_A6, 0,

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

// MAIN THEME TEMPOS (FULL MELODY)
const int tempo[] = {
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
  12, 12, 12, 12,
};

// MAIN THEME NOTES, TRIMMED (WHAT PLAYS WHEN YOU COMPLETE THE GAME)
const int melody_cut[] = {// -7 columns
  0, NOTE_AS6, NOTE_A6, 0,

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

// MAIN THEME TEMPOS, TRIMMED (WHAT PLAYS WHEN YOU COMPLETE THE GAME)
const int tempo_cut[] = {// -7 columns
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
  12, 12, 12, 12,
};

// UNDERWORLD NOTES (FULL MELODY)
const int underworld_melody[] = {
  NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
  NOTE_AS3, NOTE_AS4, 0,
  0,
  NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
  NOTE_AS3, NOTE_AS4, 0,
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

// UNDERWORLD TEMPOS (FULL MELODY)
const int underworld_tempo[] = {
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

// UNDERWORLD NOTES, TRIMMED (WHAT PLAYS WHEN YOU COMPLETE THE GAME)
const int underworld_melody_cut[] = {// -4 rows
  NOTE_AS3, NOTE_AS4, 0,
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

// UNDERWORLD TEMPOS, TRIMMED (WHAT PLAYS WHEN YOU COMPLETE THE GAME)
const int underworld_tempo_cut[] = {// -4 rows
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

//################################
//###########  PINS ##############
//################################
#define BUZZER 8
#define RED_LED 2
#define GREEN_LED 3
#define GAME_SELECT 4
#define RELAY_1 5
#define RELAY_2 6

//################################
//#########  CONSTANTS ###########
//################################
const int MAX_FAILS = 10;      // Allowed penalties (keeps the water bottle from emptying too fast)
const int SENSITIVITY = 100;   // Raise if it sounds without touching the lemons (170 for a laptop on charger)
const int NOTE_DURATION = 50;  // Note duration; feels like real time (the lower, the better the feel)
                               // [reconstructed — original line corrupted by stray keystrokes]
const bool serialEnabled = true;

//################################
//#########  VARIABLES ###########
//################################

int sequenceLength = 10;  // NUMBER OF NOTES TO GUESS (MAX 10)
int noteIndex = -1;       // INDEX INTO THE SEQUENCE ARRAY (ADVANCES WITH EACH CORRECT NOTE, RESETS ON FAIL)
int pressedNote = 0;      // LAST NOTE PRESSED
int game;                 // (SWAP 1,2 FOR 0 AND 7)  GAME SELECTOR (1 = MARIO MAIN THEME, 2 = MARIO UNDERGROUND THEME)
int keyboardOffset;       // 0 SELECTS THE FIRST HALF OF THE KEYBOARD FOR GAME 1, 7 FOR GAME 2
int expectedNote;         // SEQUENCE NOTE AT THE CURRENT POSITION
bool started = false;     // CODE THAT ONLY RUNS WHILE started IS FALSE
int fails = 0;            // PENALTIES APPLIED SO FAR
bool dead = true;         // WHEN MAX PENALTIES ARE REACHED, PLAY THE DEATH MELODY IF A KEY IS TOUCHED
int count = 0;            // LOOP ITERATION COUNTER

// SECRET SEQUENCE 1
int sequence_1[10] = {NOTE_E7, NOTE_C7, NOTE_E7, NOTE_G7, NOTE_G6, NOTE_C7, NOTE_G6, NOTE_E6, NOTE_A6, NOTE_B6};

// SECRET SEQUENCE 2
int sequence_2[10] = {NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4, NOTE_AS3, NOTE_AS4, NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4};

// KEY NOTES: 0 to 6 for game 1, 7 to 13 for game 2
int keys[14] = {NOTE_E6, NOTE_G6, NOTE_A6, NOTE_B6, NOTE_C7, NOTE_E7, NOTE_G7, NOTE_A3, NOTE_AS3, NOTE_C4, NOTE_A4, NOTE_AS4, NOTE_C5, NOTE_D5};


void sing(int s);
void buzz(int targetPin, long frequency, long length);
void death_melody();


//################################
//###########  SETUP #############
//################################
void setup() {
  // Serial.begin(9600);

  pinMode(0, INPUT);
  pinMode(1, INPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(GAME_SELECT, INPUT);
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
}


//################################
//###########  LOOP ##############
//################################
void loop() {
  // SERIAL MONITOR READOUT
  /*
     for(int i = 0; i<7; i++){
       delay(120);
       Serial.print("Analog ");
       Serial.print(i);
       Serial.print(" : ");
       Serial.println(analogRead(i));
     }
   */
  if (digitalRead(7) == HIGH) {
    started = false;
  }


  // PICK THE GAME AT PROGRAM START (NEEDS IMPROVEMENT)
  if (!started) {
    analogWrite(RELAY_1, HIGH);
    analogWrite(RELAY_2, HIGH);
    started = true;
    if (digitalRead(GAME_SELECT) == HIGH) {
      game = 1;
      noteIndex = -1;
      pressedNote = 0;
    } else {
      game = 2;
      noteIndex = -1;
      pressedNote = 0;
    }
  }

  /*
    if(analogRead(0)>SENSITIVITY){
      if(analogRead(1)>SENSITIVITY){
        if(game = 1){
            game = 7;
        }else{
          game = 1;
        }
      }
    }
   */

  // PICK THE KEYBOARD HALF FOR THE SELECTED GAME (improve)
  if (game > 0) {
    if (game == 1) {
      keyboardOffset = 0;
    } else {
      keyboardOffset = 7;
    }

//################################
//########  READ INPUT ###########
//################################

    for (int i = 0; i < 7; i++) {
      if (analogRead(i) > SENSITIVITY) {
        tone(BUZZER, keys[i + keyboardOffset], NOTE_DURATION);
        if (pressedNote != keys[i + keyboardOffset]) {
          //RELAY_2 = 8; // test penalties
          noteIndex++;
        }
        pressedNote = keys[i + keyboardOffset];
        count = 0;
        dead = true;
      }
    }

//################################
//#########  CHECKS ##############
//################################


    /* if the last pressed note matches the current position of the sequence,
       the green LED lights for 5 seconds; each pressed note advances the sequence by 1.
       if the note does NOT match the sequence, the red LED lights and the sequence resets.
     */

    if (game == 3 && dead) {
      death_melody();
      dead = false;
    } else {


      // if you hit all 10 notes in a row, the full song plays
      if (noteIndex >= sequenceLength && game != 3) {
        noteIndex = 0;

        if (game == 1) {
          // Game 1
          sing(-1);  // trimmed continuation of song 1
          // sing(1); // full song 1
        }
        if (game == 2) {
          // Game 2
          sing(-2);  // trimmed continuation of song 2
          // sing(2); // full song 2
        }
        fails = 0;
      }

      // if pressedNote left its initial value, check that it matches the current position of the secret sequence
      if (pressedNote != 0) {
        if (game == 1) {
          expectedNote = sequence_1[noteIndex];
        } else {
          expectedNote = sequence_2[noteIndex];
        }

        // CORRECT
        if (pressedNote == expectedNote) {
          //digitalWrite(RED_LED, LOW);
          digitalWrite(GREEN_LED, HIGH);

        // WRONG
        } else {
          digitalWrite(GREEN_LED, LOW);
          digitalWrite(RED_LED, HIGH);
          // failing from note 7 onwards triggers the water pump
          if (noteIndex >= 7) {
            digitalWrite(RELAY_1, HIGH);
            digitalWrite(RELAY_2, LOW);
            tone(BUZZER, NOTE_D1);
            delay(1000);
            noTone(BUZZER);
            digitalWrite(RELAY_1, LOW);
            digitalWrite(RELAY_2, HIGH);
            fails++;
            if (fails >= MAX_FAILS) {
              game = 3;
              dead = true;
            }

          }  // note >= 7
          // reset
          noteIndex = -1;
          pressedNote = 0;
        }  // pressedNote == expectedNote or not

        // turn LEDs off after 5 seconds
        count++;
        if (count >= 50) {
          //pressedNote = 0;
          digitalWrite(GREEN_LED, LOW);
          digitalWrite(RED_LED, LOW);
          count = 0;
        }
      }
    }  // game not 3, or !dead
  }  // game > 0
}  // loop

//################################
//#########  FUNCTIONS ###########
//################################


int song = 0;

void sing(int s) {
  // iterate over the notes of the melody:
  song = s;
  if (song == 2) {
    Serial.println(" 'Underworld Theme'");
    int size = sizeof(underworld_melody) / sizeof(int);
    for (int thisNote = 0; thisNote < size; thisNote++) {

      // to calculate the note duration, take one second
      // divided by the note type.
      // e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
      int noteDuration = 1000 / underworld_tempo[thisNote];

      buzz(BUZZER, underworld_melody[thisNote], noteDuration);

      // to distinguish the notes, set a minimum time between them.
      // the note's duration + 30% seems to work well:
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);

      // stop the tone playing:
      buzz(BUZZER, 0, noteDuration);
    }

  } else if (song == -2) {
    Serial.println(" 'Underworld Theme CUT'");
    int size = sizeof(underworld_melody_cut) / sizeof(int);
    for (int thisNote = 0; thisNote < size; thisNote++) {

      int noteDuration = 1000 / underworld_tempo_cut[thisNote];

      buzz(BUZZER, underworld_melody_cut[thisNote], noteDuration);

      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);

      buzz(BUZZER, 0, noteDuration);
    }

  } else if (song == 1) {

    Serial.println(" 'Mario Theme'");
    int size = sizeof(melody) / sizeof(int);
    for (int thisNote = 0; thisNote < size; thisNote++) {

      int noteDuration = 1000 / tempo[thisNote];

      buzz(BUZZER, melody[thisNote], noteDuration);

      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);

      buzz(BUZZER, 0, noteDuration);
    }
  } else if (song == -1) {

    Serial.println(" 'Mario Theme CUT'");
    int size = sizeof(melody_cut) / sizeof(int);
    for (int thisNote = 0; thisNote < size; thisNote++) {

      int noteDuration = 1000 / tempo_cut[thisNote];

      buzz(BUZZER, melody_cut[thisNote], noteDuration);

      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);

      buzz(BUZZER, 0, noteDuration);
    }
  }
}

void buzz(int targetPin, long frequency, long length) {
  digitalWrite(13, HIGH);
  long delayValue = 1000000 / frequency / 2;  // calculate the delay value between transitions
  //// 1 second's worth of microseconds, divided by the frequency, then split in half since
  //// there are two phases to each cycle
  long numCycles = frequency * length / 1000;  // calculate the number of cycles for proper timing
  //// multiply frequency, which is really cycles per second, by the number of seconds to
  //// get the total number of cycles to produce
  for (long i = 0; i < numCycles; i++) {  // for the calculated length of time...
    digitalWrite(targetPin, HIGH);        // write the buzzer pin high to push out the diaphragm
    delayMicroseconds(delayValue);        // wait for the calculated delay value
    digitalWrite(targetPin, LOW);         // write the buzzer pin low to pull back the diaphragm
    delayMicroseconds(delayValue);        // wait again for the calculated delay value
  }
  digitalWrite(13, LOW);
}


void death_melody() {
  for (int thisNote = 1; thisNote < (death[0] * 2 + 1); thisNote = thisNote + 2) {
    tone(BUZZER, death[thisNote], (1000 / death[thisNote + 1]));
    delay((1000 / death[thisNote + 1]) * 1.30);
    noTone(BUZZER);
  }
}
