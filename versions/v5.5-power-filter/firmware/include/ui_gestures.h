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
 * While PLAYING (game or free play):
 *
 *   tap +                      EV_NUDGE_MORE     more sensitive, one step
 *   hold + (>= 400 ms)         EV_NUDGE_MORE …   ramps every 120 ms, forever
 *   tap −                      EV_NUDGE_LESS     less sensitive, one step
 *   hold − 400 … 1000 ms       EV_NUDGE_LESS …   ramps, same as +
 *   hold − at 1000 ms          EV_ARM_START      ramp STOPS, the menu arms
 *   hold − 1000 … 3000 ms      EV_ARM_TICK       every 500 ms, rising pitch
 *   release − while arming     EV_ARM_CANCEL     nothing changed
 *   hold − at 3000 ms          EV_MENU_OPEN      the menu opens
 *   press + while − is arming  EV_ARM_CANCEL     …and the both-hold takes over
 *   hold BOTH 1000 ms          EV_SMART_ADJUST   unchanged since 2026-07-28
 *
 * In the MENU:
 *
 *   tap +                      EV_MENU_NEXT      next item, wraps
 *   tap −                      EV_MENU_PREV      previous item, wraps
 *   press BOTH                 EV_MENU_ACCEPT    (either order, any overlap)
 *   hold either 3000 ms        EV_MENU_CANCEL    escape hatch, changes nothing
 *   20 s with no button        EV_MENU_CANCEL    idle timeout
 *
 * ── The four collisions, and how each is resolved ───────────────────────────
 *
 * 1. "hold − 3 s" vs "hold − to ramp the margin down". The ramp is CAPPED at
 *    GES_MENU_ARM_MS: past one second the button is arming the menu and no
 *    longer touching the margin. main.cpp additionally restores the margin to
 *    its value at press time when the menu opens, so opening the menu never
 *    leaves the sensitivity somewhere else (see marginBeforeHold there).
 *
 * 2. "press both" (accept) vs "tap one" (navigate). In the menu, navigation
 *    fires on RELEASE, never on press. So the first button of a two-button
 *    press cannot navigate before the second one arrives: an accept can never
 *    be preceded by a phantom step, and the two buttons need no coincidence
 *    window at all — the first simply waits.
 *
 * 3. "press both" (accept) vs "hold both 1 s" (smart adjust). They live in
 *    different states: smart adjust exists only while playing, accept only in
 *    the menu. Holding − for 3 s never emits EV_SMART_ADJUST and holding both
 *    never emits EV_MENU_OPEN — asserted as its own test case.
 *
 * 4. THE ONE THAT NEARLY SHIPPED: press +, then − on top of it, then let go of
 *    + while still holding −. The menu-open clock cannot be "when − went down"
 *    or that release would open the menu on the spot, with no arming meter and
 *    no warning — a two-finger fidget turning into a mode change. The clock is
 *    minusAloneSince_: it starts when − becomes the ONLY button down, from
 *    whatever combination came before. So that release begins a fresh 3 s hold,
 *    arming meter and all. Test: "plus then minus then release plus".
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
static const uint16_t GES_REPEAT_DELAY_MS  = 400;    // hold this long to ramp
static const uint16_t GES_REPEAT_EVERY_MS  = 120;    // ...then one step this often
static const uint16_t GES_BOTH_HOLD_MS     = 1000;   // both -> smart adjust
static const uint16_t GES_MENU_ARM_MS      = 1000;   // − alone: ramp stops, arms
static const uint16_t GES_MENU_HOLD_MS     = 3000;   // − alone: menu opens
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
    EV_ARM_START,      // − crossed 1 s: show the arming meter, stop ramping
    EV_ARM_TICK,       // ...and one of these every 500 ms until it opens
    EV_ARM_CANCEL,     // released, or + joined in, before 3 s
    EV_MENU_OPEN,
    EV_MENU_NEXT,
    EV_MENU_PREV,
    EV_MENU_ACCEPT,
    EV_MENU_CANCEL
  };

  // itemCount = how many entries the wheel has (4 levels + free play = 5).
  void begin(uint8_t itemCount) {
    itemCount_ = itemCount ? itemCount : 1;
    item_ = 0;
    menu_ = false;
    arming_ = false;
    swallow_ = false;
    minusAlonePrev_ = false;
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

    // ...and the menu-open clock is not "when − went down" but "when − became
    // the only button down", for the same reason.
    const bool minusAlone = m && !p;
    if (minusAlone && !minusAlonePrev_) {
      minusAloneSince_ = now;
      arming_ = false;
    }
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

  // 0..100 while the menu is arming: how full the LED bar should be.
  uint8_t armPercent(uint32_t now) const {
    if (!arming_) return 0;
    const uint32_t held = (uint32_t) (now - minusAloneSince_);
    if (held <= GES_MENU_ARM_MS) return 0;
    const uint32_t span = (uint32_t) (GES_MENU_HOLD_MS - GES_MENU_ARM_MS);
    const uint32_t pct = ((held - GES_MENU_ARM_MS) * 100u) / span;
    return (uint8_t) (pct > 100 ? 100 : pct);
  }
  bool arming() const { return arming_; }

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
      if (arming_) { arming_ = false; bothSince_ = now; return EV_ARM_CANCEL; }
      if (bothSince_ == 0) bothSince_ = now;
      else if ((uint32_t) (now - bothSince_) >= GES_BOTH_HOLD_MS) {
        swallow_ = true;
        bothSince_ = 0;
        return EV_SMART_ADJUST;
      }
      return EV_NONE;
    }
    bothSince_ = 0;

    // ── − let go mid-arming: nothing happened, say so ────────────────────────
    if (minus_.fell && arming_) { arming_ = false; return EV_ARM_CANCEL; }

    // ── first press of either button: one step, immediately ─────────────────
    if (plus_.rose)  { plusRepeat_  = now + GES_REPEAT_DELAY_MS; return EV_NUDGE_MORE; }
    if (minus_.rose) { minusRepeat_ = now + GES_REPEAT_DELAY_MS; return EV_NUDGE_LESS; }

    // ── + held alone: ramp, forever. + has no long-press meaning. ───────────
    if (p && !m) {
      if ((int32_t) (now - plusRepeat_) >= 0) {
        plusRepeat_ = now + GES_REPEAT_EVERY_MS;
        return EV_NUDGE_MORE;
      }
      return EV_NONE;
    }

    // ── − held alone: ramp, then ARM, then OPEN ─────────────────────────────
    if (m && !p) {
      const uint32_t held = (uint32_t) (now - minusAloneSince_);
      if (held >= GES_MENU_HOLD_MS) {
        arming_ = false;
        menu_ = true;
        swallow_ = true;               // − is still down; don't let its release
        menuIdleFrom_ = now;           // count as the first navigation tap
        return EV_MENU_OPEN;
      }
      if (held >= GES_MENU_ARM_MS) {
        if (!arming_) {                // the ramp ends here, on purpose
          arming_ = true;
          nextArmTick_ = now + GES_MENU_TICK_MS;
          return EV_ARM_START;
        }
        if ((int32_t) (now - nextArmTick_) >= 0) {
          nextArmTick_ = now + GES_MENU_TICK_MS;
          return EV_ARM_TICK;
        }
        return EV_NONE;
      }
      if ((int32_t) (now - minusRepeat_) >= 0) {
        minusRepeat_ = now + GES_REPEAT_EVERY_MS;
        return EV_NUDGE_LESS;
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

  Debounced plus_, minus_;
  uint8_t itemCount_ = 1;
  uint8_t item_ = 0;
  bool menu_ = false;
  bool arming_ = false;
  bool swallow_ = false;
  bool minusAlonePrev_ = false;
  uint32_t plusSince_ = 0, minusSince_ = 0, minusAloneSince_ = 0;
  uint32_t plusRepeat_ = 0, minusRepeat_ = 0;
  uint32_t bothSince_ = 0;
  uint32_t nextArmTick_ = 0;
  uint32_t menuIdleFrom_ = 0;
};

#endif  // UI_GESTURES_H
