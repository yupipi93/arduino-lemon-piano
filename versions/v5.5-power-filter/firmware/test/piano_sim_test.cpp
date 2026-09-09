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
// The documented way in: hold − for three seconds.
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
  touchKey(5);                                    // the SAME key again
  ok(!serialSince(sm, "OK 2/10"), "the same lemon twice does NOT score twice (the game's repeat filter)");
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
  ok(serialSince(sm, "keep holding for the mode menu"), "  ...it warns before it opens");
  ok(serialSince(sm, "MODE MENU"), "  ...and at 3 s the wheel is open");
  ok(tonesSince(tm).size() > 3, "  ...with sound to match",
     std::to_string(tonesSince(tm).size()) + " tones");
  ok(serialSince(sm, "> Level 1"), "  ...opened ON the mode being played");
  board.minusDown = false; runFor(300);
}

static void test_wheel_reaches_free_play_and_wraps() {
  section("4. The wheel turns to every mode, and wraps");
  boot();
  openMenu();
  size_t sm = serialMark();
  tapPlus(); runFor(2600);  ok(serialSince(sm, "> Level 2"), "+ -> Level 2");
  sm = serialMark();
  tapPlus(); runFor(3000);  ok(serialSince(sm, "> Level 3"), "+ -> Level 3");
  sm = serialMark();
  tapPlus(); runFor(4000);  ok(serialSince(sm, "> Level 4"), "+ -> Level 4");
  sm = serialMark();
  tapPlus(); runFor(1500);  ok(serialSince(sm, "> FREE PLAY"), "+ -> FREE PLAY, the last item");
  sm = serialMark();
  tapPlus(); runFor(2600);  ok(serialSince(sm, "> Level 1"), "+ WRAPS round to Level 1");
  sm = serialMark();
  tapMinus(); runFor(1500); ok(serialSince(sm, "> FREE PLAY"), "− WRAPS the other way");
  eqInt(level, 1, "and nothing has been accepted yet: still playing level 1");
}

static void test_free_play_mapping() {
  section("5. FREE PLAY: the seven lemons are do re mi fa sol la si");
  boot();
  openMenu();
  tapPlus(); runFor(2600); tapPlus(); runFor(3000);
  tapPlus(); runFor(4000); tapPlus(); runFor(1500);   // -> FREE PLAY
  acceptSelection();
  eqInt(level, FREE_PLAY, "the wheel handed over the instrument");
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
  openMenu();
  tapPlus(); runFor(2600); tapPlus(); runFor(3000);
  tapPlus(); runFor(4000); tapPlus(); runFor(1500);
  acceptSelection();

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

static void test_free_play_pitch_bar() {
  section("7. FREE PLAY: the bar is a pitch meter, and its idle shape is unique");
  boot();
  openMenu();
  tapPlus(); runFor(2600); tapPlus(); runFor(3000);
  tapPlus(); runFor(4000); tapPlus(); runFor(1500);
  acceptSelection();
  ok(ledPattern() == "#........#", "idle: only the two ends are lit", ledPattern());

  board.touched[0] = true; runFor(150);
  eqInt(litLeds(), 1, "the lowest lemon lights one LED");
  board.touched[0] = false; runFor(200);

  board.touched[6] = true; runFor(150);
  eqInt(litLeds(), 10, "the highest lights all ten");
  board.touched[6] = false; runFor(200);
  ok(ledPattern() == "#........#", "  ...and it goes back to the idle shape", ledPattern());
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

int main(int argc, char **argv) {
  if (argc > 1 && std::string(argv[1]) == "-v") board.traceSerial = true;
  printf("\n\033[1mLemon Piano V5.5 — firmware on a fake board\033[0m\n");
  printf("src/main.cpp compiled against test/arduino/Arduino.h — no hardware, no emulator\n");

  test_boot_unchanged();
  test_game_still_scores();
  test_menu_opens_and_previews();
  test_wheel_reaches_free_play_and_wraps();
  test_free_play_mapping();
  test_free_play_repeats_and_never_scores();
  test_free_play_pitch_bar();
  test_cancel_changes_nothing();
  test_accept_restarts_the_chosen_level();
  test_opening_the_menu_gives_the_knob_back();
  test_winning_never_lands_on_free_play();
  test_open_and_accept_is_a_no_op();

  printf("\n%d checks, \033[%sm%d failed\033[0m\n\n",
         checks, failures ? "31" : "32", failures);
  return failures ? 1 : 0;
}
