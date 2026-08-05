/* touch-noise — the V5.5 bench validation recipe, reduced to ONE channel so it
 * can run before the PCB exists. It answers the question V6 actually rests on:
 * is the keyboard's ADC quieter on battery than on mains?
 *
 * The whole V6 argument is that the V5.5 filter is a SERIES filter and so
 * cannot touch the common-mode path (the player's body is coupled to the mains
 * while the board is earthed through the charger). On battery the board floats
 * WITH the player and that difference cancels. This sketch measures it.
 *
 * One channel is enough: the V5 measurements showed all seven behave alike once
 * they are pulled up (idle 1022, noise 1 count on every channel).
 *
 * WIRING — three parts:
 *
 *   +5 V ──[220 Ω]──┬── A0
 *                   └── ~20 cm wire, or a clip to a lemon   (the "key")
 *   GND ── second clip, held in your hand                   (the player)
 *
 *   Power the Nano exactly as psu-probe does: module 5 V → Nano 5V pin,
 *   module GND → Nano GND. ⚠ Never the Nano's own USB at the same time.
 *
 * TWO WAYS TO READ IT, and the difference matters:
 *
 *   PC-FREE (the valid one). No serial cable at all. D13, the on-board LED,
 *   flashes once per NOISE EVENT — every sample that deviates from the boot
 *   baseline by more than EVENT_MARGIN counts, i.e. every sample the game would
 *   have read as a key press. Flip a light switch 20 times and count flashes.
 *   Attaching a PC ties its earthy ground to this circuit and re-creates the
 *   very path being measured, which is why the real run has no PC on it.
 *
 *   SERIAL (setup + sanity). 9600 baud through a USB-TTL adapter (TX/RX/GND
 *   only) prints the baseline, live readings and cumulative event count so you
 *   can confirm the rig works and that touching the key really does drag the
 *   pin down 3-4 counts. Use it to set up, then unplug it to measure.
 *
 * THE PROTOCOL — three runs, same room, same switch, same 20 flips:
 *   A) wall charger straight into the Nano's 5V pin   ← today's baseline
 *   B) battery + module                              ← should be the quietest
 *   C) battery + module while charging (micro-USB in) ← should be the worst
 * Record the flash count for each. Prediction: B < A, and C > B.
 *
 * Serial: 9600 baud, CSV.
 */
#include <Arduino.h>

static const uint8_t KEY_PIN   = A0;
static const uint8_t LED_PIN   = 13;

// 4 counts is the real thing: V5 measured a touch as a 3-4 count dip (skin
// ~1 MΩ against 220 Ω), and the firmware's auto margin floor is 4. So an
// "event" here is precisely what the game cannot distinguish from a finger.
static const uint16_t EVENT_MARGIN   = 4;

static const uint16_t SAMPLE_MS      = 50;    // 20 Hz, as the V5 bench sampler
static const uint16_t CAL_SAMPLES    = 200;   // ~10 s of quiet at boot
static const uint16_t FLASH_MS       = 25;    // long enough for the eye
static const uint16_t REPORT_EVERY   = 20;    // serial lines: 1 s apart

static uint16_t baseline    = 0;
static uint16_t calNoise    = 0;
static uint32_t events      = 0;
static uint16_t worstDev    = 0;
static uint16_t tick        = 0;

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println();
  Serial.println(F("# touch-noise — hands OFF the key while calibrating"));

  // Baseline = mean of a quiet window; calNoise = its peak-to-peak spread.
  // Same idea as the game's autoCalibrate(), so the numbers are comparable.
  uint32_t sum = 0;
  uint16_t lo = 1023, hi = 0;
  for (uint16_t i = 0; i < CAL_SAMPLES; i++) {
    const uint16_t r = analogRead(KEY_PIN);
    sum += r;
    if (r < lo) lo = r;
    if (r > hi) hi = r;
    delay(SAMPLE_MS);
  }
  baseline = (uint16_t)(sum / CAL_SAMPLES);
  calNoise = hi - lo;

  Serial.print(F("# baseline=")); Serial.print(baseline);
  Serial.print(F(" quiet_pp=")); Serial.print(calNoise);
  Serial.print(F(" event_margin=")); Serial.println(EVENT_MARGIN);
  if (calNoise >= EVENT_MARGIN) {
    Serial.println(F("# WARNING: idle noise already >= the touch margin."));
    Serial.println(F("#          Fix the rig before trusting any count."));
  }
  Serial.println(F("# ms,raw,dev,worst_dev,events"));

  // Three slow blinks: calibration done, counting from here.
  for (uint8_t i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH); delay(120);
    digitalWrite(LED_PIN, LOW);  delay(120);
  }
}

void loop() {
  const uint16_t raw = analogRead(KEY_PIN);

  // With a pull-up, anything real (a finger, or a transient) drags the pin
  // DOWN. Deviation upwards is not a key press, so only measure downwards.
  const uint16_t dev = (raw < baseline) ? (baseline - raw) : 0;
  if (dev > worstDev) worstDev = dev;

  if (dev >= EVENT_MARGIN) {
    events++;
    digitalWrite(LED_PIN, HIGH);
    delay(FLASH_MS);
    digitalWrite(LED_PIN, LOW);
    delay(SAMPLE_MS > FLASH_MS ? SAMPLE_MS - FLASH_MS : 0);
  } else {
    delay(SAMPLE_MS);
  }

  if (++tick >= REPORT_EVERY) {
    tick = 0;
    Serial.print(millis());   Serial.print(',');
    Serial.print(raw);        Serial.print(',');
    Serial.print(dev);        Serial.print(',');
    Serial.print(worstDev);   Serial.print(',');
    Serial.println(events);
  }
}
