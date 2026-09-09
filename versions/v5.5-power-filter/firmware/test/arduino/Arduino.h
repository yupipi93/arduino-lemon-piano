/* Arduino.h — a host stand-in for the AVR runtime, just big enough to run
 * src/main.cpp on a PC.
 *
 * WHY THIS EXISTS. The Velxio browser emulation cannot reach the mode wheel:
 * ten LEDs, a buzzer and key 7 already use every digital line, so there are no
 * pins left for the two buttons (see ../../emulation/README.md). And a piano
 * with a hidden menu is exactly the kind of thing that "compiles fine" and then
 * behaves wrongly in someone's hands. So the firmware gets driven end to end
 * HERE instead: real main.cpp, real melodies, real calibration, fake pins.
 *
 * It is a stand-in, not an emulator, and the difference matters when reading a
 * green result. Modelled: virtual time that advances the way the real chip's
 * does, seven analog touch channels with a baseline and noise, two buttons, ten
 * LEDs, and a log of every tone and every pin write. NOT modelled: interrupt
 * timing, Timer2, the ADC's actual behaviour, and anything electrical. It
 * proves the FIRMWARE'S LOGIC, and says nothing about the fruit.
 *
 * Time advances on delay(), delayMicroseconds(), and on every analogRead —
 * 112 us, the real conversion time — so a loop() that scans seven keys four
 * times over costs the ~3 ms it costs on the board, and gesture timings play
 * out at something close to their true rate rather than infinitely fast.
 */
#ifndef ARDUINO_SHIM_H
#define ARDUINO_SHIM_H

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>

// ── pins, in the Nano's numbering ───────────────────────────────────────────
#define A0 14
#define A1 15
#define A2 16
#define A3 17
#define A4 18
#define A5 19
#define A6 20
#define A7 21

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2

#define PROGMEM
#define pgm_read_word(addr) (*(const int *) (addr))
#define pgm_read_byte(addr) (*(const uint8_t *) (addr))

class __FlashStringHelper;
#define F(str) ((const __FlashStringHelper *) (str))

// ── the fake machine ────────────────────────────────────────────────────────
struct FakeBoard {
  static const int PIN_COUNT = 32;
  static const int KEY_COUNT_ = 7;
  static const int ADC_TIME_US = 112;     // one real analogRead

  uint64_t micros_ = 0;

  int baseline[KEY_COUNT_];               // what an untouched channel reads
  int touchDepth = 8;                     // how far a finger drags it down
  bool touched[KEY_COUNT_];
  uint32_t noiseSeed = 12345;
  int noiseSpan = 2;                      // peak-to-peak idle wobble, in counts

  bool plusDown = false;                  // D12, active low
  bool minusDown = false;                 // A7, active low via a 10k pull-up

  uint8_t pinState[PIN_COUNT];
  uint8_t pinMode_[PIN_COUNT];

  // logs
  struct ToneEvent { int freq; uint32_t atMs; };
  std::vector<ToneEvent> tones;           // every tone() that started
  std::string serial;
  bool traceSerial = false;

  FakeBoard() {
    for (int i = 0; i < KEY_COUNT_; i++) { baseline[i] = 1022; touched[i] = false; }
    for (int i = 0; i < PIN_COUNT; i++) { pinState[i] = LOW; pinMode_[i] = INPUT; }
  }

  uint32_t ms() const { return (uint32_t) (micros_ / 1000); }
  void advanceUs(uint64_t us) { micros_ += us; }

  int nextNoise() {                       // deterministic: the same run twice
    noiseSeed = noiseSeed * 1103515245u + 12345u;
    return (int) ((noiseSeed >> 16) % (uint32_t) (noiseSpan + 1));
  }
  int readChannel(int k) {
    advanceUs(ADC_TIME_US);
    int v = baseline[k] - nextNoise();
    if (touched[k]) v -= touchDepth;
    return v < 0 ? 0 : v;
  }
};

extern FakeBoard board;

// ── the API main.cpp actually uses ──────────────────────────────────────────
inline unsigned long millis() { return board.ms(); }
inline unsigned long micros() { return (unsigned long) board.micros_; }
inline void delay(unsigned long ms) { board.advanceUs((uint64_t) ms * 1000); }
inline void delayMicroseconds(unsigned int us) { board.advanceUs(us); }

inline void pinMode(uint8_t pin, uint8_t mode) {
  if (pin < FakeBoard::PIN_COUNT) board.pinMode_[pin] = mode;
}
inline void digitalWrite(uint8_t pin, uint8_t v) {
  if (pin < FakeBoard::PIN_COUNT) board.pinState[pin] = v;
}
inline int digitalRead(uint8_t pin) {
  if (pin == 12) return board.plusDown ? LOW : HIGH;   // SENS_UP, to GND
  return board.pinState[pin];
}
inline int analogRead(uint8_t pin) {
  if (pin < FakeBoard::KEY_COUNT_) return board.readChannel(pin);
  if (pin == A7) { board.advanceUs(FakeBoard::ADC_TIME_US);
                   return board.minusDown ? 0 : 1023; }
  board.advanceUs(FakeBoard::ADC_TIME_US);
  return 0;
}
inline void tone(uint8_t, unsigned int freq) {
  FakeBoard::ToneEvent e = { (int) freq, board.ms() };
  board.tones.push_back(e);
}
inline void noTone(uint8_t) {}

// ── Serial ──────────────────────────────────────────────────────────────────
struct FakeSerial {
  void begin(long) {}
  void put(const std::string &s) {
    board.serial += s;
    if (board.traceSerial) fputs(s.c_str(), stdout);
  }
  void print(const char *s) { put(s); }
  void print(const __FlashStringHelper *s) { put((const char *) s); }
  void print(char c) { put(std::string(1, c)); }
  void print(int v) { put(std::to_string(v)); }
  void print(long v) { put(std::to_string(v)); }
  void print(unsigned v) { put(std::to_string(v)); }
  void print(unsigned long v) { put(std::to_string(v)); }
  void println() { put("\n"); }
  template <typename T> void println(T v) { print(v); put("\n"); }
};
extern FakeSerial Serial;

#endif  // ARDUINO_SHIM_H
