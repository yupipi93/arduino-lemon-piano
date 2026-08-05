/* psu-probe — characterise a 5 V power source before it is allowed near the
 * piano. Built for V6's IP5356 + LiPo block, but it is source-agnostic.
 *
 * The Nano is the instrument, because it is the only voltmeter on the bench:
 *
 *   - RAIL: AVcc measured against the ATmega's internal 1.1 V bandgap. This is
 *     the same node the piano's ADC uses as its reference, so it is exactly the
 *     number that matters — and it needs no external parts.
 *   - CELL: the battery's B+ read on A1. A 1S LiPo tops out at 4.2 V, below
 *     AVcc, so it connects DIRECTLY — no divider, nothing to get wrong.
 *   - BOOTS: a counter in EEPROM. It survives losing power, so a source that
 *     HICCUPS (drops out and recovers — inrush restart, pass-through cycling)
 *     shows up as the count climbing while you watch. A source that LATCHES off
 *     (low-load auto-shutdown) instead shows up as the board going dark and
 *     staying dark, with UP resetting to 0 when you press the module's button.
 *   - MIN/MAX rail: latched extremes, so a dip you did not witness is still
 *     recorded.
 *
 * WIRING — read this twice, the first line can destroy the Nano:
 *
 *   ⚠ NEVER connect the Nano's own USB while feeding its 5V pin. Use a
 *     USB-TTL adapter for serial and connect ONLY TX / RX / GND — leave the
 *     adapter's VCC pin unconnected. On the real board the 1N5817 makes USB
 *     safe; on this breadboard there is no diode.
 *
 *   module 5 V out ──→ Nano 5V pin        (not VIN: no regulator in the way,
 *   module GND     ──→ Nano GND            same as the piano's J1 → 5V path)
 *   cell B+        ──→ Nano A1            (direct, ≤ 4.2 V)
 *   USB-TTL TX     ──→ Nano RX0 (D0)
 *   USB-TTL RX     ──→ Nano TX1 (D1)
 *   USB-TTL GND    ──→ Nano GND
 *
 * ⚠ The PC's ground now touches this circuit. That is fine for every test in
 *   this sketch (voltage, dropouts, load steps) but it re-introduces the
 *   mains common-mode path — so it would corrupt the noise comparison. That is
 *   what the sibling `touch-noise` sketch is for, and it runs PC-free.
 *
 * Serial: 9600 baud, one CSV line per 200 ms.
 */
#include <Arduino.h>
#include <EEPROM.h>

// ── calibration ──────────────────────────────────────────────────────────────
// Vcc = BANDGAP_SCALE / adc. 1125300 assumes the bandgap sits at exactly
// 1.1 V; the real part is 1.0-1.2 V, so an uncalibrated reading can be ~10 %
// out. To calibrate: put the KWS-X1 in line, read its volts, then set
//     BANDGAP_SCALE = 1125300 * (KWS_millivolts / RAIL_millivolts_printed)
// and reflash. Do this ONCE — it is a property of this particular chip.
static const int32_t BANDGAP_SCALE = 1125300L;

static const uint8_t  CELL_PIN      = A1;
static const uint16_t SAMPLE_MS     = 200;
static const int      EEPROM_BOOTS  = 0;   // one byte, wraps at 255

static uint16_t railMin = 0xFFFF;
static uint16_t railMax = 0;
static uint8_t  boots   = 0;

/* AVcc in millivolts, via the internal 1.1 V reference. */
static uint16_t readRailMillivolts() {
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // AVcc ref, 1V1 input
  delay(3);                                                // reference settling
  ADCSRA |= _BV(ADSC);
  while (ADCSRA & _BV(ADSC)) {}
  const uint16_t adc = ADC;
  if (adc == 0) return 0;
  return (uint16_t)(BANDGAP_SCALE / adc);
}

void setup() {
  Serial.begin(9600);

  boots = EEPROM.read(EEPROM_BOOTS);
  boots++;
  EEPROM.write(EEPROM_BOOTS, boots);

  Serial.println();
  Serial.println(F("# psu-probe — hold BOTH module button and patience"));
  Serial.print(F("# boots since last EEPROM reset: "));
  Serial.println(boots);
  Serial.println(F("# ms,rail_mV,cell_mV,rail_min_mV,rail_max_mV,boots"));
}

void loop() {
  const uint16_t rail = readRailMillivolts();
  if (rail) {
    if (rail < railMin) railMin = rail;
    if (rail > railMax) railMax = rail;
  }

  // First read after touching ADMUX is discarded: the mux has just moved off
  // the bandgap and the sample-and-hold still carries its charge.
  (void)analogRead(CELL_PIN);
  const uint16_t cellCounts = analogRead(CELL_PIN);
  const uint16_t cell = (uint16_t)(((uint32_t)cellCounts * rail) / 1023UL);

  Serial.print(millis());   Serial.print(',');
  Serial.print(rail);       Serial.print(',');
  Serial.print(cell);       Serial.print(',');
  Serial.print(railMin);    Serial.print(',');
  Serial.print(railMax);    Serial.print(',');
  Serial.println(boots);

  delay(SAMPLE_MS);
}
