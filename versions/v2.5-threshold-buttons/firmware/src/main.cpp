/* LEMON PIANO V2.5 — keyboard test with a LIVE touch threshold
   Author : Sergio Conejero, 2026-07-27

   V2 (../../v2-keyboard-test/) is the 2019 keyboard test rig: 7 fruit keys on
   A0..A6, one fixed note each on the buzzer, and a hardcoded touch threshold —
   the famous `<= 1019`. To change the sensitivity you had to edit, recompile and
   reflash, for a number that depends on the fruit, the power supply, the mains
   outlet and where your feet are.

   V2.5 turns that into a knob you can turn while your hand is on a lemon.

   Hardware delta vs V2: TWO BUTTONS, and nothing else.
     - D10 -> MORE sensitive (smaller margin: a lighter touch fires)
     - D11 -> LESS sensitive (bigger margin: needs a firmer grip, ignores noise)
     - both held 1 s -> recalibrate every key's baseline (hands off the fruit)
   The knob is a MARGIN from each key's own resting level, not an absolute
   reading — see the BEHAVIOUR block for why that is the only thing that can work
   on a rig whose idle level drifts.
   Wired button-to-GND with the AVR's internal pull-ups (INPUT_PULLUP), so they
   need no external resistors — a deliberate difference from V3/V4.5, whose
   active-HIGH buttons each need a 10 kΩ pulldown. This is a bench instrument;
   fewer parts on the breadboard wins.

   And the serial monitor is ON (9600 baud), which V2 never had — its
   `Serial.begin` is commented out. It prints, continuously:
     - a live readout of all seven raw averages against the threshold,
     - every threshold change,
     - every note as it plays, with the reading that triggered it.

   Sensing polarity is V2's, i.e. the 2019 wiring: each pin is pulled UP to 5 V
   through 220 Ω and idles near 1023; the player holds a GND clip, so touching a
   fruit drags that pin DOWN. Hence "touched" means `average <= threshold`.
   >>> If your board is wired the 2026 way instead (clip on +5 V, 220 Ω in series,
   pins floating near 0 and RISING on touch), this comparison is backwards and no
   threshold value can work. Flip TOUCH_WHEN_BELOW to false for that wiring. <<<
*/

#include <Arduino.h>

//################################
//###########  PINS ##############
//################################
#ifdef VELXIO_EMULATION
#define BUZZER 11           // MUST be D11 in the browser, not merely "a PWM pin".
                            // Velxio's buzzer ends a note only on a Timer2
                            // duty->0 event and noTone() leaves OCR2A set, so the
                            // note-off is the explicit `OCR2A = 0` below — and
                            // OCR2A is Timer2/OC2A, i.e. D11. On D9 (Timer1) that
                            // write clears nothing and the note beeps forever;
                            // that is the 2026-07-13 "endless beep" bug, see
                            // CHANGELOG.md.
#define KEY7_PIN 12         // A6 does not exist in the browser AVR (avr8js
                            // exposes A0..A5), so key 7 moves to a digital pin
#define THRESHOLD_UP 10     // button to GND
#define THRESHOLD_DOWN 7    // D11 is the buzzer here, so THRESHOLD - moves to D7
#else
#define BUZZER 8            // same buzzer pin as every version of this project
#define THRESHOLD_UP 10     // button to GND (internal pull-up)
#define THRESHOLD_DOWN 11   // button to GND (internal pull-up)
#endif

#define CAL_LED 13          // the board's ON-BOARD LED: lit = calibrating,
                            // hands off the fruit. No external part needed.

const uint8_t KEY_COUNT = 7;      // A0..A6

// ── Velxio emulation notes ──────────────────────────────────────────────────
// The 2019 sensing polarity this version inherits (pins pulled UP, a touch drags
// them DOWN) is exactly what a pushbutton + pull-up does in the browser: idle
// ~1023, pressed ~0. So keys 1-6 need NO shim at all — only the buzzer pin and
// key 7 move. The threshold buttons work unchanged (active-low), but Velxio does
// not model the AVR's internal pull-ups, so the spec wires external 10k ones.

//################################
//#########  BEHAVIOUR ###########
//################################
// The knob is SENSITIVITY, not an absolute reading. A key counts as touched when
// its reading has moved TOUCH_MARGIN counts away from that key's own resting
// level — so the number you dial means the same thing on any fruit, any supply
// and at any time of day, which an absolute threshold cannot (this rig's idle
// level was measured drifting ~170 counts on a ~25 s cycle).
//
//   threshold[i] = baseline[i] - margin   with TOUCH_WHEN_BELOW (2019 wiring)
//   threshold[i] = baseline[i] + margin   otherwise               (2026 wiring)
//
// SMALL margin = very sensitive (a light touch fires, noise may too).
// LARGE margin = insensitive (needs a firm grip, immune to noise).
//
// true  = 2019 wiring: pins pulled up through 220 Ohm, a touch drags them DOWN
// false = 2026 wiring: pins float low, +5 V clip, a touch pushes them UP
const bool TOUCH_WHEN_BELOW = true;

// Measured on the real 2019-wired rig (2026-07-27): idle 1023, and a fruit touch
// dips the reading to 1019 — a 3-4 count signal, because ~1 MOhm of skin against
// a 220 Ohm pull-up is a very lopsided divider. That is exactly where the 2019
// sketch's `<= 1019` came from. So the knob has to work in SINGLE counts down
// here; a default of 40 or a floor of 4 can never fire on this wiring.
// Default = the working point found on the real rig on 2026-07-27: baseline 1022
// minus 4 counts = threshold 1018, which detects a fruit touch reliably. Because
// the knob is a margin, this stays the same *sensitivity* even if the baseline
// moves — that is the whole reason it is not stored as "1018".
int  touchMargin = 4;             // THE KNOB, in ADC counts
const int MARGIN_MIN = 1;         // 1 count: as sensitive as the ADC allows
const int MARGIN_MAX = 600;       // above this nothing can ever reach it
const int MARGIN_STEP_FINE = 1;   // step below MARGIN_COARSE_ABOVE
const int MARGIN_STEP_COARSE = 5; // step above it
const int MARGIN_COARSE_ABOVE = 20;

// Baseline = each key's resting level, measured at boot and then tracked slowly
// while the key is NOT touched, so a drifting idle level cannot make the knob
// lie. A touch never feeds the baseline; a key that reads "touched" for longer
// than STUCK_MS is assumed to be noise, not a finger, and is re-baselined.
const uint8_t CAL_SAMPLES = 24;                  // boot calibration depth (fast: ~200 ms)
const unsigned long BASELINE_EVERY_MS = 100;     // drift-tracking interval
const int BASELINE_DIVISOR = 8;                  // baseline += (reading-baseline)/8
const unsigned long STUCK_MS = 5000;             // stuck-key re-baseline timeout
const unsigned long RECAL_HOLD_MS = 1000;        // both buttons this long = smart adjust

// Boot auto-calibration: the margin is derived from the MEASURED noise, so a
// quiet rig gets a tight margin and a noisy one gets a wide one, with no dial to
// turn. The floor is 4 because that is the working point measured on the real
// 2019-wired rig (baseline 1022, touch dips to 1018).
const int NOISE_FACTOR = 2;        // auto margin = NOISE_FACTOR x worst noise
const int AUTO_MARGIN_MIN = 4;     // ...but never tighter than this

// Smart adjust (both buttons 1 s) samples while the player HOLDS a lemon, then
// puts the margin midway between the idle noise floor and the depth of that real
// touch — the widest separation available on this fruit, right now.
const unsigned long LEARN_MS = 600;
const uint8_t LEARN_BURSTS = 4;    // sampling bursts, with a blip between each

// UI sound feedback for every state change (set false for a silent rig).
const bool UI_SOUNDS = true;
const int UI_TICK_MS = 14;

const unsigned long BUTTON_DEBOUNCE_MS = 40;
const unsigned long REPEAT_DELAY_MS = 400;   // hold this long to start ramping
const unsigned long REPEAT_EVERY_MS = 120;   // then one step this often
const unsigned long READOUT_EVERY_MS = 500;  // live readout period
const int NOTE_MS = 150;                     // note length, as in V2

// Key notes: the SAME seven the game plays in game 1 (Mario Main Theme), taken
// from ../../v5-led-bar/firmware/include/notes.h. V2 used a low C3..B3 octave,
// which sounds nothing like the piano these lemons are being tuned for — this
// way the rig you tune with and the game you play use the same voice.
const int NOTES[KEY_COUNT] = {1319, 1568, 1760, 1976, 2093, 2637, 3136};
const char *NOTE_NAMES[KEY_COUNT] = {"E6", "G6", "A6", "B6", "C7", "E7", "G7"};

//################################
//#########  STATE ###############
//################################
int  average[KEY_COUNT];               // last 4-sample average per key
int  baseline[KEY_COUNT];              // each key's resting level (tracked)
int  noiseLevel[KEY_COUNT];            // peak-to-peak idle noise, from calibration
int  lastSoundedKey = -1;              // a key sounds ONCE until another is played
bool keyDown[KEY_COUNT];               // edge detection, so a held key plays once
unsigned long touchedSince[KEY_COUNT]; // when this key started reading touched
unsigned long lastReadout = 0;
unsigned long lastBaselineTick = 0;

struct Button {
  uint8_t pin;
  int step;
  bool pressed;
  unsigned long changedAt;
  unsigned long nextRepeat;
};
// D10 = "more sensitive" = a SMALLER margin, so its direction is negative. The
// labels on the diagram and the box read in feel, not in counts. The step size is
// chosen per press by stepSize(), fine near the bottom where the signal lives.
Button buttons[2] = {
  {THRESHOLD_UP,   -1, false, 0, 0},   // D10: more sensitive
  {THRESHOLD_DOWN, +1, false, 0, 0},   // D11: less sensitive
};
unsigned long bothHeldSince = 0;       // both buttons held = recalibrate

//################################
//#######  PROTOTYPES ############
//################################
int  readKey(uint8_t i);
int  thresholdFor(uint8_t i);
bool touched(uint8_t i);
void autoCalibrate();
void learnFromTouch();
void trackBaselines();
void serviceButtons();
void playTone(int freq, int ms);
void soundCalStart();
void soundCalDone();
void soundTick();
void soundLimit();
void soundLearnBlip();
void soundLearnOk();
void soundLearnFail();
void soundStuck();
int  stepSize();
void nudgeMargin(int direction);
void printReadout();


void setup() {
  Serial.begin(9600);
  Serial.println(F("Lemon Piano V2.5 - keyboard test, live SENSITIVITY"));
  Serial.print(F("buzzer pin: D")); Serial.println(BUZZER);
  Serial.print(F("more sensitive: D")); Serial.print(THRESHOLD_UP);
  Serial.print(F("   less sensitive: D")); Serial.println(THRESHOLD_DOWN);
  Serial.print(F("touch = reading moves "));
  Serial.print(TOUCH_WHEN_BELOW ? F("DOWN") : F("UP"));
  Serial.print(F(" by the margin from each key's baseline ("));
  Serial.print(TOUCH_WHEN_BELOW ? F("2019") : F("2026"));
  Serial.println(F(" wiring)"));
  Serial.print(F("margin=")); Serial.print(touchMargin);
  Serial.println(F("   (hold both buttons 1s WHILE TOUCHING a lemon = smart adjust)"));

  pinMode(BUZZER, OUTPUT);
  pinMode(CAL_LED, OUTPUT);
  digitalWrite(CAL_LED, LOW);
  pinMode(THRESHOLD_UP, INPUT_PULLUP);
  pinMode(THRESHOLD_DOWN, INPUT_PULLUP);
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    keyDown[i] = false;
  }

#ifdef VELXIO_EMULATION
  pinMode(KEY7_PIN, INPUT_PULLUP);   // key 7's digital stand-in

  // BOOT GUARD (same fix as 2026-07-13): Velxio's first SPICE solve takes a
  // moment, and until it drives our nets every input reads 0/LOW — which with
  // this version's polarity means "every key touched" AND "both threshold
  // buttons held", i.e. a burst of notes and a threshold running away on
  // auto-repeat. Wait for the solver to land before playing anything.
  Serial.println(F("Emulation build: waiting for circuit solve (inputs idle-high)..."));
  const int IDLE_HIGH = 512;   // pull-ups idle near 1023; 0 means "solver not up yet"
  while (readKey(0) < IDLE_HIGH || readKey(6) < IDLE_HIGH
         || digitalRead(THRESHOLD_UP) == LOW || digitalRead(THRESHOLD_DOWN) == LOW) {
    delay(10);
  }
  Serial.println(F("Inputs idle. Ready to play."));
#endif

  autoCalibrate();
  Serial.println(F("     reading (* = touched) ................................ | margin"));
}


void loop() {
  serviceButtons();

  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    average[i] = readKey(i);
  }
  trackBaselines();

  // A key sounds ONCE and then holds its lock: keeping the lemon pressed, or
  // letting go and touching it again, is silent. Only playing a DIFFERENT key
  // releases the lock. On fruit this is what makes flaky contact bearable — a
  // flickering touch used to machine-gun the buzzer.
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    bool now = touched(i);
    if (now && !keyDown[i]) {
      if ((int) i == lastSoundedKey) {
        Serial.print(F("    key ")); Serial.print(i + 1);
        Serial.println(F(" again - locked (play another key to unlock it)"));
        keyDown[i] = now;
        continue;
      }
      lastSoundedKey = i;
      Serial.print(F("KEY ")); Serial.print(i + 1);
      Serial.print(F(" ")); Serial.print(NOTE_NAMES[i]);
      Serial.print(F("  reading=")); Serial.print(average[i]);
      Serial.print(F("  baseline=")); Serial.print(baseline[i]);
      Serial.print(F("  threshold=")); Serial.print(thresholdFor(i));
      Serial.print(F("  margin=")); Serial.println(touchMargin);
      playTone(NOTES[i], NOTE_MS);
    }
    keyDown[i] = now;
  }

  printReadout();
}


//################################
//#########  SENSING #############
//################################

// V2's 4-sample average, kept as-is: it is what the 2019 rig used to fight noise.
int readKey(uint8_t i) {
#ifdef VELXIO_EMULATION
  if (i == 6) {
    // Key 7 is a digital button in the browser; report it on the same 0..1023
    // scale so the threshold comparison needs no special case.
    return digitalRead(KEY7_PIN) == LOW ? 0 : 1023;
  }
#endif
  long sum = 0;
  for (uint8_t n = 0; n < 4; n++) {
    sum += analogRead(i);
  }
  return (int) (sum / 4);
}

// Where this key's trigger point sits right now: its own resting level, moved by
// the margin in the direction a touch pushes it.
int thresholdFor(uint8_t i) {
  int t = TOUCH_WHEN_BELOW ? baseline[i] - touchMargin : baseline[i] + touchMargin;
  if (t < 0) t = 0;
  if (t > 1023) t = 1023;
  return t;
}

bool touched(uint8_t i) {
  return TOUCH_WHEN_BELOW ? (average[i] <= thresholdFor(i))
                          : (average[i] >= thresholdFor(i));
}

//################################
//#######  SOUND (UI) ############
//################################
// Every UI sound lives WELL BELOW the key notes (E6..G7 = 1319..3136 Hz), so a
// state chirp can never be mistaken for the piano being played. (They used to sit
// above the old low C3..B3 octave; moving the keys up to the game's own voice
// moved the UI down.) All of them are short and blocking: this is a bench rig,
// and a tick you can hear beats a tick that never interrupts anything.

// One tone, both builds. The browser needs the note-off written by hand (Velxio
// ends a note only on a Timer2 duty->0 event and noTone() leaves OCR2A set).
void playTone(int freq, int ms) {
  if (freq <= 0 || ms <= 0) return;
  tone(BUZZER, freq);
  delay(ms);
  noTone(BUZZER);
#ifdef VELXIO_EMULATION
  OCR2A = 0;
#endif
}

void uiGap(int ms) { delay(ms); }

// "Wait" — calibration starting, hands off. Falling pair.
void soundCalStart() {
  if (!UI_SOUNDS) return;
  playTone(700, 45); uiGap(10); playTone(450, 60);
}

// "Ready" — calibration finished. Rising pair, the all-clear to touch again.
void soundCalDone() {
  if (!UI_SOUNDS) return;
  playTone(450, 45); uiGap(8); playTone(700, 45); uiGap(8); playTone(950, 70);
}

// Button tick. The PITCH TRACKS THE MARGIN, so holding a button sweeps a
// glissando and you can hear where the setting is without reading anything:
// low pitch = wide margin (insensitive), high pitch = tight margin (sensitive).
// Range 250..950 Hz: under the key notes, so the two never blur.
void soundTick() {
  if (!UI_SOUNDS) return;
  int m = touchMargin;
  if (m > 60) m = 60;                       // squash the long tail
  int freq = 950 - (long) (m - MARGIN_MIN) * 700 / (60 - MARGIN_MIN);
  playTone(freq, UI_TICK_MS);
}

// "Bonk" — the knob is against its end stop and refuses to move.
void soundLimit() {
  if (!UI_SOUNDS) return;
  playTone(160, 70); uiGap(30); playTone(160, 70);
}

// Sonar blip, emitted BETWEEN sampling bursts during smart adjust (never during
// one: the buzzer current would ride into the analog readings we are measuring).
void soundLearnBlip() {
  if (!UI_SOUNDS) return;
  playTone(800, 14);
}

// "Learned it" — rising triad.
void soundLearnOk() {
  if (!UI_SOUNDS) return;
  playTone(500, 55); uiGap(8); playTone(700, 55); uiGap(8); playTone(950, 90);
}

// "Could not learn" — two falling low beeps, the classic error shape.
void soundLearnFail() {
  if (!UI_SOUNDS) return;
  playTone(300, 110); uiGap(40); playTone(200, 160);
}

// Warble: a key was stuck "touched" and got re-baselined. Something is wrong
// with that contact and you should know without watching the log.
void soundStuck() {
  if (!UI_SOUNDS) return;
  playTone(400, 40); uiGap(10); playTone(650, 40); uiGap(10); playTone(400, 40);
}


//################################
//#########  BASELINE ############
//################################

// Fast, smart, automatic: measure every key's resting level AND its idle noise,
// then derive the margin from the noise itself. The ON-BOARD LED (D13) is lit for
// the whole ~200 ms so the player knows to keep their hands off the fruit.
void autoCalibrate() {
  digitalWrite(CAL_LED, HIGH);
  Serial.println(F("Auto-calibrating - LED ON = HANDS OFF THE FRUIT..."));
  soundCalStart();                    // "wait" chirp, then silence to measure

  int worstNoise = 0;
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
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
    keyDown[i] = false;
  }
  lastSoundedKey = -1;

  // The margin has to clear the noise, or idle jitter becomes ghost notes.
  int autoMargin = worstNoise * NOISE_FACTOR;
  if (autoMargin < AUTO_MARGIN_MIN) autoMargin = AUTO_MARGIN_MIN;
  if (autoMargin > MARGIN_MAX) autoMargin = MARGIN_MAX;
  touchMargin = autoMargin;

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

  digitalWrite(CAL_LED, LOW);
  soundCalDone();                     // all clear: you may touch the fruit
}


// ── 3. Smart adjust: learn the margin from a REAL touch ─────────────────────
// Triggered by holding both buttons for 1 s WHILE TOUCHING A LEMON. It watches
// every channel for LEARN_MS, works out which key the finger is on (the biggest
// movement away from that key's baseline, in the direction a touch pushes it),
// measures how far the other channels wander meanwhile (the noise floor), and
// puts the margin halfway between the two — the cleanest separation this fruit
// can currently give. If the touch is not clearly above the noise it says so and
// changes nothing, because a bad margin is worse than the old one.
void learnFromTouch() {
  digitalWrite(CAL_LED, HIGH);
  Serial.println(F("SMART ADJUST - KEEP TOUCHING THE LEMON while the LED is on..."));

  int lo[KEY_COUNT], hi[KEY_COUNT];
  for (uint8_t i = 0; i < KEY_COUNT; i++) { lo[i] = 1023; hi[i] = 0; }

  // Sample in bursts with an audible blip BETWEEN them, never during: the buzzer
  // draws current through the same ground as the analog front end, so a tone
  // playing while we measure would pollute the very reading we are learning from.
  for (uint8_t burst = 0; burst < LEARN_BURSTS; burst++) {
    unsigned long until = millis() + LEARN_MS / LEARN_BURSTS;
    while ((long) (until - millis()) > 0) {
      for (uint8_t i = 0; i < KEY_COUNT; i++) {
        int v = readKey(i);
        if (v < lo[i]) lo[i] = v;
        if (v > hi[i]) hi[i] = v;
      }
    }
    soundLearnBlip();                 // "still listening"
  }

  // How far did each channel move, in the direction a touch moves it?
  int moved[KEY_COUNT];
  int best = -1, bestDepth = 0;
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    moved[i] = TOUCH_WHEN_BELOW ? (baseline[i] - lo[i]) : (hi[i] - baseline[i]);
    if (moved[i] < 0) moved[i] = 0;
    if (moved[i] > bestDepth) { bestDepth = moved[i]; best = i; }
  }
  // Noise floor = the worst wander among the keys NOT being touched.
  int floorNoise = 0;
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    if ((int) i != best && moved[i] > floorNoise) floorNoise = moved[i];
  }

  Serial.print(F("  strongest: key "));
  Serial.print(best + 1);
  Serial.print(F("  depth=")); Serial.print(bestDepth);
  Serial.print(F("  noise floor (other keys)=")); Serial.println(floorNoise);

  if (best < 0 || bestDepth < floorNoise + 2) {
    Serial.println(F("  !! touch not separable from noise - margin unchanged."));
    Serial.println(F("     Were you touching a lemon? Is the GND clip in your hand?"));
    soundLearnFail();
  } else {
    int m = (floorNoise + bestDepth) / 2;
    if (m < MARGIN_MIN) m = MARGIN_MIN;
    if (m > MARGIN_MAX) m = MARGIN_MAX;
    touchMargin = m;
    Serial.print(F("  learned margin=")); Serial.print(touchMargin);
    Serial.print(F("  (midway between ")); Serial.print(floorNoise);
    Serial.print(F(" and ")); Serial.print(bestDepth);
    Serial.print(F(")  key ")); Serial.print(best + 1);
    Serial.print(F(" threshold=")); Serial.println(thresholdFor(best));
    soundLearnOk();
  }

  for (uint8_t i = 0; i < KEY_COUNT; i++) { keyDown[i] = false; touchedSince[i] = 0; }
  lastSoundedKey = -1;
  digitalWrite(CAL_LED, LOW);
}


// Follow the idle level slowly while a key is NOT touched, so drift cannot make
// the margin lie. A key stuck "touched" for STUCK_MS is noise, not a finger — it
// gets re-baselined so the rig recovers by itself.
void trackBaselines() {
  unsigned long now = millis();
  if (now - lastBaselineTick < BASELINE_EVERY_MS) return;
  lastBaselineTick = now;

  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    if (!touched(i)) {
      touchedSince[i] = 0;
      baseline[i] += (average[i] - baseline[i]) / BASELINE_DIVISOR;
    } else {
      if (touchedSince[i] == 0) {
        touchedSince[i] = now;
      } else if (now - touchedSince[i] > STUCK_MS) {
        Serial.print(F("!!! key ")); Serial.print(i + 1);
        Serial.print(F(" stuck touched for ")); Serial.print(STUCK_MS / 1000);
        Serial.println(F("s - re-baselining it (noise, not a finger?)"));
        soundStuck();
        baseline[i] = average[i];
        touchedSince[i] = 0;
        keyDown[i] = false;
      }
    }
  }
}


//################################
//#########  BUTTONS #############
//################################

// Debounced, with auto-repeat while held so a 0..1023 sweep does not need 200
// individual presses. Active-low: pressed == LOW (button shorts the pin to GND).
void serviceButtons() {
  unsigned long now = millis();

  // Both buttons held together = recalibrate the baselines (hands off first).
  bool bothDown = (digitalRead(THRESHOLD_UP) == LOW) &&
                  (digitalRead(THRESHOLD_DOWN) == LOW);
  if (bothDown) {
    if (bothHeldSince == 0) {
      bothHeldSince = now;
    } else if (now - bothHeldSince > RECAL_HOLD_MS) {
      bothHeldSince = 0;
      learnFromTouch();
      for (uint8_t b = 0; b < 2; b++) {          // swallow this press
        buttons[b].pressed = true;
        buttons[b].nextRepeat = now + REPEAT_DELAY_MS * 4;
      }
    }
    return;                                       // no margin change meanwhile
  }
  bothHeldSince = 0;

  for (uint8_t b = 0; b < 2; b++) {
    Button &btn = buttons[b];
    bool down = (digitalRead(btn.pin) == LOW);

    if (down != btn.pressed) {
      if (now - btn.changedAt < BUTTON_DEBOUNCE_MS) continue;   // bounce
      btn.pressed = down;
      btn.changedAt = now;
      if (down) {
        nudgeMargin(btn.step);
        btn.nextRepeat = now + REPEAT_DELAY_MS;
      }
    } else if (down && now >= btn.nextRepeat) {
      nudgeMargin(btn.step);
      btn.nextRepeat = now + REPEAT_EVERY_MS;
    }
  }
}

// One count at a time where the fruit signal actually is, five where you are just
// sweeping. Without this, a 3-count touch is unreachable from a step of 5.
int stepSize() {
  return touchMargin > MARGIN_COARSE_ABOVE ? MARGIN_STEP_COARSE : MARGIN_STEP_FINE;
}

void nudgeMargin(int direction) {
  int before = touchMargin;
  int delta = direction < 0 ? -stepSize() : stepSize();
  touchMargin += delta;
  if (touchMargin < MARGIN_MIN) touchMargin = MARGIN_MIN;
  if (touchMargin > MARGIN_MAX) touchMargin = MARGIN_MAX;
  if (touchMargin == before) {                       // against an end stop
    Serial.print(F("!! margin already at the "));
    Serial.print(before <= MARGIN_MIN ? F("MINIMUM (most sensitive)")
                                      : F("MAXIMUM (least sensitive)"));
    Serial.print(F(" = ")); Serial.println(before);
    soundLimit();
    return;
  }
  soundTick();                                       // pitch tracks the margin
  Serial.print(F(">>> margin=")); Serial.print(touchMargin);
  Serial.print(direction < 0 ? F("  (more sensitive)") : F("  (less sensitive)"));
  Serial.print(F("  thresholds now "));
  Serial.print(thresholdFor(0)); Serial.print(F(".."));
  Serial.println(thresholdFor(KEY_COUNT - 1));
}


//################################
//#########  READOUT #############
//################################

// The live picture: every average next to the threshold, with a marker on each
// channel currently counted as touched. This is the instrument — watch it while
// you touch a lemon and pick a threshold that only marks the key you touched.
void printReadout() {
  unsigned long now = millis();
  if (now - lastReadout < READOUT_EVERY_MS) return;
  lastReadout = now;

  Serial.print(F("     "));
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    Serial.print(touched(i) ? '*' : ' ');
    Serial.print(average[i]);
    Serial.print('/');
    Serial.print(baseline[i]);          // reading / its own baseline
    Serial.print(' ');
  }
  Serial.print(F("| margin="));
  Serial.println(touchMargin);
}
