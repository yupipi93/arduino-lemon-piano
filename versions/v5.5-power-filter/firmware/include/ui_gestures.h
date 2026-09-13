/* ui_gestures.h — the two buttons, as one testable state machine.
 *
 * V5.5 grew a LEVEL MENU (2026-09-09) on the same two buttons that already
 * carried the sensitivity knob and the smart-adjust gesture. Three gestures
 * sharing two buttons is exactly where a piano starts doing the wrong thing —
 * so the decision layer lives HERE, as plain C++ with no Arduino in it, and is
 * driven through every overlapping timeline by ../test/ui_gestures_test.cpp on
 * the host. main.cpp only maps the events it emits onto sound and light.
 *
 * INPUT  : the two raw button levels + millis(), once per loop().
 * OUTPUT : at most one Event per call.
 *
 * ── The complete gesture map ────────────────────────────────────────────────
 *
 * THE TWO BUTTONS ARE SYMMETRIC (2026-09-13). A tap is the sensitivity knob, a
 * hold is a mode change, and which mode depends on which button. There is no
 * ramp on either: holding a button is a gesture of its own now, so a hold must
 * not also be dozens of taps. Sergio asked for exactly this -- "tienen que ser
 * pulsaciones individuales para subir y bajar la sensibilidad".
 *
 * While PLAYING (game or free play):
 *
 *   tap +                      EV_NUDGE_MORE      more sensitive, ONE step
 *   tap −                      EV_NUDGE_LESS      less sensitive, ONE step
 *   hold either at 1000 ms     EV_ARM_START       the arming meter appears
 *   hold either 1000 … 3000    EV_ARM_TICK        every 500 ms, rising pitch
 *   release while arming       EV_ARM_CANCEL      nothing changed
 *   hold + at 3000 ms          EV_TOGGLE_FREEPLAY free play <-> the level
 *   hold − at 3000 ms          EV_MENU_OPEN       the level wheel
 *   press the other mid-arm    EV_ARM_CANCEL      …and the both-hold takes over
 *   hold BOTH 1000 ms          EV_SMART_ADJUST    unchanged since 2026-07-28
 *
 * In the MENU (four levels — free play left the wheel on 2026-09-13):
 *
 *   tap +                      EV_MENU_NEXT      next item, wraps
 *   tap −                      EV_MENU_PREV      previous item, wraps
 *   press BOTH                 EV_MENU_ACCEPT    (either order, any overlap)
 *   hold either 3000 ms        EV_MENU_CANCEL    escape hatch, changes nothing
 *   20 s with no button        EV_MENU_CANCEL    idle timeout
 *
 * ── The four collisions, and how each is resolved ───────────────────────────
 *
 * 1. "hold a button" vs "that same press was also a tap". The nudge fires on
 *    PRESS, because a knob that waits for the release feels broken. So every
 *    hold gesture starts by moving the sensitivity one step. main.cpp undoes
 *    it: it snapshots the margin as each button goes down and restores that
 *    snapshot when the hold fires (marginBeforePlus / marginBeforeMinus), so
 *    changing mode never leaves the sensitivity somewhere else. Before
 *    2026-09-13 the ramp made this a much bigger correction than one step.
 *
 * 2. "press both" (accept) vs "tap one" (navigate). In the menu, navigation
 *    fires on RELEASE, never on press. So the first button of a two-button
 *    press cannot navigate before the second one arrives: an accept can never
 *    be preceded by a phantom step, and the two buttons need no coincidence
 *    window at all — the first simply waits.
 *
 * 3. "press both" (accept) vs "hold both 1 s" (smart adjust). They live in
 *    different states: smart adjust exists only while playing, accept only in
 *    the menu. Holding one button for 3 s never emits EV_SMART_ADJUST and
 *    holding both never emits EV_MENU_OPEN or EV_TOGGLE_FREEPLAY — asserted as
 *    its own test case.
 *
 * 4. THE ONE THAT NEARLY SHIPPED: press +, then − on top of it, then let go of
 *    + while still holding −. The mode-change clock cannot be "when − went
 *    down" or that release would open the menu on the spot, with no arming
 *    meter and no warning — a two-finger fidget turning into a mode change. The
 *    clock is aloneSince_: it starts when a button becomes the ONLY one down,
 *    from whatever combination came before. So that release begins a fresh 3 s
 *    hold, arming meter and all. Since + grew a hold of its own on 2026-09-13
 *    the same clock, and the same test, cover it in both directions.
 *
 * After any state-changing event the machine SWALLOWS both buttons until they
 * are released together, so the release that ends a gesture can never start
 * the next one.
 */
#ifndef UI_GESTURES_H
#define UI_GESTURES_H

#include <stdint.h>

// ── Timings. The host test compiles this same header, so these ARE the numbers
// under test; changing one here changes what the test proves. ───────────────
static const uint16_t GES_DEBOUNCE_MS      = 40;     // both buttons
static const uint16_t GES_BOTH_HOLD_MS     = 1000;   // both -> smart adjust
static const uint16_t GES_HOLD_ARM_MS      = 1000;   // either alone: the meter starts
static const uint16_t GES_HOLD_FIRE_MS     = 3000;   // either alone: the mode changes
static const uint16_t GES_MENU_TICK_MS     = 500;    // arming feedback cadence
static const uint16_t GES_MENU_CANCEL_MS   = 3000;   // in-menu long press = escape
static const uint32_t GES_MENU_IDLE_MS     = 20000;  // in-menu inactivity = escape

class UiGestures {
 public:
  enum Event : uint8_t {
    EV_NONE = 0,
    EV_NUDGE_MORE,     // + : one step more sensitive
    EV_NUDGE_LESS,     // − : one step less sensitive
    EV_SMART_ADJUST,   // both held 1 s while playing
    EV_ARM_START,      // a button crossed 1 s: show the arming meter
    EV_ARM_TICK,       // ...and one of these every 500 ms until it fires
    EV_ARM_CANCEL,     // released, or the other button joined in, before 3 s
    EV_TOGGLE_FREEPLAY,// + held 3 s: free play <-> the level that was playing
    EV_MENU_OPEN,      // − held 3 s: the level wheel
    EV_MENU_NEXT,
    EV_MENU_PREV,
    EV_MENU_ACCEPT,
    EV_MENU_CANCEL
  };

  // itemCount = how many entries the wheel has (the four levels; free play left
  // the wheel on 2026-09-13 and lives on a + hold instead).
  void begin(uint8_t itemCount) {
    itemCount_ = itemCount ? itemCount : 1;
    item_ = 0;
    menu_ = false;
    arming_ = ARM_NONE;
    swallow_ = false;
    plusAlonePrev_ = minusAlonePrev_ = false;
    bothSince_ = 0;
    plus_.reset();
    minus_.reset();
  }

  // Call once per loop() with the two RAW button levels (true = pressed).
  Event update(bool plusDown, bool minusDown, uint32_t now) {
    plus_.feed(plusDown, now);
    minus_.feed(minusDown, now);
    const bool p = plus_.level;
    const bool m = minus_.level;

    // Every press stamps its own clock HERE, before any branch can read it. A
    // branch that inherits a stale timestamp fires its gesture instantly (see
    // collision 4), and that is the whole family of bugs this line prevents.
    if (plus_.rose)  plusSince_  = now;
    if (minus_.rose) minusSince_ = now;

    // ...and a mode-change clock is not "when that button went down" but "when
    // it became the only button down", for the same reason. Both buttons carry
    // a hold gesture now, so both need the clock (see collision 4).
    const bool plusAlone  = p && !m;
    const bool minusAlone = m && !p;
    if (plusAlone  && !plusAlonePrev_)  { plusAloneSince_  = now; arming_ = ARM_NONE; }
    if (minusAlone && !minusAlonePrev_) { minusAloneSince_ = now; arming_ = ARM_NONE; }
    plusAlonePrev_  = plusAlone;
    minusAlonePrev_ = minusAlone;

    // A gesture has fired: ignore everything until the player lets go of both,
    // so the release that ends one gesture cannot begin another.
    if (swallow_) {
      if (!p && !m) swallow_ = false;
      bothSince_ = 0;
      menuIdleFrom_ = now;
      return EV_NONE;
    }

    return menu_ ? updateMenu(p, m, now) : updatePlay(p, m, now);
  }

  bool inMenu() const { return menu_; }
  uint8_t item() const { return item_; }

  // Where the wheel starts when the menu opens — main.cpp sets this to whatever
  // mode is currently running, so "open, accept" is always a no-op.
  void setItem(uint8_t i) { item_ = (uint8_t) (i % itemCount_); }

  // 0..100 while a hold is arming: how full the LED bar should be.
  uint8_t armPercent(uint32_t now) const {
    if (arming_ == ARM_NONE) return 0;
    const uint32_t since = (arming_ == ARM_PLUS) ? plusAloneSince_ : minusAloneSince_;
    const uint32_t held = (uint32_t) (now - since);
    if (held <= GES_HOLD_ARM_MS) return 0;
    const uint32_t span = (uint32_t) (GES_HOLD_FIRE_MS - GES_HOLD_ARM_MS);
    const uint32_t pct = ((held - GES_HOLD_ARM_MS) * 100u) / span;
    return (uint8_t) (pct > 100 ? 100 : pct);
  }
  bool arming() const { return arming_ != ARM_NONE; }
  // WHICH hold is charging — main.cpp says a different thing for each.
  bool armingPlus() const { return arming_ == ARM_PLUS; }

  // main.cpp calls this after anything BLOCKING (a preview melody, a fanfare):
  // millis() jumped, and an idle timeout must not count the music as idleness.
  void noteActivity(uint32_t now) { menuIdleFrom_ = now; }

 private:
  struct Debounced {
    bool level, raw, rose, fell;
    uint32_t changedAt;
    void reset() { level = raw = rose = fell = false; changedAt = 0; }
    void feed(bool v, uint32_t now) {
      rose = fell = false;
      if (v != raw) { raw = v; changedAt = now; }
      if (v != level && (uint32_t) (now - changedAt) >= GES_DEBOUNCE_MS) {
        level = v;
        if (v) rose = true; else fell = true;
      }
    }
  };

  Event updatePlay(bool p, bool m, uint32_t now) {
    // ── BOTH DOWN: smart adjust (and it cancels any arming in progress) ──────
    if (p && m) {
      if (arming_ != ARM_NONE) { arming_ = ARM_NONE; bothSince_ = now; return EV_ARM_CANCEL; }
      if (bothSince_ == 0) bothSince_ = now;
      else if ((uint32_t) (now - bothSince_) >= GES_BOTH_HOLD_MS) {
        swallow_ = true;
        bothSince_ = 0;
        return EV_SMART_ADJUST;
      }
      return EV_NONE;
    }
    bothSince_ = 0;

    // ── let go mid-arming: nothing happened, say so ──────────────────────────
    if ((minus_.fell && arming_ == ARM_MINUS) || (plus_.fell && arming_ == ARM_PLUS)) {
      arming_ = ARM_NONE;
      return EV_ARM_CANCEL;
    }

    // ── first press of either button: ONE step, immediately, and that is all
    // this press will ever do to the knob. No ramp — holding is a gesture now,
    // and main.cpp puts this step back if the hold turns into a mode change. ──
    if (plus_.rose)  return EV_NUDGE_MORE;
    if (minus_.rose) return EV_NUDGE_LESS;

    // ── one button held alone: ARM, then FIRE ───────────────────────────────
    // Identical timing for both; only the event at the end differs. + swaps the
    // instrument for the game, − opens the level wheel.
    if (p != m) {
      const bool isPlus  = p;
      const Arming which = isPlus ? ARM_PLUS : ARM_MINUS;
      const uint32_t held =
          (uint32_t) (now - (isPlus ? plusAloneSince_ : minusAloneSince_));

      if (held >= GES_HOLD_FIRE_MS) {
        arming_ = ARM_NONE;
        swallow_ = true;             // the button is still down; its release
        if (isPlus) return EV_TOGGLE_FREEPLAY;   // must not start the next gesture
        menu_ = true;
        menuIdleFrom_ = now;
        return EV_MENU_OPEN;
      }
      if (held >= GES_HOLD_ARM_MS) {
        if (arming_ != which) {
          arming_ = which;
          nextArmTick_ = now + GES_MENU_TICK_MS;
          return EV_ARM_START;
        }
        if ((int32_t) (now - nextArmTick_) >= 0) {
          nextArmTick_ = now + GES_MENU_TICK_MS;
          return EV_ARM_TICK;
        }
      }
      return EV_NONE;
    }

    return EV_NONE;
  }

  Event updateMenu(bool p, bool m, uint32_t now) {
    // ── BOTH DOWN = ACCEPT. No coincidence window is needed: navigation fires
    // on release, so the button that arrived first has not done anything yet.
    if (p && m) {
      menu_ = false;
      swallow_ = true;
      return EV_MENU_ACCEPT;
    }

    // ── a long press on EITHER button leaves the menu, changing nothing ─────
    if (p && (uint32_t) (now - plusSince_) >= GES_MENU_CANCEL_MS) {
      menu_ = false; swallow_ = true; return EV_MENU_CANCEL;
    }
    if (m && (uint32_t) (now - minusSince_) >= GES_MENU_CANCEL_MS) {
      menu_ = false; swallow_ = true; return EV_MENU_CANCEL;
    }

    if (plus_.rose || minus_.rose) { menuIdleFrom_ = now; return EV_NONE; }

    // ── navigation, on RELEASE (see collision 2 at the top of this file) ────
    if (plus_.fell) {
      menuIdleFrom_ = now;
      if ((uint32_t) (now - plusSince_) < GES_MENU_CANCEL_MS) {
        item_ = (uint8_t) ((item_ + 1) % itemCount_);
        return EV_MENU_NEXT;
      }
      return EV_NONE;
    }
    if (minus_.fell) {
      menuIdleFrom_ = now;
      if ((uint32_t) (now - minusSince_) < GES_MENU_CANCEL_MS) {
        item_ = (uint8_t) ((item_ + itemCount_ - 1) % itemCount_);
        return EV_MENU_PREV;
      }
      return EV_NONE;
    }

    if (!p && !m && (uint32_t) (now - menuIdleFrom_) >= GES_MENU_IDLE_MS) {
      menu_ = false;
      return EV_MENU_CANCEL;
    }
    return EV_NONE;
  }

  enum Arming : uint8_t { ARM_NONE = 0, ARM_PLUS, ARM_MINUS };

  Debounced plus_, minus_;
  uint8_t itemCount_ = 1;
  uint8_t item_ = 0;
  bool menu_ = false;
  Arming arming_ = ARM_NONE;
  bool swallow_ = false;
  bool plusAlonePrev_ = false, minusAlonePrev_ = false;
  uint32_t plusSince_ = 0, minusSince_ = 0;
  uint32_t plusAloneSince_ = 0, minusAloneSince_ = 0;
  uint32_t bothSince_ = 0;
  uint32_t nextArmTick_ = 0;
  uint32_t menuIdleFrom_ = 0;
};

#endif  // UI_GESTURES_H
