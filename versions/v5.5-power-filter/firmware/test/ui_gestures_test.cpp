/* ui_gestures_test.cpp — the deterministic proof that the three gestures
 * sharing two buttons never step on each other.
 *
 * The Arduino is not the place to find out that "hold − for the menu" ate the
 * sensitivity ramp, or that accepting a level also navigated one past it. This
 * runs ../include/ui_gestures.h on the host, drives it through every timeline
 * that mixes the gestures, and asserts the exact sequence of events each one
 * must produce. No Arduino, no hardware, no emulator: `./run.sh`, exit 0.
 *
 * Build & run:  versions/v5.5-power-filter/firmware/test/run.sh
 */
#include "../include/ui_gestures.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

typedef UiGestures::Event Ev;

static const char *evName(Ev e) {
  switch (e) {
    case UiGestures::EV_NONE:         return "NONE";
    case UiGestures::EV_NUDGE_MORE:   return "NUDGE_MORE";
    case UiGestures::EV_NUDGE_LESS:   return "NUDGE_LESS";
    case UiGestures::EV_SMART_ADJUST: return "SMART_ADJUST";
    case UiGestures::EV_ARM_START:    return "ARM_START";
    case UiGestures::EV_ARM_TICK:     return "ARM_TICK";
    case UiGestures::EV_ARM_CANCEL:   return "ARM_CANCEL";
    case UiGestures::EV_MENU_OPEN:    return "MENU_OPEN";
    case UiGestures::EV_MENU_NEXT:    return "MENU_NEXT";
    case UiGestures::EV_MENU_PREV:    return "MENU_PREV";
    case UiGestures::EV_MENU_ACCEPT:  return "MENU_ACCEPT";
    case UiGestures::EV_MENU_CANCEL:  return "MENU_CANCEL";
  }
  return "?";
}

// ── the rig: a fake millis() and two fake buttons ───────────────────────────
struct Sim {
  UiGestures g;
  uint32_t t = 0;
  bool plus = false, minus = false;
  uint32_t tick = 1;                  // loop() period, in ms
  std::vector<Ev> seen;
  std::vector<uint32_t> at;

  explicit Sim(uint8_t items = 5, uint32_t tickMs = 1) : tick(tickMs) {
    g.begin(items);
  }
  void step() {
    Ev e = g.update(plus, minus, t);
    if (e != UiGestures::EV_NONE) { seen.push_back(e); at.push_back(t); }
    t += tick;
  }
  void advance(uint32_t ms) { for (uint32_t i = 0; i < ms; i += tick) step(); }
  void hold(bool &b, uint32_t ms) { b = true;  advance(ms); }
  void off(bool &b, uint32_t ms)  { b = false; advance(ms); }
  // A tap: down for `downMs`, then up for `upMs` (long enough to debounce).
  void tapPlus(uint32_t downMs = 90, uint32_t upMs = 120) {
    hold(plus, downMs); off(plus, upMs);
  }
  void tapMinus(uint32_t downMs = 90, uint32_t upMs = 120) {
    hold(minus, downMs); off(minus, upMs);
  }
  int count(Ev e) const {
    int n = 0;
    for (size_t i = 0; i < seen.size(); i++) if (seen[i] == e) n++;
    return n;
  }
  // The event sequence with repeats collapsed — timelines that ramp emit dozens
  // of identical nudges, and what matters is the SHAPE.
  std::string shape() const {
    std::string s;
    for (size_t i = 0; i < seen.size(); i++) {
      if (i && seen[i] == seen[i - 1]) continue;
      if (!s.empty()) s += " ";
      s += evName(seen[i]);
    }
    return s;
  }
  std::string full() const {
    std::string s;
    for (size_t i = 0; i < seen.size(); i++) {
      char buf[64];
      snprintf(buf, sizeof buf, "%s@%lu ", evName(seen[i]), (unsigned long) at[i]);
      s += buf;
    }
    return s;
  }
};

// ── assertions ──────────────────────────────────────────────────────────────
static int failures = 0;
static int checks = 0;

static void ok(bool cond, const std::string &what, const std::string &detail) {
  checks++;
  if (cond) {
    printf("  \033[32mPASS\033[0m  %s\n", what.c_str());
  } else {
    failures++;
    printf("  \033[31mFAIL\033[0m  %s\n        %s\n", what.c_str(), detail.c_str());
  }
}
static void eqShape(const Sim &s, const char *expected, const char *what) {
  ok(s.shape() == expected, what,
     "expected shape [" + std::string(expected) + "]\n        got      [" +
     s.shape() + "]\n        raw      " + s.full());
}
static void eqInt(long got, long expected, const char *what) {
  ok(got == expected, what,
     "expected " + std::to_string(expected) + ", got " + std::to_string(got));
}
static void section(const char *name) { printf("\n\033[1m%s\033[0m\n", name); }

// ── the use cases, one function each ────────────────────────────────────────

// 1. The sensitivity knob still works exactly as it did before the menu existed.
static void test_tap_each_button() {
  section("1. Taps still nudge the sensitivity");
  { Sim s; s.tapPlus();  eqShape(s, "NUDGE_MORE", "a tap on + is one step, more sensitive"); }
  { Sim s; s.tapMinus(); eqShape(s, "NUDGE_LESS", "a tap on − is one step, less sensitive"); }
}

static void test_plus_ramps_forever() {
  section("2. + has no long press: it just ramps");
  Sim s;
  s.hold(s.plus, 4000);
  eqShape(s, "NUDGE_MORE", "holding + for 4 s only ever nudges");
  eqInt(s.count(UiGestures::EV_MENU_OPEN), 0, "  ...and never opens the menu");
  eqInt(s.count(UiGestures::EV_ARM_START), 0, "  ...and never arms it");
  // 400 ms delay, then one every 120 ms, plus the initial press.
  ok(s.count(UiGestures::EV_NUDGE_MORE) >= 25 && s.count(UiGestures::EV_NUDGE_MORE) <= 32,
     "  ...at the documented ramp rate (1 + ~30 in 4 s)",
     "got " + std::to_string(s.count(UiGestures::EV_NUDGE_MORE)));
}

// 3. The collision that mattered most: − is both the knob and the menu key.
static void test_minus_ramp_stops_when_arming() {
  section("3. − ramps the knob, then stops and arms the menu");
  Sim s;
  s.hold(s.minus, 900);              // still purely a knob down here
  eqInt(s.count(UiGestures::EV_ARM_START), 0, "at 900 ms it has not armed yet");
  ok(s.count(UiGestures::EV_NUDGE_LESS) >= 4, "  ...and it HAS been ramping the knob",
     "got " + std::to_string(s.count(UiGestures::EV_NUDGE_LESS)) + " nudges");

  s.advance(150);                    // ...crossing the 1 s arming line
  eqInt(s.count(UiGestures::EV_ARM_START), 1, "at 1 s the menu arms");
  const int nudgesAtArming = s.count(UiGestures::EV_NUDGE_LESS);

  s.advance(1200);                   // now well inside the charge
  eqInt(s.count(UiGestures::EV_NUDGE_LESS), nudgesAtArming,
        "  ...and the knob stopped moving the moment it armed");
  ok(s.count(UiGestures::EV_ARM_TICK) >= 1, "  ...with audible ticks while it charges",
     "got " + std::to_string(s.count(UiGestures::EV_ARM_TICK)));
  eqInt(s.count(UiGestures::EV_MENU_OPEN), 0, "  ...but it is not open yet");
}

static void test_minus_release_cancels_arming() {
  section("4. Letting − go mid-charge cancels, and changes nothing");
  Sim s;
  s.hold(s.minus, 2000);
  s.off(s.minus, 200);
  eqInt(s.count(UiGestures::EV_ARM_CANCEL), 1, "releasing at 2 s cancels the arming");
  eqInt(s.count(UiGestures::EV_MENU_OPEN), 0, "  ...and the menu never opened");
  ok(!s.g.inMenu(), "  ...and we are still playing", "inMenu() == true");
}

static void test_minus_opens_menu() {
  section("5. Holding − for 3 s opens the menu");
  Sim s;
  s.hold(s.minus, 3100);
  eqInt(s.count(UiGestures::EV_MENU_OPEN), 1, "the menu opens, exactly once");
  eqInt(s.count(UiGestures::EV_SMART_ADJUST), 0, "  ...and never fires smart adjust");
  ok(s.g.inMenu(), "  ...and inMenu() says so", "inMenu() == false");
  // Still holding: nothing more may happen until the button is released.
  size_t before = s.seen.size();
  s.advance(4000);
  eqInt((long) (s.seen.size() - before), 0,
        "  ...and keeping − held afterwards does nothing at all");
  // And the release that ends the gesture must not navigate the wheel.
  s.off(s.minus, 300);
  eqInt(s.count(UiGestures::EV_MENU_PREV), 0,
        "  ...nor does letting go of it move the wheel");
  eqInt((long) s.g.item(), 0, "  ...the wheel is still on the item it opened on");
}

static void test_arm_meter_fills() {
  section("6. The arming meter is a real progress bar");
  Sim s;
  s.hold(s.minus, 1100);
  ok(s.g.arming(), "it is arming at 1.1 s", "arming() == false");
  uint8_t at1100 = s.g.armPercent(s.t);
  s.advance(900);
  uint8_t at2000 = s.g.armPercent(s.t);
  s.advance(900);
  uint8_t at2900 = s.g.armPercent(s.t);
  ok(at1100 < 20 && at2000 > 40 && at2000 < 60 && at2900 > 90,
     "  ...and it fills 0 -> 100 % across the 1 s → 3 s charge",
     "got " + std::to_string(at1100) + "% / " + std::to_string(at2000) + "% / " +
     std::to_string(at2900) + "%");
}

// 7. The wheel, including the wrap Sergio asked for in both directions.
static void test_wheel_wraps_both_ways() {
  section("7. The wheel turns, and wraps, in both directions");
  Sim s;                                   // 5 items: levels 1-4 + free play
  s.hold(s.minus, 3100); s.off(s.minus, 300);
  eqInt((long) s.g.item(), 0, "the menu opened on item 0");
  s.tapPlus(); eqInt((long) s.g.item(), 1, "+ -> item 1");
  s.tapPlus(); eqInt((long) s.g.item(), 2, "+ -> item 2");
  s.tapPlus(); eqInt((long) s.g.item(), 3, "+ -> item 3");
  s.tapPlus(); eqInt((long) s.g.item(), 4, "+ -> item 4 (free play, the last one)");
  s.tapPlus(); eqInt((long) s.g.item(), 0, "+ WRAPS back round to item 0");
  s.tapMinus(); eqInt((long) s.g.item(), 4, "− WRAPS the other way, to item 4");
  s.tapMinus(); eqInt((long) s.g.item(), 3, "− -> item 3");
  eqInt(s.count(UiGestures::EV_MENU_ACCEPT), 0, "and no tap was ever read as an accept");
}

static void test_navigation_is_on_release() {
  section("8. A tap moves the wheel when it is RELEASED, not when pressed");
  Sim s;
  s.hold(s.minus, 3100); s.off(s.minus, 300);
  s.plus = true;
  s.advance(500);                          // held, not yet released
  eqInt((long) s.g.item(), 0, "holding + has not moved the wheel yet");
  s.off(s.plus, 100);
  eqInt((long) s.g.item(), 1, "  ...it moves on release");
}

// 9. Accept: both buttons, in either order, with any overlap.
static void test_accept_both_buttons() {
  section("9. Both buttons together accept the selection");
  for (int order = 0; order < 2; order++) {
    for (int gap = 0; gap <= 400; gap += 200) {
      Sim s;
      s.hold(s.minus, 3100); s.off(s.minus, 300);
      s.tapPlus();                                   // move to item 1 first
      bool &first  = order ? s.minus : s.plus;
      bool &second = order ? s.plus  : s.minus;
      first = true;
      s.advance((uint32_t) gap);                     // the two never land together
      second = true;
      s.advance(150);
      char what[128];
      snprintf(what, sizeof what, "%s first, %d ms apart -> ACCEPT (item %u)",
               order ? "−" : "+", gap, (unsigned) s.g.item());
      eqInt(s.count(UiGestures::EV_MENU_ACCEPT), 1, what);
      eqInt((long) s.g.item(), 1, "  ...on the item that was selected, not the next one");
      ok(!s.g.inMenu(), "  ...and the menu is closed", "still in menu");
      // Releasing both must not navigate or re-accept.
      size_t before = s.seen.size();
      s.plus = false; s.minus = false; s.advance(400);
      eqInt((long) (s.seen.size() - before), 0, "  ...and letting go does nothing more");
    }
  }
}

static void test_accept_after_first_button_waits() {
  section("10. The first of the two buttons never navigates before the second lands");
  Sim s;
  s.hold(s.minus, 3100); s.off(s.minus, 300);
  s.plus = true;
  s.advance(900);                          // a LONG wait between the two presses
  s.minus = true;
  s.advance(150);
  eqInt(s.count(UiGestures::EV_MENU_ACCEPT), 1,
        "even 900 ms apart it is an accept");
  eqInt(s.count(UiGestures::EV_MENU_NEXT) + s.count(UiGestures::EV_MENU_PREV), 0,
        "  ...with no stray step in between");
  eqInt((long) s.g.item(), 0, "  ...on the item that was showing");
}

// 11. Escape hatches — the menu must never be a trap.
static void test_menu_long_press_cancels() {
  section("11. A long press in the menu is the escape hatch");
  for (int which = 0; which < 2; which++) {
    Sim s;
    s.hold(s.minus, 3100); s.off(s.minus, 300);
    s.tapPlus();                                       // wheel on item 1
    bool &b = which ? s.minus : s.plus;
    s.hold(b, 3100);
    eqInt(s.count(UiGestures::EV_MENU_CANCEL), 1,
          which ? "holding − for 3 s leaves the menu" : "holding + for 3 s leaves the menu");
    eqInt(s.count(UiGestures::EV_MENU_ACCEPT), 0, "  ...without accepting anything");
    ok(!s.g.inMenu(), "  ...and we are back to playing", "still in menu");
    size_t before = s.seen.size();
    s.off(b, 300);
    eqInt((long) (s.seen.size() - before), 0, "  ...and its release does not navigate");
  }
}

static void test_menu_idle_timeout() {
  section("12. An abandoned menu closes itself");
  Sim s;
  s.hold(s.minus, 3100); s.off(s.minus, 300);
  s.advance(19000);
  eqInt(s.count(UiGestures::EV_MENU_CANCEL), 0, "at 19 s it is still waiting");
  s.advance(2000);
  eqInt(s.count(UiGestures::EV_MENU_CANCEL), 1, "at 20 s idle it closes itself");
  ok(!s.g.inMenu(), "  ...and hands the piano back", "still in menu");
}

// 13-15. The overlaps, stated as the questions someone would actually ask.
static void test_smart_adjust_still_works() {
  section("13. Smart adjust survived: both buttons, 1 s, while playing");
  Sim s;
  s.plus = true; s.advance(30); s.minus = true;   // a human presses them ~30 ms apart
  s.advance(1200);
  eqInt(s.count(UiGestures::EV_SMART_ADJUST), 1, "both held 1 s = smart adjust, once");
  eqInt(s.count(UiGestures::EV_MENU_OPEN), 0, "  ...and it is not the menu");
  s.advance(3000);
  eqInt(s.count(UiGestures::EV_SMART_ADJUST), 1, "  ...and it does not repeat while held");
  s.plus = false; s.minus = false; s.advance(300);
  eqInt(s.count(UiGestures::EV_MENU_PREV), 0, "  ...and the release is swallowed");
}

static void test_holding_both_never_opens_menu() {
  section("14. Holding BOTH for 3 s is smart adjust, never the menu");
  Sim s;
  s.plus = true; s.minus = true;
  s.advance(3500);
  eqInt(s.count(UiGestures::EV_MENU_OPEN), 0, "3.5 s on both buttons: no menu");
  eqInt(s.count(UiGestures::EV_SMART_ADJUST), 1, "  ...just the one smart adjust");
}

static void test_plus_joining_a_charge_hands_over() {
  section("15. Pressing + mid-charge cancels the menu and hands over to smart adjust");
  Sim s;
  s.hold(s.minus, 1600);                   // charging
  ok(s.g.arming(), "− is charging the menu", "not arming");
  s.plus = true;
  s.advance(120);
  eqInt(s.count(UiGestures::EV_ARM_CANCEL), 1, "+ joining cancels the charge");
  ok(!s.g.arming(), "  ...and the meter is gone", "still arming");
  s.advance(1200);
  eqInt(s.count(UiGestures::EV_SMART_ADJUST), 1, "  ...and 1 s later it is smart adjust");
  eqInt(s.count(UiGestures::EV_MENU_OPEN), 0, "  ...the menu never opened");
}

// 16. THE ONE THAT NEARLY SHIPPED (collision 4 in ui_gestures.h).
static void test_plus_then_minus_then_release_plus() {
  section("16. + then − then release + does NOT fling the menu open");
  Sim s;
  s.plus = true;  s.advance(100);
  s.minus = true; s.advance(400);          // both down, but under the 1 s smart-adjust
  eqInt(s.count(UiGestures::EV_SMART_ADJUST), 0, "a short two-finger fidget does nothing");
  s.plus = false; s.advance(200);          // ...now let go of + only
  eqInt(s.count(UiGestures::EV_MENU_OPEN), 0,
        "releasing + does NOT open the menu, even though − has been down 700 ms");
  s.advance(900);
  eqInt(s.count(UiGestures::EV_ARM_START), 1,
        "  ...the charge clock restarts from the moment − became the only button");
  s.advance(2200);
  eqInt(s.count(UiGestures::EV_MENU_OPEN), 1,
        "  ...so a full 3 s hold from THAT moment is what opens it");
}

// 16b. And once a gesture HAS fired, nothing else may until both are released.
static void test_swallow_until_both_released() {
  section("16b. After a gesture fires, both buttons must be released first");
  Sim s;
  s.plus = true; s.minus = true;
  s.advance(1200);
  eqInt(s.count(UiGestures::EV_SMART_ADJUST), 1, "both held 1 s: smart adjust fired");
  s.plus = false;
  s.advance(5000);                         // − still held, for another 5 s
  eqInt(s.count(UiGestures::EV_MENU_OPEN), 0,
        "holding − on after it does NOT open the menu — let go of both first");
  eqInt(s.count(UiGestures::EV_ARM_START), 0, "  ...it does not even arm");
  s.minus = false; s.advance(200);         // both up: the machine is live again
  s.hold(s.minus, 3100);
  eqInt(s.count(UiGestures::EV_MENU_OPEN), 1, "  ...and now a clean 3 s hold works");
}

// 17. Bounce, and the loop rate the AVR actually runs at.
static void test_contact_bounce_is_ignored() {
  section("17. Contact bounce is not a gesture");
  Sim s;
  for (int i = 0; i < 6; i++) {            // 6 bounces of 5 ms each
    s.hold(s.minus, 5);
    s.off(s.minus, 5);
  }
  s.advance(200);
  eqInt((long) s.seen.size(), 0, "a 60 ms burst of 5 ms bounces emits nothing");
}

static void test_slow_loop_rate() {
  section("18. It behaves the same at the rate loop() really runs (7 ms)");
  Sim s(5, 7);                             // 7 ms per loop: 7 keys x 4 analogReads
  s.hold(s.minus, 3108); s.off(s.minus, 301);
  eqInt(s.count(UiGestures::EV_MENU_OPEN), 1, "the menu still opens on a 7 ms loop");
  s.tapPlus(91, 119); eqInt((long) s.g.item(), 1, "  ...and the wheel still turns");
  s.plus = true; s.advance(35); s.minus = true; s.advance(147);
  eqInt(s.count(UiGestures::EV_MENU_ACCEPT), 1, "  ...and both-press still accepts");
}

// 19. Life goes on afterwards: the knob works again once the menu is done.
static void test_knob_works_after_menu() {
  section("19. The sensitivity knob comes straight back after the menu");
  Sim s;
  s.hold(s.minus, 3100); s.off(s.minus, 300);
  s.plus = true; s.advance(60); s.minus = true; s.advance(150);   // accept
  s.plus = false; s.minus = false; s.advance(300);
  size_t before = s.seen.size();
  s.tapMinus();
  eqInt((long) (s.seen.size() - before), 1, "one tap on −, one event");
  ok(s.seen.back() == UiGestures::EV_NUDGE_LESS, "  ...and it is the knob again",
     std::string("got ") + evName(s.seen.back()));
}

int main() {
  printf("\n\033[1mLemon Piano V5.5 — button gesture state machine\033[0m\n");
  printf("arm %u ms · open %u ms · both %u ms · debounce %u ms · idle %lu ms\n",
         (unsigned) GES_MENU_ARM_MS, (unsigned) GES_MENU_HOLD_MS,
         (unsigned) GES_BOTH_HOLD_MS, (unsigned) GES_DEBOUNCE_MS,
         (unsigned long) GES_MENU_IDLE_MS);

  test_tap_each_button();
  test_plus_ramps_forever();
  test_minus_ramp_stops_when_arming();
  test_minus_release_cancels_arming();
  test_minus_opens_menu();
  test_arm_meter_fills();
  test_wheel_wraps_both_ways();
  test_navigation_is_on_release();
  test_accept_both_buttons();
  test_accept_after_first_button_waits();
  test_menu_long_press_cancels();
  test_menu_idle_timeout();
  test_smart_adjust_still_works();
  test_holding_both_never_opens_menu();
  test_plus_joining_a_charge_hands_over();
  test_plus_then_minus_then_release_plus();
  test_swallow_until_both_released();
  test_contact_bounce_is_ignored();
  test_slow_loop_rate();
  test_knob_works_after_menu();

  printf("\n%d checks, \033[%sm%d failed\033[0m\n\n",
         checks, failures ? "31" : "32", failures);
  return failures ? 1 : 0;
}
