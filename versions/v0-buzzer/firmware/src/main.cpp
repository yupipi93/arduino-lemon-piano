/* LEMON PIANO V0 — buzzer bring-up: play a scale, forever
   Author : Sergio Conejero, 2026-07-26

   The smallest board in the project: an ATmega328 and ONE passive buzzer on D8.
   No keys, no LEDs (bar the on-board one), no game, no touch sensing — so if
   this does not sound right, the problem is the buzzer, its wiring, or the pin,
   and nothing else.

   What it does, in a loop that never ends:
     C4 D4 E4 F4 G4 A4 B4 C5   (ascending)
     C5 B4 A4 G4 F4 E4 D4      (descending, back to the start)
     ... short pause, repeat.

   Each note is NOTE_MS long with a real GAP_MS of SILENCE after it, so you can
   hear where every note starts and stops. The on-board LED (D13) lights for the
   duration of each note: if the LED steps through the scale but you hear
   nothing (or a rattle), the sketch is fine and the fault is downstream.
   Every note is also printed at 9600 baud.

   Two playback paths, so you can tell WHICH one degraded:
     - default            -> tone() / noTone(), the AVR hardware timer path used
                             for key notes in every version.
     - -DUSE_BUZZ         -> the bit-banged square wave (buzz(), digitalWrite +
                             delayMicroseconds) that playSong() uses for the
                             Mario themes. Build it with `pio run -e nanoatmega328-buzz`.

   Buzzer pin: D8 on hardware — the same pin every version uses, so nothing has
   to be rewired to run this. In the Velxio browser build the buzzer moves to
   D11 (see emuTone below).
*/

#include <Arduino.h>

//################################
//###########  PINS ##############
//################################
#ifdef VELXIO_EMULATION
#define BUZZER 11  // Velxio polls Timer2 duty only on PWM pins (3/5/6/9/10/11)
                   // and fires note-off only on a duty->0 event; on D8 the first
                   // tone would beep forever (see emuTone below)
#else
#define BUZZER 8   // hardware: the project's buzzer pin in every version
#endif
#define BUZZ_LED 13  // on-board LED — lit for the duration of each note

//################################
//#########  THE SCALE ###########
//################################
// One octave of C major. Frequencies in Hz (equal temperament, A4 = 440).
#define NOTE_C4 262
#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_B4 494
#define NOTE_C5 523

const int SCALE[] = {
  NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5,
  NOTE_B4, NOTE_A4, NOTE_G4, NOTE_F4, NOTE_E4, NOTE_D4,
};
const uint8_t SCALE_LENGTH = sizeof(SCALE) / sizeof(SCALE[0]);

const unsigned long NOTE_MS  = 300;   // how long each note sounds
const unsigned long GAP_MS   = 80;    // silence between notes (hear each attack)
const unsigned long PAUSE_MS = 700;   // silence between passes of the scale

const bool serialEnabled = true;      // note log at 9600 baud

//################################
//#######  BUZZER PATHS ##########
//################################

#ifdef VELXIO_EMULATION
// Velxio's buzzer part starts a WebAudio note when Timer2 duty goes >0 and stops
// it ONLY on a duty->0 event, and noTone() leaves OCR2A set. Clearing OCR2A after
// each note is what makes the note actually END in the browser.
void emuTone(int frequency, unsigned long duration) {
  if (frequency <= 0) return;
  tone(BUZZER, frequency);
  delay(duration);
  noTone(BUZZER);
  OCR2A = 0;
}
#endif

#ifdef USE_BUZZ
// The bit-banged square wave used by playSong() for the Mario themes: toggle the
// pin by hand, half a period at a time. Blocking, and it holds the pin, so it
// cannot share the timer with tone().
void buzz(int pin, long frequency, long length) {
  if (frequency <= 0) return;                       // no divide-by-zero
  long halfPeriodUs = 1000000L / frequency / 2;
  long cycles = frequency * length / 1000;
  for (long i = 0; i < cycles; i++) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(halfPeriodUs);
    digitalWrite(pin, LOW);
    delayMicroseconds(halfPeriodUs);
  }
}
#endif

// Play one note through whichever path this build selected, then go silent.
void playNote(int frequency) {
  digitalWrite(BUZZ_LED, HIGH);

#if defined(VELXIO_EMULATION)
  emuTone(frequency, NOTE_MS);
#elif defined(USE_BUZZ)
  buzz(BUZZER, frequency, (long) NOTE_MS);
#else
  tone(BUZZER, frequency);
  delay(NOTE_MS);
  noTone(BUZZER);
#endif

  digitalWrite(BUZZ_LED, LOW);
  delay(GAP_MS);                  // real silence — this is what makes a dropout
}                                 // or a stuck note obvious

//################################
//###########  SETUP #############
//################################
void setup() {
  if (serialEnabled) {
    Serial.begin(9600);
    Serial.println(F("Lemon Piano V0 - buzzer scale"));
#ifdef USE_BUZZ
    Serial.println(F("path: bit-banged buzz()"));
#else
    Serial.println(F("path: tone()"));
#endif
    Serial.print(F("buzzer pin: D"));
    Serial.println(BUZZER);
  }

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
  pinMode(BUZZ_LED, OUTPUT);
  digitalWrite(BUZZ_LED, LOW);
}


//################################
//###########  LOOP ##############
//################################
void loop() {
  for (uint8_t i = 0; i < SCALE_LENGTH; i++) {
    if (serialEnabled) {
      Serial.print(F("note "));
      Serial.print(i + 1);
      Serial.print(F("/"));
      Serial.print(SCALE_LENGTH);
      Serial.print(F(" - "));
      Serial.print(SCALE[i]);
      Serial.println(F(" Hz"));
    }
    playNote(SCALE[i]);
  }

  if (serialEnabled) Serial.println(F("scale done - pausing"));
  delay(PAUSE_MS);
}
