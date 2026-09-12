/*
  Lemon Piano — v0.7.1 board probe
  ================================

  Diagnostic firmware for the assembled PCB. It answers, in one flash, the three
  questions that "the board boots but nothing sounds" can mean:

    1. Do the seven key channels move when a lemon is touched, and by how much?
       The whole touch signal on this board is ~4 ADC counts out of 1023, so
       "it does not work" and "it works and the game's margin is 2 counts too
       wide" look identical from the outside. Here they are different numbers.

    2. Are the two SENS buttons stuck? On a 6 mm tact switch the internally
       shorted pins are the pair 4.5 mm apart. If the footprint grouped the
       6.5 mm pair instead, SENS+ / SENS- are hard shorted to GND and read
       permanently held. This prints their state.

    3. Does D13 drive BUZ1 at all? It beeps once at startup, before touching
       anything, and D13 is also the Nano's on-board L LED — so a silent beep
       with a blinking L is a transducer fault, not a firmware one.

  Board pin map (pcb/docs/NETLIST.md, v0.7.1 copper):
     A0..A6  keys 1..7, each with a 220 ohm pull-up to +5V. The PLAYER HOLDS
             GND (J2 pin 8). A touch drags the pin DOWN.
     A7      SENS- button, external 10k pull-up (R18), button to GND.
     D12     SENS+ button, internal pull-up, button to GND.
     D13     passive buzzer BUZ1 (+ J5 in parallel).
     D2..D11 the ten LEDs. Deliberately left alone here.

  Each channel is read TWICE and the first read discarded. At 220 ohm that
  changes nothing; if the pull-ups are ever raised to 100k-1M (the fix proposed
  in pcb/docs/REVIEW-v0.7.1-silent-keys.md) the first read after a mux switch is
  mostly the previous channel's residue, and this probe must not lie about that.
*/

#include <Arduino.h>

const uint8_t KEY_COUNT = 7;
const uint8_t SENS_UP   = 12;   // D12, internal pull-up
const uint8_t SENS_DOWN = A7;   // analog-only, external 10k pull-up
const uint8_t BUZZER    = 13;   // D13, shared with the on-board L LED

const uint8_t  CAL_SAMPLES  = 32;
const uint16_t REPORT_MS    = 100;    // 10 Hz
const int      SHOW_TOUCH_AT = 2;     // mark a channel once it has moved this
                                      // far from baseline, in ADC counts. Two,
                                      // not four: the point is to SEE a signal
                                      // the game would have rejected.

// This probe is deliberately DIRECTION-AGNOSTIC. The game firmware only ever
// looks DOWN (thresholdFor = baseline - margin), and so does its smart-adjust
// gesture: learnFromTouch() tracks lo[] only, so a touch that raises a pin
// scores dropped = 0 on every channel and the gesture reports "touch not
// separable from noise - margin unchanged" no matter how wide MARGIN_MAX is.
// If the rig's touch goes UP, no amount of calibration range can find it, and
// a probe that also only looked down would hide exactly that. So this one
// reports both directions and names the winner.

int baseline[KEY_COUNT];
int noise[KEY_COUNT];
int minSeen[KEY_COUNT];
int maxSeen[KEY_COUNT];

// Read one channel, discarding the first conversion after the mux switch.
int readKey(uint8_t i) {
  analogRead(i);
  return analogRead(i);
}

void measureBaselines() {
  Serial.println(F("Calibrating - HANDS OFF THE FRUIT..."));
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    long sum = 0;
    int lo = 1023, hi = 0;
    for (uint8_t n = 0; n < CAL_SAMPLES; n++) {
      int v = readKey(i);
      sum += v;
      if (v < lo) lo = v;
      if (v > hi) hi = v;
      delay(2);
    }
    baseline[i] = (int) (sum / CAL_SAMPLES);
    noise[i]    = hi - lo;
    minSeen[i]  = 1023;
    maxSeen[i]  = 0;
  }

  int worstNoise = 0;
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    if (noise[i] > worstNoise) worstNoise = noise[i];
    Serial.print(F("  key ")); Serial.print(i + 1);
    Serial.print(F("  baseline=")); Serial.print(baseline[i]);
    Serial.print(F("  noise="));    Serial.println(noise[i]);
  }

  int gameMargin = worstNoise * 2;
  if (gameMargin < 4) gameMargin = 4;
  Serial.print(F("The GAME would have used margin="));
  Serial.print(gameMargin);
  Serial.println(F(" counts (max(4, 2 x worst noise))."));

  // The two readings that decide whether the front end exists at all.
  if (baseline[0] < 900) {
    Serial.println(F("!! baseline is NOT near 1022: the +5V rail or R1-R7 are"));
    Serial.println(F("!! missing. Check continuity R1 pad2 <-> U1 pad12 (5V)."));
  }
  if (gameMargin > 4) {
    Serial.println(F("!! margin > 4 is already wider than the whole 220-ohm"));
    Serial.println(F("!! touch signal. The game cannot trigger on this board."));
  }
}

void setup() {
  Serial.begin(9600);
  delay(200);
  Serial.println();
  Serial.println(F("=== Lemon Piano v0.7.1 board probe ==="));

  pinMode(SENS_UP, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  // Question 3, asked before anything else can go wrong.
  Serial.println(F("Beeping D13 for 400 ms - listen, and watch the L LED."));
  tone(BUZZER, 1000);
  delay(400);
  noTone(BUZZER);
  digitalWrite(BUZZER, LOW);
  Serial.println(F("  heard it?      -> BUZ1 and D13 are fine."));
  Serial.println(F("  L LED lit, no sound -> BUZ1 / J5 fault, not the keys."));
  Serial.println(F("  neither        -> D13 itself, or the Nano is not seated."));

  measureBaselines();

  Serial.println();
  Serial.println(F("Live. Touch the fruit exactly as you normally play - clip"));
  Serial.println(F("or no clip, whatever works on your breadboard. The probe"));
  Serial.println(F("does not care which direction the signal goes."));
  Serial.println(F("Columns: raw(delta). '-n' = pin dragged DOWN n counts,"));
  Serial.println(F("'+n' = pushed UP. '*' = moved >= 2 counts either way."));
  Serial.println();
}

void report() {
  Serial.print(F("keys:"));
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    int v = readKey(i);
    if (v < minSeen[i]) minSeen[i] = v;
    if (v > maxSeen[i]) maxSeen[i] = v;
    int delta = baseline[i] - v;       // positive = dragged DOWN = a touch
    Serial.print(' ');
    Serial.print(delta >= SHOW_TOUCH_AT ? '*' : ' ');
    Serial.print(v);
    Serial.print('(');
    if (delta >= 0) Serial.print('-');
    else            Serial.print('+');
    Serial.print(abs(delta));
    Serial.print(')');
  }

  // Question 2. Both should read "up" with nothing pressed.
  bool upPressed   = digitalRead(SENS_UP) == LOW;
  int  downRaw     = analogRead(SENS_DOWN);
  Serial.print(F("   SENS+:"));
  Serial.print(upPressed ? F("DOWN") : F("up  "));
  Serial.print(F(" SENS-:"));
  Serial.print(downRaw < 512 ? F("DOWN") : F("up  "));
  Serial.print('(');
  Serial.print(downRaw);
  Serial.print(')');
  Serial.println();
}

void loop() {
  static unsigned long lastReport = 0;
  static unsigned long lastSummary = 0;
  unsigned long now = millis();

  if (now - lastReport >= REPORT_MS) {
    lastReport = now;
    report();
  }

  // Every 10 s, the thing a scrolling log cannot show: how far each channel has
  // EVER moved, in BOTH directions. Three outcomes, and they are three
  // different faults:
  //   biggest movement DOWN  -> the board's polarity. The game can work; the
  //                             margin is the only thing to tune.
  //   biggest movement UP    -> the rig is the V4/V4.5 front end (pin floating,
  //                             series resistor, player on +5 V). The game
  //                             firmware and this board cannot see it at all,
  //                             and smart adjust will never find it either.
  //   nothing moves (<= 2)   -> the front end is dead: no touch signal exists
  //                             to threshold, whatever the margin says.
  if (now - lastSummary >= 10000UL) {
    lastSummary = now;
    int worstDown = 0, worstUp = 0;
    Serial.print(F("-- since boot   DOWN:"));
    for (uint8_t i = 0; i < KEY_COUNT; i++) {
      int d = baseline[i] - minSeen[i];
      if (d < 0) d = 0;
      if (d > worstDown) worstDown = d;
      Serial.print(' '); Serial.print(d);
    }
    Serial.print(F("   UP:"));
    for (uint8_t i = 0; i < KEY_COUNT; i++) {
      int u = maxSeen[i] - baseline[i];
      if (u < 0) u = 0;
      if (u > worstUp) worstUp = u;
      Serial.print(' '); Serial.print(u);
    }
    Serial.println();

    Serial.print(F("   verdict: "));
    if (worstDown <= 2 && worstUp <= 2) {
      Serial.println(F("NOTHING MOVES. No touch signal to threshold."));
    } else if (worstDown >= worstUp) {
      Serial.print(F("signal goes DOWN, best ")); Serial.print(worstDown);
      Serial.println(F(" counts - this board's polarity. Tune the margin."));
    } else {
      Serial.print(F("signal goes UP, best ")); Serial.print(worstUp);
      Serial.println(F(" counts - WRONG POLARITY for this board and firmware."));
      Serial.println(F("   The game looks DOWN only; so does smart adjust. This"));
      Serial.println(F("   rig is the V4/V4.5 front end, not V5's."));
    }
  }
}
