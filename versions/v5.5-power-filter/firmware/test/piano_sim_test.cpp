/* piano_sim_test.cpp — the whole firmware, driven end to end on the host.
 *
 * ui_gestures_test.cpp proves the BUTTONS. This proves the PIANO: it compiles
 * the real src/main.cpp against the fake board in arduino/Arduino.h, boots it,
 * and then plays it — touching lemons, holding buttons, turning the wheel — and
 * asserts what came out of the buzzer and the LED bar.
 *
 * What it can prove: the note mapping, that free play repeats a key where the
 * game refuses to, that the wheel reaches every mode and wraps, that cancelling
 * changes nothing, that winning never lands on free play, that opening the menu
 * hands the sensitivity back. What it CANNOT prove: anything electrical — see
 * the honesty note at the top of arduino/Arduino.h. A green run here means the
 * firmware is worth flashing, not that the fruit will behave.
 *
 * Build & run:  versions/v5.5-power-filter/firmware/test/run.sh
 */
#include "arduino/Arduino.h"

FakeBoard board;
FakeSerial Serial;

#include "../src/main.cpp"   // the firmware under test, verbatim

#include <string>
#include <vector>

// LED_PINS lives in the firmware, so the fake board's frame recorder can only
// be defined once main.cpp has been included. One frame per change to the bar.
void FakeBoard::noteLedFrame(uint8_t pin) {
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    if (LED_PINS[i] != pin) continue;
    std::string f;
    for (uint8_t j = 0; j < LED_COUNT; j++) f += pinState[LED_PINS[j]] ? '#' : '.';
    pushLedFrame(f);
    return;
  }
}

// ── the rig ─────────────────────────────────────────────────────────────────
static int failures = 0, checks = 0;

static void ok(bool cond, const std::string &what, const std::string &detail = "") {
  checks++;
  if (cond) printf("  \033[32mPASS\033[0m  %s\n", what.c_str());
  else { failures++; printf("  \033[31mFAIL\033[0m  %s\n        %s\n",
                            what.c_str(), detail.c_str()); }
}
static void eqInt(long got, long want, const std::string &what) {
  ok(got == want, what, "expected " + std::to_string(want) + ", got " + std::to_string(got));
}
static void section(const char *n) { printf("\n\033[1m%s\033[0m\n", n); }

// Run loop() until `ms` of virtual time has gone by.
static void runFor(uint32_t ms) {
  const uint64_t until = board.micros_ + (uint64_t) ms * 1000;
  while (board.micros_ < until) loop();
}
static size_t toneMark() { return board.tones.size(); }
static std::vector<int> tonesSince(size_t mark) {
  std::vector<int> out;
  for (size_t i = mark; i < board.tones.size(); i++) out.push_back(board.tones[i].freq);
  return out;
}
static size_t frameMark() { return board.ledFrames.size(); }
static std::vector<std::string> framesSince(size_t mark) {
  std::vector<std::string> out;
  for (size_t i = mark; i < board.ledFrames.size(); i++) {
    if (out.empty() || out.back() != board.ledFrames[i]) out.push_back(board.ledFrames[i]);
  }
  return out;
}
// Did the bar ever wear this exact shape during the film?
static bool filmHas(const std::vector<std::string> &f, const std::string &shape) {
  for (size_t i = 0; i < f.size(); i++) if (f[i] == shape) return true;
  return false;
}
// The last "theme: f f f ..." line the firmware printed, as frequencies. A theme
// is bit-banged (buzz(), not tone()), so this log line is the only way to see
// WHICH notes an announcement played — and it is the clue, so which notes it
// plays is exactly the thing worth pinning.
static std::vector<int> lastThemeLine() {
  std::vector<int> out;
  size_t at = board.serial.rfind("theme:");
  if (at == std::string::npos) return out;
  size_t end = board.serial.find('\n', at);
  std::string line = board.serial.substr(at + 6, end - at - 6);
  size_t i = 0;
  while (i < line.size()) {
    while (i < line.size() && line[i] == ' ') i++;
    size_t j = i;
    while (j < line.size() && line[j] >= '0' && line[j] <= '9') j++;
    if (j > i) out.push_back(atoi(line.substr(i, j - i).c_str()));
    i = j + 1;
  }
  return out;
}
static size_t serialMark() { return board.serial.size(); }
static bool serialSince(size_t mark, const char *needle) {
  return board.serial.find(needle, mark) != std::string::npos;
}
static int litLeds() {
  int n = 0;
  for (uint8_t i = 0; i < LED_COUNT; i++) if (board.pinState[LED_PINS[i]]) n++;
  return n;
}
static std::string ledPattern() {
  std::string s;
  for (uint8_t i = 0; i < LED_COUNT; i++) s += board.pinState[LED_PINS[i]] ? '#' : '.';
  return s;
}

static void touchKey(int key, uint32_t holdMs = 130, uint32_t gapMs = 160) {
  board.touched[key] = true;  runFor(holdMs);
  board.touched[key] = false; runFor(gapMs);
}
static void tapPlus(uint32_t hold = 90, uint32_t gap = 160) {
  board.plusDown = true;  runFor(hold);
  board.plusDown = false; runFor(gap);
}
static void tapMinus(uint32_t hold = 90, uint32_t gap = 160) {
  board.minusDown = true;  runFor(hold);
  board.minusDown = false; runFor(gap);
}
// The documented way into free play since 2026-09-13: hold + for three seconds.
// It is a switch, not a menu entry, so it also takes you back out.
static void toggleFreePlayByHold() {
  board.plusDown = true;  runFor(3400);
  board.plusDown = false; runFor(4000);   // the accept cue + the mode's own intro
}
// The documented way into the level wheel: hold − for three seconds.
static void openMenu() {
  board.minusDown = true;  runFor(3400);
  board.minusDown = false; runFor(300);
}
// ...and the documented way to say yes: both buttons at once.
static void acceptSelection() {
  board.plusDown = true;  runFor(60);
  board.minusDown = true; runFor(200);
  board.plusDown = false; board.minusDown = false;
  runFor(4000);            // the accept cue + the mode's own intro melody
}

// Boot the firmware into a known state before each scenario.
static void boot() {
  board = FakeBoard();
  Serial = FakeSerial();
  level = 1;
  setup();
  runFor(200);
}

// ── the scenarios ───────────────────────────────────────────────────────────

static void test_boot_unchanged() {
  section("1. It still boots into level 1 the way it always did");
  boot();
  ok(board.serial.find("Lemon Piano V5.5") != std::string::npos, "the banner is there");
  ok(board.serial.find("Level 1") != std::string::npos, "it announces Level 1");
  ok(board.serial.find("auto margin=") != std::string::npos, "auto-calibration ran");
  ok(touchMargin >= AUTO_MARGIN_MIN, "  ...and set a usable margin",
     "margin = " + std::to_string(touchMargin));
  eqInt(level, 1, "  ...and the mode is level 1, not the wheel");
}

static void test_game_still_scores() {
  section("2. The game is untouched: the level 1 code still scores");
  boot();
  size_t sm = serialMark();
  touchKey(5);                                    // key 6 = level 1's first note
  ok(serialSince(sm, "OK 1/10"), "the first note of the code lights LED 1");
  eqInt(board.pinState[LED_PINS[0]], HIGH, "  ...and it really is LED 1");
  sm = serialMark();
  size_t tm2 = toneMark();
  touchKey(5);                                    // the SAME key again
  ok(!serialSince(sm, "OK 2/10"), "the same lemon twice does NOT score twice (the game's repeat filter)");
  ok(!tonesSince(tm2).empty(), "  ...but it DOES sound: a piano before it is a game");
  sm = serialMark();
  touchKey(0);                                    // key 1: wrong note
  ok(serialSince(sm, "WRONG"), "a wrong note is punished");
  eqInt(litLeds(), 0, "  ...and blanks the bar");
}

static void test_menu_opens_and_previews() {
  section("3. Holding − for 3 s opens the wheel, and it says so");
  boot();
  size_t sm = serialMark(), tm = toneMark();
  board.minusDown = true;
  runFor(900);
  ok(!serialSince(sm, "MODE MENU"), "at 900 ms it is still the sensitivity knob");
  runFor(2600);
  ok(serialSince(sm, "keep holding for the level wheel"), "  ...it warns before it opens");
  ok(serialSince(sm, "MODE MENU"), "  ...and at 3 s the wheel is open");
  ok(tonesSince(tm).size() > 3, "  ...with sound to match",
     std::to_string(tonesSince(tm).size()) + " tones");
  ok(serialSince(sm, "> Level 1"), "  ...opened ON the mode being played");
  board.minusDown = false; runFor(300);
}

static void test_wheel_reaches_every_level_and_wraps() {
  section("4. The wheel turns to every LEVEL, and wraps (free play left it)");
  boot();
  openMenu();
  size_t sm = serialMark();
  tapPlus(); runFor(2600);  ok(serialSince(sm, "> Level 2"), "+ -> Level 2");
  sm = serialMark();
  tapPlus(); runFor(3000);  ok(serialSince(sm, "> Level 3"), "+ -> Level 3");
  sm = serialMark();
  tapPlus(); runFor(4000);  ok(serialSince(sm, "> Level 4"), "+ -> Level 4");
  sm = serialMark();
  tapPlus(); runFor(2600);  ok(serialSince(sm, "> Level 1"), "+ WRAPS round to Level 1");
  sm = serialMark();
  tapMinus(); runFor(4000); ok(serialSince(sm, "> Level 4"), "− WRAPS the other way");
  ok(!board.serial.find("FREE PLAY (the piano)") ||
     board.serial.find("> FREE PLAY") == std::string::npos,
     "  ...and FREE PLAY is not on the wheel at all");
  eqInt(level, 1, "and nothing has been accepted yet: still playing level 1");
}

static void test_free_play_mapping() {
  section("5. FREE PLAY: the seven lemons are do re mi fa sol la si");
  boot();
  toggleFreePlayByHold();
  eqInt(level, FREE_PLAY, "holding + handed over the instrument");
  ok(board.serial.find("do re mi fa sol la si") != std::string::npos,
     "  ...and the log says what it is");

  const int expect[7] = {NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5, NOTE_A5, NOTE_B5};
  const char *name[7] = {"do", "re", "mi", "fa", "sol", "la", "si"};
  for (int k = 0; k < 7; k++) {
    size_t tm = toneMark();
    touchKey(k);
    std::vector<int> t = tonesSince(tm);
    ok(!t.empty() && t[0] == expect[k],
       std::string("key ") + std::to_string(k + 1) + " = " + name[k] + " (" +
       std::to_string(expect[k]) + " Hz)",
       t.empty() ? "no tone at all" : "got " + std::to_string(t[0]) + " Hz");
  }
}

static void test_free_play_repeats_and_never_scores() {
  section("6. FREE PLAY: the same lemon, over and over — and never a verdict");
  boot();
  toggleFreePlayByHold();

  size_t sm = serialMark(), tm = toneMark();
  for (int i = 0; i < 5; i++) touchKey(2);          // the SAME key, five times
  std::vector<int> t = tonesSince(tm);
  int mi = 0;
  for (size_t i = 0; i < t.size(); i++) if (t[i] == NOTE_E5) mi++;
  eqInt(mi, 5, "five taps on one lemon = five notes (the game would give one)");
  ok(!serialSince(sm, "OK "), "  ...and nothing was scored");
  ok(!serialSince(sm, "WRONG"), "  ...and nothing was punished");
  ok(!serialSince(sm, "STUCK"), "  ...and it never complains about a locked key");

  // ...and the code of level 1, played in full, wins nothing here.
  sm = serialMark();
  const int code[10] = {5, 4, 5, 6, 1, 4, 1, 0, 2, 3};
  for (int i = 0; i < 10; i++) touchKey(code[i]);
  ok(!serialSince(sm, "WIN"), "  ...and level 1's whole code wins nothing: there is no game");
  eqInt(level, FREE_PLAY, "  ...it is still the piano");
}

// The idle bar used to be "#........#", the two ends lit, and that was the mark
// of the mode. Sergio killed it on 2026-09-15: it was still sitting there under
// every note the player pressed, inside the pitch reading, looking like part of
// it. Free play idles DARK now, so every lit LED was lit by a finger.
// The bar in free play has NO standing state at all as of 2026-09-15: it idles
// dark, a note draws a wave and then dark again. Sergio removed the pitch meter
// that used to sit under the wave — "quita el estático... están solapados" —
// so the only thing the bar ever does here is move. What each key's wave looks
// like is test 24's job; this one pins that nothing stays behind.
static void test_free_play_bar_never_stands_still() {
  section("7. FREE PLAY: the bar only ever MOVES — nothing is left standing");
  boot();
  toggleFreePlayByHold();
  ok(ledPattern() == "..........", "idle: nothing is lit at all", ledPattern());

  size_t tm = toneMark();
  board.touched[0] = true; runFor(500);
  ok(ledPattern() == "..........",
     "the lowest lemon: the wave passes and leaves the bar dark", ledPattern());
  ok(!tonesSince(tm).empty(), "  ...while its note is still sounding");
  board.touched[0] = false; runFor(300);

  tm = toneMark();
  board.touched[6] = true; runFor(500);
  ok(ledPattern() == "..........",
     "the highest lemon: same, no block of light left behind", ledPattern());
  ok(!tonesSince(tm).empty(), "  ...and it too is still sounding");
  board.touched[6] = false; runFor(300);
  ok(ledPattern() == "..........", "  ...and it is dark once let go", ledPattern());
}

static void test_cancel_changes_nothing() {
  section("8. Leaving the wheel by a long press changes NOTHING");
  boot();
  touchKey(5);                                   // one correct note: LED 1 lit
  eqInt(litLeds(), 1, "the game is one note in");
  openMenu();
  tapPlus(); runFor(2600);                       // move the wheel to Level 2
  size_t sm = serialMark();
  board.plusDown = true; runFor(3400);           // long press = escape
  board.plusDown = false; runFor(600);
  ok(serialSince(sm, "menu closed - nothing changed"), "the long press left the menu");
  eqInt(level, 1, "  ...the level did not change to the one being previewed");
  eqInt(litLeds(), 1, "  ...and the progress bar is exactly where it was");
  sm = serialMark();
  touchKey(4);                                   // key 5 = the code's 2nd note
  ok(serialSince(sm, "OK 2/10"), "  ...the game carries on from note 2");
}

static void test_accept_restarts_the_chosen_level() {
  section("9. Accepting a level starts it clean");
  boot();
  touchKey(5);                                   // one note into level 1
  openMenu();
  tapPlus(); runFor(2600);                       // Level 2
  acceptSelection();
  eqInt(level, 2, "level 2 was accepted");
  eqInt(litLeds(), 0, "  ...with an empty bar: a chosen level starts from zero");
  size_t sm = serialMark();
  touchKey(2);                                   // key 3 = level 2's first note
  ok(serialSince(sm, "OK 1/10"), "  ...and level 2's own code is what scores now");
}

static void test_opening_the_menu_gives_the_knob_back() {
  section("10. Opening the wheel does not leave the sensitivity somewhere else");
  boot();
  const int before = touchMargin;
  board.minusDown = true;
  runFor(950);                                   // the first second DOES ramp
  const int ramped = touchMargin;
  ok(ramped > before, "holding − ramps the knob for the first second",
     std::to_string(before) + " -> " + std::to_string(ramped));
  runFor(2600);                                  // ...and then opens the menu
  board.minusDown = false; runFor(300);
  ok(gestures.inMenu(), "the menu opened");
  eqInt(touchMargin, before, "  ...and put the margin back where it found it");
}

static void test_winning_never_lands_on_free_play() {
  section("11. Winning cycles the levels and never falls into free play");
  boot();
  const int code[10] = {5, 4, 5, 6, 1, 4, 1, 0, 2, 3};   // level 1, as key indices
  size_t sm = serialMark();
  for (int i = 0; i < 10; i++) touchKey(code[i]);
  ok(serialSince(sm, "WIN"), "level 1's code wins");
  runFor(60000);                                  // theme + fanfare + next intro
  eqInt(level, 2, "  ...and it advances to level 2, not to the wheel's 5th item");
  ok(level != FREE_PLAY, "  ...free play is chosen, never earned");
}

static void test_open_and_accept_is_a_no_op() {
  section("12. Open the wheel, accept immediately: nothing moved");
  boot();
  openMenu();
  acceptSelection();
  eqInt(level, 1, "it opened on level 1 and accepted level 1");
}

// ── 13. A held key does not smear across its neighbours (2026-09-13) ────────
// POSITIVE CONTROL for the high-impedance keyboard. At 1 MOhm a conversion
// closes only 57 % of the gap to the real level, so the first reads on a newly
// selected channel mostly carry the PREVIOUS channel's. readKey() must keep
// converting until the channel stops moving; if it does not, a single key held
// down smears across the scan and the wrong key -- or no key -- is seen.
//
// This section guards the FIRST step only: cut readKey() down to a single
// conversion with no discard at all and it fails. One discard is enough for a
// 200-count touch between two idle neighbours, and is NOT enough for the 1023
// count drop the SENS - button makes -- that is test 14's job, and the two
// together are what pin the settling loop. The other 56 checks pass either way,
// because a 220 Ohm front end settles instantly and cannot show any of it.
static void test_first_conversion_is_discarded() {
  section("13. A held key does not smear across its neighbours");

  board = FakeBoard();
  board.adcSettle = FakeBoard::SETTLE_1M;  // model a 1 MOhm keyboard
  board.touchDepth = 200;           // 1 MOhm pull-up territory, not 4 counts
  setup();

  ok(touchMargin >= 4, "it still calibrates with residue present",
     "margin=" + std::to_string(touchMargin));

  // Every channel idle: a residue-contaminated read would still be ~baseline,
  // so this only checks nothing drifted into a false press.
  runFor(300);
  eqInt(strongestKey(), -1, "nothing is pressed while every channel is idle");

  // Hold key 5 (index 4) and scan. Its neighbours are idle, so an undiscarded
  // first conversion would hand key 4's or key 6's level to key 5 and back.
  board.touched[4] = true;
  for (int i = 0; i < 20; i++) runFor(10);
  eqInt(strongestKey(), 4, "the held key is the one seen, not its neighbour");

  int held = readKey(4);
  ok(held <= 1023 - 150,
     "the held channel reads its own settled level, not the previous channel's",
     "readKey(4)=" + std::to_string(held));

  int neighbour = readKey(5);
  ok(neighbour >= 1023 - 30,
     "an idle neighbour is not dragged down by the held channel",
     "readKey(5)=" + std::to_string(neighbour));

  board.touched[4] = false;
  for (int i = 0; i < 20; i++) runFor(10);
  eqInt(strongestKey(), -1, "and it releases cleanly");
}


// ── 14. The SENS - button must not press a key (2026-09-13) ─────────────────
// REGRESSION for what the real v0.7.1 board did the day its pull-ups went to
// 1 MOhm: pressing the - button played a note, and in the game that note was
// scored as a wrong guess. Reported by Sergio, 2026-09-13.
//
// The cause is entirely inside the chip. SENS - is A7, which is an ADC channel,
// on the SAME multiplexer as the seven keys, and serviceButtons() reads it at
// the top of every loop() -- immediately before the key scan starts at A0.
// While the button is held A7 sits at 0 V, so the sample-and-hold cap is handed
// to key 1 holding zero. Through 220 Ohm that was refilled before the first
// conversion finished; through 1 MOhm it is not, and key 1 reads far enough
// below its baseline to look touched.
//
// Which is also why it stopped happening once the margin was raised past the
// middle of the LED bar: the droop is finite, so a wide enough margin hides it.
// This test pins the fix instead of the workaround -- no phantom at margin 4.
static void test_minus_button_does_not_press_a_key() {
  section("14. Holding SENS - does not press a key");

  board = FakeBoard();
  board.adcSettle = FakeBoard::SETTLE_1M;   // the 1 MOhm keyboard he now has
  setup();

  eqInt(touchMargin, 4, "it calibrates to the tight margin the quiet board earns");
  runFor(300);
  eqInt(strongestKey(), -1, "nothing is pressed with both buttons up");

  // Exactly what loop() does: read A7 with the button down, then scan the keys.
  board.minusDown = true;
  analogRead(SENS_DOWN);
  int first = readKey(0);
  ok(first >= thresholdFor(0),
     "key 1 read straight after A7 is still above its threshold",
     "readKey(0)=" + std::to_string(first) +
     " threshold=" + std::to_string(thresholdFor(0)));
  analogRead(SENS_DOWN);        // poison it again: strongestKey() is what loop()
                                // actually calls first, straight after the button
  eqInt(strongestKey(), -1, "and no key is seen as pressed");

  // End to end: hold - for a full nudge and make sure the buzzer only ever
  // says "sensitivity", never a note from the keyboard.
  board = FakeBoard();
  board.adcSettle = FakeBoard::SETTLE_1M;
  setup();
  runFor(300);
  size_t mark = toneMark();
  for (int i = 0; i < 6; i++) tapMinus();
  std::vector<int> heard = tonesSince(mark);
  bool anyKeyNote = false;
  for (size_t i = 0; i < heard.size(); i++)
    for (uint8_t k = 0; k < KEY_COUNT; k++)
      if (heard[i] == keys[k + (level - 1) * KEY_COUNT]) anyKeyNote = true;
  ok(!anyKeyNote, "six taps on - produce no key note at all",
     std::to_string(heard.size()) + " tones heard, one of them a key note");

  // And the margin really did move, so the taps were not simply ignored.
  ok(touchMargin > 4, "the taps did reach the sensitivity knob",
     "margin=" + std::to_string(touchMargin));
}


// ── 15. Free play is a SWITCH on +, not an item on the wheel (2026-09-13) ───
// Sergio: "mantener el botón más es un switch entre el modo libre y el nivel".
// So it has to go both ways, and coming back has to land where it left.
static void test_free_play_is_a_switch_on_plus() {
  section("15. Holding + switches between free play and the level");

  boot();
  eqInt(level, 1, "it boots into level 1");
  toggleFreePlayByHold();
  eqInt(level, FREE_PLAY, "hold + -> free play");
  toggleFreePlayByHold();
  eqInt(level, 1, "hold + again -> back to the level, which was 1");

  // ...and it remembers a level that is NOT 1, which is the whole reason the
  // switch keeps a snapshot instead of hard-coding level 1.
  boot();
  openMenu();
  tapPlus(); runFor(2600);            // -> Level 2
  tapPlus(); runFor(3000);            // -> Level 3
  acceptSelection();
  eqInt(level, 3, "the wheel put us on level 3");
  toggleFreePlayByHold();
  eqInt(level, FREE_PLAY, "  ...hold + -> free play");
  toggleFreePlayByHold();
  eqInt(level, 3, "  ...and hold + again comes back to LEVEL 3, not level 1");

  // The − wheel is still a way out of free play, and it opens on a real level.
  toggleFreePlayByHold();
  eqInt(level, FREE_PLAY, "back into free play");
  size_t sm = serialMark();
  openMenu();
  ok(serialSince(sm, "> Level 3"),
     "  ...opening the wheel from free play lands on the level it interrupted");
  acceptSelection();
  eqInt(level, 3, "  ...and accepting it leaves free play");
}

// ── 16. A HELD lemon sounds ONCE (2026-09-13) ──────────────────────────────
// REGRESSION for what Sergio reported the day the keyboard went to 1 MOhm: in
// free play, resting a finger on a lemon machine-gunned the note. The contact
// is not a switch — it breaks for a few ms at a time without the finger moving,
// and every one of those used to end the note and start a new one.
//
// Delete the RELEASE_CONFIRM_MS guard in loop() and this section fails.
static void test_held_fruit_sounds_once() {
  section("16. Holding a lemon is ONE note, letting go and touching again is two");

  boot();
  toggleFreePlayByHold();
  board.dropoutEveryMs = 200;         // flaky contact: 30 ms clear every 200 ms
  board.dropoutMs = 30;

  size_t tm = toneMark();
  board.touched[2] = true;  runFor(2000);   // ten dropouts' worth of holding
  board.touched[2] = false; runFor(400);
  int mi = 0;
  std::vector<int> t = tonesSince(tm);
  for (size_t i = 0; i < t.size(); i++) if (t[i] == NOTE_E5) mi++;
  eqInt(mi, 1, "two seconds of holding one lemon = ONE note");

  // ...and a real release still lets it sound again.
  tm = toneMark();
  board.touched[2] = true;  runFor(300);
  board.touched[2] = false; runFor(400);
  t = tonesSince(tm);
  mi = 0;
  for (size_t i = 0; i < t.size(); i++) if (t[i] == NOTE_E5) mi++;
  eqInt(mi, 1, "  ...and touching it again after letting go sounds it again");

  // Fast deliberate playing must not be swallowed by the guard.
  tm = toneMark();
  for (int i = 0; i < 4; i++) touchKey(2, 120, 200);
  t = tonesSince(tm);
  mi = 0;
  for (size_t i = 0; i < t.size(); i++) if (t[i] == NOTE_E5) mi++;
  eqInt(mi, 4, "  ...and four deliberate taps are still four notes");

  board.dropoutEveryMs = 0;
}

// ── 17. The bar counts SENSITIVITY, not margin (2026-09-13) ────────────────
// Sergio: + put lights OUT and − turned them ON. A button labelled "more" that
// takes light away reads as "less" from across the room.
static void test_bar_counts_sensitivity() {
  section("17. More LEDs = more sensitive, so + adds light");

  boot();
  touchMargin = 2;  showMarginOnBar();
  const int litSensitive = litLeds();
  touchMargin = 18; showMarginOnBar();
  const int litBlunt = litLeds();
  ok(litSensitive > litBlunt, "a tight margin lights MORE LEDs than a wide one",
     "margin 2 -> " + std::to_string(litSensitive) +
     " LEDs, margin 18 -> " + std::to_string(litBlunt));
  eqInt(litBlunt, 1, "  ...and the bluntest end still shows one, never zero");

  // End to end, through the buttons.
  boot();
  touchMargin = 10;
  tapMinus(); runFor(50);
  const int afterMinus = litLeds();
  const int marginAfterMinus = touchMargin;
  tapPlus(); runFor(50);
  tapPlus(); runFor(50);
  tapPlus(); runFor(50);
  tapPlus(); runFor(50);
  ok(touchMargin < marginAfterMinus, "+ makes it more sensitive",
     "margin " + std::to_string(marginAfterMinus) + " -> " + std::to_string(touchMargin));
  ok(litLeds() > afterMinus, "  ...and that LIGHTS MORE LEDs, not fewer",
     std::to_string(afterMinus) + " -> " + std::to_string(litLeds()));
}


// ── 18. A repeated lemon sounds, costs nothing, and SHOWS that (2026-09-13) ─
// Sergio: the locked-key rattle read as a mistake, and pressing the same lemon
// twice is not a mistake -- it is just not a move. So the repeat now (a) sounds
// its note like any other press, (b) scores nothing and costs nothing, and
// (c) says so with the backwards sweep instead of a scolding noise.
//
// The bar is what carries the message, so the bar is what this pins: the score
// is IDENTICAL before and after, which is exactly what a wrong note is not.
static void test_repeat_sounds_but_scores_nothing() {
  section("18. A repeated lemon sounds, scores nothing, and costs nothing");

  boot();
  const int code[10] = {5, 4, 5, 6, 1, 4, 1, 0, 2, 3};
  touchKey(code[0]);
  touchKey(code[1]);
  eqInt(litLeds(), 2, "two correct notes, two LEDs");
  const std::string before = ledPattern();

  // The same lemon again: it is the note the game just accepted.
  size_t sm = serialMark(), tm = toneMark();
  touchKey(code[1]);
  std::vector<int> heard = tonesSince(tm);

  ok(!heard.empty(), "the repeat SOUNDS",
     std::to_string(heard.size()) + " tones");
  ok(!heard.empty() && heard[0] == keys[code[1] + (level - 1) * KEY_COUNT],
     "  ...and it is that lemon's own note, at full voice",
     heard.empty() ? "silence" : "got " + std::to_string(heard[0]));
  ok(!serialSince(sm, "WRONG"), "  ...the game does NOT punish it");
  ok(!serialSince(sm, "OK 3/10"), "  ...and does NOT score it");
  eqInt(currentStep, 2, "  ...the progress is untouched");
  ok(ledPattern() == before, "  ...and the bar ends exactly where it started",
     before + " -> " + ledPattern());
  ok(serialSince(sm, "scores nothing"), "  ...and it says so in the log");

  // The old scolding noise is gone: the ONLY sound was the note itself.
  eqInt((long) heard.size(), 1, "  ...and the locked-key rattle is gone: one sound, the note");

  // ...and the cue is the WHOLE BAR, lit for exactly as long as the lemon is
  // held (2026-09-15 — it was a backwards sweep between 09-13 and today, and
  // the motion moved to free play's wave). The bar starts and ends on the same
  // score, so the only proof the player was told anything is what it does in
  // between, while the finger is still down.
  boot();
  touchKey(code[0]);
  touchKey(code[1]);
  board.touched[code[1]] = true; runFor(300);        // ...and HOLD the repeat
  ok(ledPattern() == "##########",
     "  ...and the cue is every LED at once, while the lemon is held",
     ledPattern());
  board.touched[code[1]] = false; runFor(400);
  ok(ledPattern() == before,
     "  ...and letting go puts the score straight back",
     before + " -> " + ledPattern());

  // ...and the game carries on normally from there: the NEXT correct lemon
  // scores, so a repeat really did cost nothing.
  sm = serialMark();
  touchKey(code[2]);
  ok(serialSince(sm, "OK 3/10"), "the next correct lemon still scores");
  eqInt(litLeds(), 3, "  ...and the bar moves on to three");

  // A WRONG note, by contrast, still blanks the bar and LEAVES it blank --
  // which is what makes the two impossible to confuse.
  sm = serialMark();
  touchKey(0);                        // step 3 wants key 7 (code[3] = 6), not key 1
  ok(serialSince(sm, "WRONG"), "a wrong lemon is still punished");
  eqInt(litLeds(), 0, "  ...and its bar stays blank, unlike a repeat's");
}

// ── 19 ──────────────────────────────────────────────────────────────────────
// The mode no longer marks itself with a standing shape, so it has to mark
// itself with an EVENT. The sweep is that event, and this pins the three beats
// of it plus the thing it exists to guarantee: that it ends on a dark bar.
static void test_free_play_opens_with_a_sweep() {
  section("19. FREE PLAY opens with a sweep, and hands over a DARK bar");
  boot();
  size_t fm = frameMark();
  toggleFreePlayByHold();
  std::vector<std::string> film = framesSince(fm);

  ok(filmHas(film, "#........#"),
     "the two ends light up first - the old idle shape, as a frame this time");
  ok(filmHas(film, "....##...."),
     "  ...the two lights walk in and meet in the middle");
  ok(filmHas(film, "##########"),
     "  ...then open back out until the whole bar is lit");
  ok(ledPattern() == "..........",
     "  ...and when the scale has finished, the bar is dark", ledPattern());

  // ...and the melody still plays. The sweep is additive: it was never meant to
  // replace the scale that says what the seven lemons are.
  const int scale[7] = {NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5, NOTE_A5, NOTE_B5};
  size_t tm = toneMark();
  boot();
  tm = toneMark();
  toggleFreePlayByHold();
  std::vector<int> t = tonesSince(tm);
  int at = 0;
  for (size_t i = 0; i < t.size() && at < 7; i++) if (t[i] == scale[at]) at++;
  eqInt(at, 7, "  ...and do re mi fa sol la si still plays on the way in");
}

// ── 20 ──────────────────────────────────────────────────────────────────────
// Sergio, 2026-09-15: "cuando el usuario toque dos notas a la vez... toque la
// nota correspondiente a ambas notas". One buzzer, two notes: they take turns
// fast enough to fuse. These run with couplingDepth SET, because a chord rule
// that only works on an idealised board is not a chord rule.
static void test_two_lemons_sound_as_two_notes() {
  section("20. FREE PLAY: two lemons at once sound as two notes");
  boot();
  board.touchDepth = 12;
  board.couplingDepth = 5;          // one finger already shadows the others
  toggleFreePlayByHold();

  size_t sm = serialMark(), tm = toneMark();
  board.touched[0] = true;  runFor(80);      // do
  board.touched[4] = true;  runFor(400);     // ...and sol laid on top of it
  std::vector<int> t = tonesSince(tm);
  int lo = 0, hi = 0;
  for (size_t i = 0; i < t.size(); i++) {
    if (t[i] == NOTE_C5) lo++;
    if (t[i] == NOTE_G5) hi++;
  }
  ok(lo >= 5 && hi >= 5, "both notes take turns on the one buzzer",
     "C5 x" + std::to_string(lo) + ", G5 x" + std::to_string(hi));
  ok(serialSince(sm, "~~ chord 1+5"), "  ...and the log names both lemons");
  ok(ledPattern() == "#.....#...",
     "  ...and the bar shows TWO lone LEDs, one per note", ledPattern());

  // Lift the upper voice: what is left is the note that is left. The chord goes
  // on alternating through the release-confirm window (a resting finger reads
  // clear for a scan or two without moving), so what is pinned here is where it
  // LANDS: the last thing the buzzer was told, and then silence from the
  // firmware — the surviving note sustains instead of being re-struck.
  sm = serialMark(); tm = toneMark();
  board.touched[4] = false; runFor(300);
  t = tonesSince(tm);
  ok(!t.empty() && t.back() == NOTE_C5,
     "letting the upper lemon go leaves the lower one sounding alone",
     t.empty() ? "no tone" : "ended on " + std::to_string(t.back()));
  tm = toneMark(); runFor(300);
  eqInt((long) tonesSince(tm).size(), 0, "  ...and it sustains: not one note re-struck");
  eqInt(litLeds(), 0, "  ...and the bar is dark again, like any single note");
  board.touched[0] = false; runFor(300);
  ok(ledPattern() == "..........", "  ...and dark once both are let go", ledPattern());

  // ...and the other way round: lifting the LOWER voice promotes the upper one.
  sm = serialMark();
  board.touched[0] = true;  runFor(80);
  board.touched[4] = true;  runFor(300);
  ok(serialSince(sm, "~~ chord 1+5"), "a second chord opens the same way");
  tm = toneMark();
  board.touched[0] = false; runFor(300);
  t = tonesSince(tm);
  ok(!t.empty() && t.back() == NOTE_G5,
     "lifting the LOWER lemon promotes the upper one instead of stopping",
     t.empty() ? "no tone" : "ended on " + std::to_string(t.back()));
  ok(serialSince(sm, "holds on"), "  ...and the log says which voice held on");
  eqInt(litLeds(), 0, "  ...and the bar is dark, like any single note");
  board.touched[4] = false; runFor(300);
}

// ── 21 ──────────────────────────────────────────────────────────────────────
// The expensive failure. A single note that warbles like two would ruin every
// note anyone plays, so the three gates in chordPartnerFor() are pinned here
// one at a time: a shadow too shallow, a shadow deep enough to pass the floor
// but not the ratio, and a whole hand across the fruit.
static void test_one_finger_is_never_a_chord() {
  section("21. FREE PLAY: one finger and its shadows is never a chord");
  boot();
  board.touchDepth = 12;
  board.couplingDepth = 5;            // shallow shadow: fails the absolute floor
  toggleFreePlayByHold();

  size_t sm = serialMark(), tm = toneMark();
  board.touched[3] = true; runFor(600);
  std::vector<int> t = tonesSince(tm);
  bool onlyFa = !t.empty();
  for (size_t i = 0; i < t.size(); i++) if (t[i] != NOTE_F5) onlyFa = false;
  ok(onlyFa, "one finger, one note, held - whatever the neighbours are doing",
     std::to_string(t.size()) + " tones, not all F5");
  ok(!serialSince(sm, "~~ chord"), "  ...no second voice was opened");
  eqInt(litLeds(), 0, "  ...and the bar is dark: one finger leaves no standing light");
  board.touched[3] = false; runFor(300);

  // A DEEPER shadow: 8 counts against the finger's 12 clears the absolute floor
  // and is still refused, because 67 % is under the 70 % a real finger makes.
  board.couplingDepth = 8;
  sm = serialMark(); tm = toneMark();
  board.touched[3] = true; runFor(600);
  t = tonesSince(tm);
  onlyFa = !t.empty();
  for (size_t i = 0; i < t.size(); i++) if (t[i] != NOTE_F5) onlyFa = false;
  ok(onlyFa, "a shadow deep enough to pass the floor is still refused by the ratio");
  ok(!serialSince(sm, "~~ chord"), "  ...still one note");
  board.touched[3] = false; runFor(300);

  // Three real fingers: that is a hand laid across the fruit, not a chord, and
  // the answer is one note rather than a guess at which two were meant.
  board.couplingDepth = 5;
  sm = serialMark();
  board.touched[0] = true;
  board.touched[2] = true;
  board.touched[4] = true; runFor(500);
  ok(!serialSince(sm, "~~ chord"), "three lemons at once open no chord at all");
  board.touched[0] = board.touched[2] = board.touched[4] = false; runFor(300);
}

// ── 22 ──────────────────────────────────────────────────────────────────────
// Sergio, 2026-09-15: on level 4 "suena la melodía, pero cuando yo toco las
// notas no están en la tonalidad correcta de ese nivel". He was right, and it
// was a whole design gap rather than a bug: levels 1 and 2 had always drawn
// their seven key notes out of their own themes, levels 3 and 4 were given
// plain C major runs. Level 4's theme is in G minor around G3-G4, so its lemons
// answered it a C major scale an octave up.
//
// This is the test that makes it impossible to reintroduce: EVERY lemon on
// EVERY level has to be a note that level's own theme actually plays.
static bool themeHas(const int *notes, size_t len, int freq) {
  for (size_t i = 0; i < len; i++) if ((int) pgm_read_word(&notes[i]) == freq) return true;
  return false;
}
static void test_every_level_plays_in_its_own_key() {
  section("22. Every level's seven lemons are notes from that level's own theme");
  boot();
  struct Row { const char *name; const int *notes; size_t len; int row; };
  const Row L[4] = {
    {"1 Overworld",  marioNotes,      MARIO_LEN,   0},
    {"2 Underworld", underworldNotes, UNDER_LEN,   1},
    {"3 Starman",    starmanNotes,    STARMAN_LEN, 2},
    {"4 Castle",     castleNotes,     CASTLE_LEN,  3},
  };
  for (int l = 0; l < 4; l++) {
    const int *row = &keys[L[l].row * KEY_COUNT];
    std::string strays;
    for (int k = 0; k < (int) KEY_COUNT; k++)
      if (!themeHas(L[l].notes, L[l].len, row[k]))
        strays += " key" + std::to_string(k + 1) + "=" + std::to_string(row[k]);
    ok(strays.empty(), std::string("level ") + L[l].name +
       ": every lemon is a note the theme plays", "strays:" + strays);
    int dup = 0;
    for (int a = 0; a < (int) KEY_COUNT; a++)
      for (int b = a + 1; b < (int) KEY_COUNT; b++) if (row[a] == row[b]) dup++;
    eqInt(dup, 0, "  ...and no two lemons share a note (they must be tellable apart)");
  }

  // ...and the CODES are untouched by the retuning. They are written down as
  // key numbers — the organiser's booklet prints them on paper — so changing
  // which note a lemon plays must not change which lemons a player presses.
  const int want[4][SEQUENCE_LENGTH] = {
    {6,5,6,7,2,5,2,1,3,4},
    {3,6,1,4,2,5,3,6,1,4},
    {2,4,6,1,5,3,7,4,2,6},
    {5,1,3,7,2,6,4,1,5,3},
  };
  const int *seq[4] = {sequence_1, sequence_2, sequence_3, sequence_4};
  for (int l = 0; l < 4; l++) {
    const int *row = &keys[l * KEY_COUNT];
    std::string got, bad;
    for (int i = 0; i < SEQUENCE_LENGTH; i++) {
      int num = 0;
      for (int k = 0; k < (int) KEY_COUNT; k++) if (row[k] == seq[l][i]) num = k + 1;
      got += std::to_string(num);
      if (num != want[l][i]) bad = "wrong at step " + std::to_string(i + 1);
      if (i > 0 && seq[l][i] == seq[l][i - 1])
        bad = "step " + std::to_string(i + 1) + " repeats the note before it";
    }
    ok(bad.empty(), std::string("level ") + std::to_string(l + 1) +
       "'s code is still " + got, bad + " (got " + got + ")");
  }
}

// ── 23 ──────────────────────────────────────────────────────────────────────
// The retuned levels are not just in tune, they are still PLAYABLE: level 4's
// code, typed on the lemons it names, still wins.
static void test_level_four_still_wins() {
  section("23. Level 4, chosen from the wheel, is won by the code on paper");
  boot();
  openMenu();
  tapPlus(); runFor(2600);            // level 2
  tapPlus(); runFor(2600);            // level 3
  tapPlus(); runFor(2600);            // level 4
  acceptSelection();
  eqInt(level, 4, "the wheel put us on level 4");

  // ...and the lemons it hands you are the Castle's own, low and in G minor.
  size_t tm = toneMark();
  touchKey(0);
  std::vector<int> t = tonesSince(tm);
  ok(!t.empty() && t[0] == NOTE_G3, "lemon 1 plays G3 - the theme's own pedal note",
     t.empty() ? "silence" : "got " + std::to_string(t[0]));

  // Winning LEVEL 3 is the end-to-end proof, and it is the one that can be run:
  // clearing level 4 is clearing the last level, which drops into the ending
  // loop — a piece that only stops for a button gesture and therefore never
  // returns on a fake board (see playEndingLoop).
  boot();
  level = 3; resetBoard(); runFor(50);
  size_t sm = serialMark();
  const int code3[SEQUENCE_LENGTH] = {2,4,6,1,5,3,7,4,2,6};
  for (int i = 0; i < SEQUENCE_LENGTH; i++) touchKey(code3[i] - 1);
  ok(serialSince(sm, "WIN"), "the printed code wins the retuned level 3");
  eqInt(level, 4, "  ...and it advances to level 4");

  // ...and on level 4 the same code takes the bar to nine of ten, with the
  // tenth lemon holding the note the sequence is waiting for. That is the whole
  // code proven without letting the ending loop start.
  sm = serialMark();
  const int code4[SEQUENCE_LENGTH] = {5,1,3,7,2,6,4,1,5,3};
  for (int i = 0; i < SEQUENCE_LENGTH - 1; i++) touchKey(code4[i] - 1);
  ok(serialSince(sm, "OK 9/10"), "level 4's code scores nine notes out of ten");
  eqInt(keys[3 * KEY_COUNT + code4[SEQUENCE_LENGTH - 1] - 1],
        sequence_4[SEQUENCE_LENGTH - 1],
        "  ...and the tenth lemon plays exactly the note it is waiting for");
}

// ── 24 ──────────────────────────────────────────────────────────────────────
// The wave breaks outward FROM the lemon: an end key sends one light the length
// of the bar, a middle key sends two lights parting. Asserted from the film,
// because by the time the note settles the wave has been and gone.
static void test_free_play_wave_starts_at_the_key() {
  section("24. FREE PLAY: the wave breaks outward from the lemon played");
  boot();
  toggleFreePlayByHold();

  size_t fm = frameMark();
  board.touched[0] = true; runFor(400);
  std::vector<std::string> film = framesSince(fm);
  ok(filmHas(film, "#........."), "key 1: the wave starts at the left end");
  ok(filmHas(film, ".........#"), "  ...and reaches the right one");
  board.touched[0] = false; runFor(300);

  fm = frameMark();
  board.touched[6] = true; runFor(400);
  film = framesSince(fm);
  size_t atRight = 0, atLeft = 0;
  for (size_t i = 0; i < film.size(); i++) {
    if (film[i] == ".........#" && atRight == 0) atRight = i + 1;
    if (film[i] == "#........." && atLeft == 0) atLeft = i + 1;
  }
  ok(atRight && atLeft && atRight < atLeft,
     "key 7: the wave runs the other way, right to left",
     "right at " + std::to_string(atRight) + ", left at " + std::to_string(atLeft));
  board.touched[6] = false; runFor(300);

  // Key 4 sits at LED 5 of ten, so its wave opens in BOTH directions at once —
  // two lit LEDs, symmetric about the middle, which no end key ever produces.
  fm = frameMark();
  board.touched[3] = true; runFor(400);
  film = framesSince(fm);
  ok(filmHas(film, "...#.#...."), "key 4: the wave parts in both directions");
  // Key 4 sits at LED 5 of ten, so the two fronts are NOT symmetric about the
  // bar: the left one runs out after four steps and the right one keeps going.
  // "#.......#." is that fourth step, which no end key can draw.
  ok(filmHas(film, "#.......#."), "  ...both fronts travelling, off-centre as they are");
  board.touched[3] = false; runFor(300);
}

// ── 25 ──────────────────────────────────────────────────────────────────────
// Sergio wanted a way to hear the level's tune again, and the FIRST version of
// it — hold the two end lemons together — was a good gesture built on the one
// thing this keyboard cannot do. He called it: "creo que es físicamente
// imposible detectar correctamente la pulsación de varias teclas a la vez. Por
// lo cual vamos a cambiar el paradigma." So it is five presses of ONE lemon
// now: nothing simultaneous, nothing to disambiguate.
//
// The two things this has to guarantee are that it FIRES and that it costs
// nothing — "el juego continúa por donde estaba".
static void test_five_taps_replay_the_theme() {
  section("25. Five taps on one lemon replay the theme, and change nothing");
  boot();
  // A theme is bit-banged, so the buzzer's edge count straight after boot IS
  // one playing of the level-1 intro. The replay has to match it exactly.
  const unsigned long oneIntro = board.pinWrites[BUZZER];

  const int code[10] = {5, 4, 5, 6, 1, 4, 1, 0, 2, 3};
  touchKey(code[0]);
  touchKey(code[1]);
  touchKey(code[2]);
  eqInt(litLeds(), 3, "the game is three notes in");

  // Drum the lemon the game has just accepted. THE NOTE YOU JUST PLAYED IS
  // PRESS ONE — that is the whole shape of the gesture, and it is what Sergio
  // described: "pulsa cuatro veces cualquier nota... pulsa, suelta y pulsa de
  // nuevo hasta cinco veces". So four MORE taps here, not five. Drumming the
  // accepted lemon also means every one of them is a repeat, which scores
  // nothing and costs nothing — the obvious move for a player who wants the
  // hint for free.
  const unsigned long before = board.pinWrites[BUZZER];
  size_t sm = serialMark();
  for (int i = 0; i < 4; i++) touchKey(code[2]);

  ok(serialSince(sm, "playing the theme again"), "the fifth tap asks for the theme");
  eqInt((long) (board.pinWrites[BUZZER] - before), (long) oneIntro,
        "  ...and it is the SAME tune, edge for edge, that level 1 opened with");
  eqInt(currentStep, 3, "  ...the game kept its place");
  eqInt(litLeds(), 3, "  ...and so did the bar");
  ok(!serialSince(sm, "WRONG"), "  ...nothing was punished");
  ok(!serialSince(sm, "OK 4/10"), "  ...and nothing was scored either");

  sm = serialMark();
  touchKey(code[3]);
  ok(serialSince(sm, "OK 4/10"), "and the game carries on from note 4");

  // FOUR is not five, and a different lemon in between starts the count over.
  boot();
  sm = serialMark();
  for (int i = 0; i < 4; i++) touchKey(0);
  ok(!serialSince(sm, "playing the theme again"), "four taps are not five");
  touchKey(1);
  for (int i = 0; i < 4; i++) touchKey(0);
  ok(!serialSince(sm, "playing the theme again"),
     "  ...and a different lemon in the middle starts the run again");
  touchKey(0);
  ok(serialSince(sm, "playing the theme again"),
     "  ...the fifth of the NEW run is what fires it");

  // ...and so does a long enough pause: drumming has to be a decision.
  boot();
  sm = serialMark();
  for (int i = 0; i < 4; i++) touchKey(0);
  runFor(2500);                       // longer than THEME_REPEAT_GAP_MS
  touchKey(0);
  ok(!serialSince(sm, "playing the theme again"),
     "a pause longer than the gap breaks the run too");

  // FREE PLAY MUST NEVER COUNT REPEATS. Playing one lemon over and over is the
  // entire point of the mode, so five taps there are five notes and nothing else.
  boot();
  toggleFreePlayByHold();
  const unsigned long quiet = board.pinWrites[BUZZER];
  sm = serialMark();
  size_t tm = toneMark();
  for (int i = 0; i < 8; i++) touchKey(2);
  ok(!serialSince(sm, "playing the theme again"),
     "free play: eight taps on one lemon ask for nothing");
  eqInt((long) (board.pinWrites[BUZZER] - quiet), 0,
        "  ...not a single note of a theme was played");
  int mi = 0;
  std::vector<int> t = tonesSince(tm);
  for (size_t i = 0; i < t.size(); i++) if (t[i] == NOTE_E5) mi++;
  eqInt(mi, 8, "  ...they were eight notes, which is what an instrument is for");
}

// ── 26 ──────────────────────────────────────────────────────────────────────
// Sergio, 2026-09-15: "hay tres versiones de cada canción. La corta, que es
// para el menú. Una de siete notas y únicamente de las siete notas que se
// tocan... y luego la versión ultra completa, full, que es ya cuando el nivel
// es completado y como celebración."
//
// The middle one is the interesting one, because it is the CLUE: the level
// announcement and the five-tap reminder must be made only of notes the player
// can answer with. Every theme reaches pitches that are on no lemon.
static void test_three_sizes_of_every_theme() {
  section("26. Every theme at three sizes: menu, the playable clue, the whole piece");
  boot();

  // The announcement is exactly seven notes, and it is the Overworld's riff.
  std::vector<int> t = lastThemeLine();
  eqInt((long) t.size(), 7, "level 1 announces itself with exactly seven notes");
  const int want1[7] = {NOTE_E7, NOTE_E7, NOTE_E7, NOTE_C7, NOTE_E7, NOTE_G7, NOTE_G6};
  bool same = t.size() == 7;
  for (size_t i = 0; i < t.size() && i < 7; i++) if (t[i] != want1[i]) same = false;
  ok(same, "  ...and it is the riff: E7 E7 E7 C7 E7 G7 G6");

  // ...and on EVERY level, every note of it is a note that level's lemons make.
  for (int lvl = 1; lvl <= LEVEL_COUNT; lvl++) {
    boot();
    level = lvl;
    resetBoard();
    playLevelIntro();
    t = lastThemeLine();
    eqInt((long) t.size(), 7,
          std::string("level ") + std::to_string(lvl) + ": seven notes announced");
    std::string strays;
    for (size_t i = 0; i < t.size(); i++) {
      bool found = false;
      for (int k = 0; k < (int) KEY_COUNT; k++)
        if (keys[(lvl - 1) * KEY_COUNT + k] == t[i]) found = true;
      if (!found) strays += " " + std::to_string(t[i]);
    }
    ok(strays.empty(), "  ...and every one of them is a note its lemons can play",
       "strays:" + strays);
  }

  // The CELEBRATION is the whole piece, and is much longer than the clue.
  boot();
  const unsigned long announce = board.pinWrites[BUZZER];
  const int code[10] = {5, 4, 5, 6, 1, 4, 1, 0, 2, 3};
  const unsigned long beforeWin = board.pinWrites[BUZZER];
  size_t sm = serialMark();
  for (int i = 0; i < SEQUENCE_LENGTH; i++) touchKey(code[i]);
  ok(serialSince(sm, "WIN"), "level 1 is won");
  const unsigned long celebration = board.pinWrites[BUZZER] - beforeWin;
  ok(celebration > announce * 4,
     "  ...and the celebration is the whole piece, many times the clue",
     std::to_string(celebration) + " edges against the clue's " +
     std::to_string(announce));
}

int main(int argc, char **argv) {
  if (argc > 1 && std::string(argv[1]) == "-v") board.traceSerial = true;
  printf("\n\033[1mLemon Piano V5.5 — firmware on a fake board\033[0m\n");
  printf("src/main.cpp compiled against test/arduino/Arduino.h — no hardware, no emulator\n");

  test_boot_unchanged();
  test_game_still_scores();
  test_menu_opens_and_previews();
  test_wheel_reaches_every_level_and_wraps();
  test_free_play_mapping();
  test_free_play_repeats_and_never_scores();
  test_free_play_bar_never_stands_still();
  test_cancel_changes_nothing();
  test_accept_restarts_the_chosen_level();
  test_opening_the_menu_gives_the_knob_back();
  test_winning_never_lands_on_free_play();
  test_open_and_accept_is_a_no_op();
  test_first_conversion_is_discarded();
  test_minus_button_does_not_press_a_key();
  test_free_play_is_a_switch_on_plus();
  test_held_fruit_sounds_once();
  test_bar_counts_sensitivity();
  test_repeat_sounds_but_scores_nothing();
  test_free_play_opens_with_a_sweep();
  test_two_lemons_sound_as_two_notes();
  test_one_finger_is_never_a_chord();
  test_every_level_plays_in_its_own_key();
  test_level_four_still_wins();
  test_free_play_wave_starts_at_the_key();
  test_five_taps_replay_the_theme();
  test_three_sizes_of_every_theme();

  printf("\n%d checks, \033[%sm%d failed\033[0m\n\n",
         checks, failures ? "31" : "32", failures);
  return failures ? 1 : 0;
}
