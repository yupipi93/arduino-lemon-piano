"""Generate ONE wiring diagram per hardware revision — declaratively.

Every version under `versions/` owns a rendered diagram of *its* hardware; this
file is the **contract** for each of them: which components exist, where they
sit, and what connects to what. All the hard work — routing orthogonal wires
that never cross a component body, never run coincident, never leave a pin
unconnected, and keep their distance — is done by the reusable **wirewright**
engine (../eda-wirewright/), which DRC-validates every diagram before saving.

Add a new hardware revision → add a builder here and list it in `TARGETS`
(see docs/VERSIONING.md for the whole checklist).

Two ways to use wirewright (this file uses the LOCAL Python API):
  • LOCAL  — the shim below, or `pip install ../eda-wirewright` + the CLI
             (`wirewright render circuit.json -o out.png`).
  • CLOUD  — public API (best for agents), POST a JSON contract → PNG:
             curl -X POST https://wirewright.scv.multitecua.com/render \\
                  -H 'Content-Type: application/json' -d @circuit.json -o out.png
             Discover parts: GET /components · docs: github.com/yupipi93/eda-wirewright

Run:  python3 tools/wiring_diagrams.py            # all versions
      python3 tools/wiring_diagrams.py v3 v4.5    # only these
Out:  versions/<version-dir>/images/wiring-<version>.png
"""
import sys
from pathlib import Path

# Consume the standalone wirewright engine from the sibling repo without an
# install (it only needs Pillow). If you `pip install wirewright`, drop this.
_WW = Path(__file__).resolve().parents[2] / "eda-wirewright" / "src"
if _WW.is_dir():
    sys.path.insert(0, str(_WW))

from wirewright import Schematic, lib, P, R, Rail, build, save, C, deco

ROOT = Path(__file__).resolve().parent.parent
VERSIONS = ROOT / "versions"

W = 3400
H, RAIL_GND_Y = 1580, 1150            # 2026 revisions (V4, V4.5, V5)
H_2019, GND_2019 = 1720, 1290         # 2019 rigs: the pull-up comb needs a taller canvas
RAIL_5V_Y = 360
RAIL_X0, RAIL_X1 = 130, W - 110
NANO_X, NANO_Y = 1150, 430          # board origin (left-top)
NANO_W_ = lib.NANO_W
KEY_X = 300                          # fruit/lemon column


def _legend_box(gnd_y, w=W, h=330):
    return dict(x=130, y=gnd_y + 60, w=w - 260, h=h)


# ── shared skeleton: rails + ATmega328P board ────────────────────────────────
def _board(title, subtitle, board, h=H, gnd_y=RAIL_GND_Y, w=W,
           bx=NANO_X, by=NANO_Y, v5_x0=RAIL_X0, v5_x1=None):
    """v5_x0/v5_x1 trim the +5 V rail's span. Default = full width, like every
    revision up to V5.5. V6 shortens it on purpose: the filtered rail must NOT
    reach the power-source block west of the filter, nor the audio amplifier
    east of it — those two hang off the *unfiltered* 5 V, and a rail that
    stopped short is the honest way to draw that."""
    s = Schematic(w, h, title=title, subtitle=subtitle)
    s.add_rail(Rail("5V", y=RAIL_5V_Y, x0=v5_x0, x1=v5_x1 or (w - 110),
                    color=C["v5"], label="+5 V"))
    s.add_rail(Rail("GND", y=gnd_y, x0=RAIL_X0, x1=w - 110, color=C["gnd"], label="GND"))
    s.add(lib.arduino_nano("U1", bx, by, board=board))
    s.connect("nano5v", C["v5"], P("U1", "5V"), R("5V"))
    s.connect("nanognd", C["gnd"], P("U1", "GND"), R("GND"))
    return s


# ── keyboard, 2019 wiring (player holds GND, pins biased HIGH) ───────────────
def keyboard_2019(s, fruit="banana", dx=0):
    """Each analog pin is pulled UP to +5 V through 220 Ω and reads ~1023 idle;
    the fruit sits directly on the pin, so touching it drags the pin down
    through the player's body to the hand-held GND clip (`analogRead <= 1019`).

    Layout: one horizontal run per key (fruit → pin, straight east) and the
    pull-up comb hanging above it in a staircase, so no run has to detour.

    `dx` shifts the whole keyboard east. V6 needs it: the battery + power-bank
    module take the top-left corner that used to be the filter's, so the filter
    and the board it feeds both move east by the same amount and the left→right
    power flow stays intact. dx=0 reproduces V1..V5.5 byte-for-byte."""
    s.add(lib.clip_box("CLIP", 130 + dx, 1130, w=430, title="hand-held GND clip",
                       sub=f"one hand on the clip, one on the {fruit} = the player"))
    s.connect("clipgnd", C["gnd"], P("CLIP", "out"), R("GND"))

    key_ys = [470 + i * 100 for i in range(7)]
    order = [6, 5, 4, 3, 2, 1, 0]                    # analog index per row
    PULLUP_Y = 420                                   # comb band, just under the +5 V rail
    for row, ai in enumerate(order):
        ky = key_ys[row]
        s.add(lib.lemon_key(f"K{ai}", KEY_X + dx, ky, ai + 1, f"A{ai}"))
        rx = 470 + dx + row * 100                   # own channel per key in the pull-up comb
        s.add(lib.resistor(f"RK{ai}", rx, PULLUP_Y, orient="V", length=50))
        # ONE node: the pin, the fruit clip and the pull-up's low side.
        s.connect(f"kn{ai}", C["key"], P("U1", f"A{ai}"), P(f"K{ai}", "clip"), P(f"RK{ai}", "b"))
        s.connect(f"kp{ai}", C["v5"], P(f"RK{ai}", "a"), R("5V"))

    s.decorations.append(deco.dashed_arrow(KEY_X + dx + 780, key_ys[-1] + 90, KEY_X + dx + 360,
                                           key_ys[-1] + 90, C["key"],
                                           label=f"GND clip → body → {fruit} → the pin drops"))
    return s


# ── keyboard, 2026 wiring (player holds 5 V, pins float LOW) ─────────────────
def keyboard_2026(s):
    """The clip flipped to +5 V in V4: each pin floats near 0 and RISES when the
    player closes 5 V → body → lemon → 220 Ω → pin."""
    s.add(lib.clip_box("CLIP", 150, 150))
    s.connect("clip5v", C["v5"], P("CLIP", "out"), R("5V"))

    key_ys = [312 + i * 94 for i in range(7)]
    order = [6, 5, 4, 3, 2, 1, 0]
    for row, ai in enumerate(order):
        ky = key_ys[row]
        s.add(lib.lemon_key(f"K{ai}", KEY_X, ky, ai + 1, f"A{ai}"))
        s.add(lib.resistor(f"RK{ai}", KEY_X + 150, ky, orient="H"))
        s.connect(f"kr{ai}", C["key"], P(f"K{ai}", "clip"), P(f"RK{ai}", "a"))
        s.connect(f"ka{ai}", C["key"], P(f"RK{ai}", "b"), P("U1", f"A{ai}"))

    s.decorations.append(deco.dashed_arrow(300, 242, 300, key_ys[0] - 36, C["key"],
                                           label="touch → body → lemon"))
    return s


def _button(s, cid, x, y, pin, label, sub, color, cap):
    """Active-HIGH button: pin → button → 5 V, plus a 10 kΩ pulldown to GND on the
    pin side (its own component, so the DRC proves it is connected). The board pin,
    the button's pin terminal and the pulldown top are ONE electrical node."""
    s.add(lib.push_button(cid, x, y, label, sub, cap=cap))
    s.add(lib.resistor(f"{cid}PD", x - 90, y + 30, orient="V", label="10k"))
    s.connect(f"{cid}sig", color, P("U1", pin), P(cid, "pin"), P(f"{cid}PD", "a"))
    s.connect(f"{cid}v5", C["v5"], P(cid, "v5"), R("5V"))
    s.connect(f"{cid}pg", C["gnd"], P(f"{cid}PD", "b"), R("GND"))


def _button_to_gnd(s, cid, x, y, pin, label, sub, color, cap):
    """Active-LOW button: pin -> button -> GND, relying on the AVR's INTERNAL
    pull-up, so unlike _button() there is no external resistor to draw. Used by
    the V2.5 bench rig, where fewer parts on the breadboard is the point."""
    s.add(lib.push_button(cid, x, y, label, sub, cap=cap))
    s.connect(f"{cid}sig", color, P("U1", pin), P(cid, "pin"))
    # push_button's far terminal is named "v5" by the library; here it goes to
    # GND, which is what makes the button active-low.
    s.connect(f"{cid}gnd", C["gnd"], P(cid, "v5"), R("GND"))


def _buzzer(s, x, y, label="passive buzzer", pin="D8"):
    s.add(lib.buzzer("BUZ", x, y, label=label, pin_label=pin))
    s.connect("buzsig", C["buzz"], P("U1", pin), P("BUZ", "sig"))
    s.connect("buzgnd", C["gnd"], P("BUZ", "gnd"), R("GND"))


BOARD_2019 = "ATmega328P · Uno in 2019 — key 7 (A6) needs a Nano / Pro Mini"
KEY_LEGEND_2019 = (C["key"], "Fruit keys (7)",
                   ["A0..A6 · 220 Ω pull-up to +5 V each",
                    "idle ≈ 1023 · touch drags the pin down (≤ 1019)"])
RAIL_LEGEND = (C["v5"], "+5 V / GND rails", ["board 5V/GND · pull-ups · player's clip"])
NOTE_2019_BOARD = ("Drawn with the Nano-style ATmega328P pinout so every revision "
                   "can be compared pin-for-pin; the 2019 rig was an Uno, whose DIP "
                   "package has no A6 — key 7 only works on a Nano / Pro Mini.",
                   C["muted"])


# ── V0 — buzzer bring-up rig ─────────────────────────────────────────────────
W_V0, H_V0, GND_V0 = 2200, 1500, 1060


def build_v0():
    s = _board("Lemon Piano V0 — buzzer bring-up rig",
               "One passive buzzer on D8 and nothing else: no keys, no game, no "
               "touch sensing. The firmware plays a C-major scale forever.",
               "ATmega328P · any Nano / Uno (no A6/A7 needed)",
               w=W_V0, h=H_V0, gnd_y=GND_V0, bx=760, by=400)

    s.add(lib.buzzer("BUZ", 1560, 560))
    s.connect("d8", C["buzz"], P("U1", "D8"), P("BUZ", "sig"))
    s.connect("bg", C["gnd"], P("BUZ", "gnd"), R("GND"))

    entries = [
        (C["buzz"], "Passive buzzer", ["D8 → buzzer + · buzzer − → GND",
                                       "the same pin every other version uses"]),
        (C["v5"], "+5 V / GND rails", ["board 5V/GND only — nothing else draws power"]),
    ]
    notes = [("D13 (the board's ON-BOARD LED) lights for the duration of every note — "
              "no external part. LED stepping through the scale but no sound = the "
              "firmware is fine and the fault is the buzzer, its wiring or the pin.",
              C["muted"]),
             ("An ACTIVE buzzer ignores the frequency and only clicks; this project "
              "assumes a PASSIVE one. Polarity matters on most modules: + to D8.",
              C["muted"])]
    s.decorations.append(deco.legend(entries=entries, notes=notes,
                                     **_legend_box(GND_V0, w=W_V0, h=260)))
    return s, "v0-buzzer", "wiring-v0.png"


# ── V1 — banana piano (untitled.es tutorial rig) ─────────────────────────────
def build_v1():
    s = _board("Lemon Piano V1 — banana piano (untitled.es tutorial rig, 2019)",
               "7 fruit touch keys · speaker on D8 · HC-SR04 ultrasonic mounted "
               "(its code is commented out) · no LEDs, no game.",
               BOARD_2019, h=H_2019, gnd_y=GND_2019)
    keyboard_2019(s)
    _buzzer(s, 1900, 560, label="speaker / buzzer")

    # HC-SR04 — physically on the rig, driven by the commented-out demo code.
    s.add(lib.ultrasonic("US", 2600, 900))
    s.connect("trig", C["ctrl"], P("U1", "D12"), P("US", "trig"))
    s.connect("echo", C["ctrl"], P("U1", "D11"), P("US", "echo"))
    s.connect("usv", C["v5"], P("US", "vcc"), R("5V"))
    s.connect("usg", C["gnd"], P("US", "gnd"), R("GND"))

    entries = [
        KEY_LEGEND_2019,
        RAIL_LEGEND,
        (C["buzz"], "Speaker", ["D8 · one fixed note per key (C3..B3)"]),
        (C["ctrl"], "HC-SR04 (leftover)", ["TRIG D12 · ECHO D11 · loop code commented out"]),
    ]
    notes = [("D13 blinks the board's ON-BOARD LED while a note plays — no external part.",
              C["muted"]), NOTE_2019_BOARD]
    s.decorations.append(deco.legend(entries=entries, notes=notes, **_legend_box(GND_2019)))
    return s, "v1-banana-piano", "wiring-v1.png"


# ── V2 — keyboard test (sensor stripped) ─────────────────────────────────────
def build_v2():
    s = _board("Lemon Piano V2 — keyboard test rig (2019)",
               "The keyboard alone: 7 fruit keys + speaker on D8. HC-SR04 removed; "
               "the sketch averages 4 samples per key and dumps A0/A1 readings.",
               BOARD_2019, h=H_2019, gnd_y=GND_2019)
    keyboard_2019(s)
    _buzzer(s, 1900, 560, label="speaker / buzzer")

    entries = [
        KEY_LEGEND_2019,
        RAIL_LEGEND,
        (C["buzz"], "Speaker", ["D8 · one fixed note per key (C3..B3)"]),
        (C["muted"], "Serial monitor", ["D0/D1 UART · prints A0/A1 for threshold hunting",
         "(Serial.begin is commented out in the sketch — line 97)"]),
    ]
    notes = [("Hardware delta vs V1: the HC-SR04 comes off the rig (D11/D12 free). "
              "Same keyboard, same speaker — this revision exists to measure the keys.",
              C["muted"]), NOTE_2019_BOARD]
    s.decorations.append(deco.legend(entries=entries, notes=notes, **_legend_box(GND_2019)))
    return s, "v2-keyboard-test", "wiring-v2.png"


# ── V2.5 — keyboard test + live threshold buttons ────────────────────────────
def build_v2_5():
    s = _board("Lemon Piano V2.5 — keyboard test with a live touch threshold",
               "V2's rig plus TWO BUTTONS that tune the touch threshold while it "
               "runs (D10 up / D11 down, to GND via the internal pull-ups) and a "
               "serial readout of every reading, the threshold and each note.",
               BOARD_2019, h=H_2019, gnd_y=GND_2019)
    keyboard_2019(s)
    _buzzer(s, 1900, 560, label="speaker / buzzer")

    _button_to_gnd(s, "TUP", 2450, 620, "D10", "THRESHOLD +", "less touch needed",
                   C["margin"], (60, 170, 90))
    _button_to_gnd(s, "TDN", 2450, 900, "D11", "THRESHOLD −", "more touch needed",
                   C["margin"], (200, 60, 60))

    entries = [
        KEY_LEGEND_2019,
        RAIL_LEGEND,
        (C["buzz"], "Speaker", ["D8 · one fixed note per key (C3..B3)"]),
        (C["margin"], "Threshold buttons",
         ["D10 = raise · D11 = lower · step 5, auto-repeat while held",
          "button straight to GND — internal pull-ups, no resistors"]),
        (C["muted"], "Serial monitor",
         ["9600 baud · live readout of A0..A6 vs the threshold",
          "plus every threshold change and every note played"]),
    ]
    notes = [("Hardware delta vs V2: two buttons, nothing else. V2's hardcoded "
              "`<= 1019` is now a variable you can dial in while touching the fruit "
              "— no edit-compile-reflash cycle to find a working threshold.",
              C["muted"]),
             ("These buttons are active-LOW (to GND) unlike V3/V4/V4.5's active-HIGH "
              "ones, so they need no 10 kΩ pulldown: fewer parts on a bench rig.",
              C["muted"]),
             NOTE_2019_BOARD]
    s.decorations.append(deco.legend(entries=entries, notes=notes,
                                     **_legend_box(GND_2019, h=380)))
    return s, "v2.5-threshold-buttons", "wiring-v2.5.png"


# ── V3 — game prototype (LEDs + game button + one relay) ─────────────────────
def build_v3():
    s = _board("Lemon Piano V3 — game prototype (2019)",
               "First secret-sequence game: red LED (D2) · green LED (D3) · "
               "game-select button (D4) · ONE relay (D5, code commented out) · speaker (D8).",
               BOARD_2019, h=H_2019, gnd_y=GND_2019)
    keyboard_2019(s)

    lx = 1700
    s.add(lib.led("LEDR", lx, 560, (215, 45, 45), "RED LED", "D2 · wrong", anode="W"))
    s.add(lib.resistor("RR", lx, 700, orient="V"))
    s.connect("d2", C["led"], P("U1", "D2"), P("LEDR", "anode"))
    s.connect("rc", C["gnd"], P("LEDR", "cathode"), P("RR", "a"))
    s.connect("rg", C["gnd"], P("RR", "b"), R("GND"))

    lx2 = 1900
    s.add(lib.led("LEDG", lx2, 560, (45, 185, 75), "GREEN LED", "D3 · right", anode="W"))
    s.add(lib.resistor("RG", lx2, 700, orient="V"))
    s.connect("d3", C["led"], P("U1", "D3"), P("LEDG", "anode"))
    s.connect("gc", C["gnd"], P("LEDG", "cathode"), P("RG", "a"))
    s.connect("gg", C["gnd"], P("RG", "b"), R("GND"))

    # single-channel relay + its load
    s.add(lib.relay_module("RLY", 2750, 560, channels=1))
    s.add(lib.water_pump("PUMP", 2750, 800,
                         note="wired to the relay — its firing code is commented out"))
    s.connect("d5", C["relay"], P("U1", "D5"), P("RLY", "IN1"))
    s.connect("rlyv", C["v5"], P("RLY", "VCC"), R("5V"))
    s.connect("rlyg", C["gnd"], P("RLY", "GND"), R("GND"))
    s.connect("pump", C["relay"], P("RLY", "OUT"), P("PUMP", "in"))

    _button(s, "SEL", 2250, 950, "D4", "GAME SELECT", "picks the sequence",
            C["ctrl"], (70, 120, 200))
    _buzzer(s, 1900, 950, label="speaker / buzzer")

    entries = [
        KEY_LEGEND_2019,
        RAIL_LEGEND,
        (C["led"], "Feedback LEDs", ["RED (D2) = wrong · GREEN (D3) = right · 220 Ω"]),
        (C["relay"], "Relay ×1", ["D5 · pinMode(OUTPUT) at boot",
                                  "the digitalWrite() burst is commented out in the sketch"]),
        (C["ctrl"], "Game select", ["D4 push button (active-HIGH + 10 kΩ pulldown)"]),
        (C["buzz"], "Speaker", ["D8 · key notes + tunes (bit-banged buzz())"]),
    ]
    notes = [("Hardware delta vs V2: two indicator LEDs, a game-select button and a "
              "single relay channel arrive — the game logic that becomes V4.", C["muted"]),
             NOTE_2019_BOARD]
    s.decorations.append(deco.legend(entries=entries, notes=notes, **_legend_box(GND_2019)))
    return s, "v3-game-prototype", "wiring-v3.png"


# ── V4 / V4.5 ────────────────────────────────────────────────────────────────
def build_v4(plus=False):
    """plus=True renders V4.5: V4's board minus the relay pair and the water
    pump, plus the two MARGIN buttons."""
    if plus:
        title = "Lemon Piano V4.5 — no water pump, plus the touch-tuning buttons"
        sub = ("V4's keyboard, LEDs and controls, with the relay pair and water pump "
               "REMOVED (D5/D6 free) and MARGIN + (D10) / MARGIN − (D11) added. "
               "A late miss now only groans. Game otherwise unchanged.")
        out = ("v4.5-margin-buttons", "wiring-v4.5.png")
    else:
        title = "Lemon Piano V4 — the 02/2019 build (relay water-pump + red/green LED)"
        sub = ("7 lemon touch keys · red/green LED · relay-driven water pump · "
               "game-select on D4 · restart · buzzer. The clip moves to +5 V.")
        out = ("v4-water-pump", "wiring-v4.png")

    s = _board(title, sub, "ATmega328P · Nano (A6 key) / Uno for keys 1-6")
    keyboard_2026(s)

    # feedback LEDs — anode faces the board (W), cathode drops to GND via 220 Ω
    lx = 1650
    s.add(lib.led("LEDR", lx, 560, (215, 45, 45), "RED LED", "D2 · wrong", anode="W"))
    s.add(lib.resistor("RR", lx, 700, orient="V"))
    s.connect("d2", C["led"], P("U1", "D2"), P("LEDR", "anode"))
    s.connect("rc", C["gnd"], P("LEDR", "cathode"), P("RR", "a"))
    s.connect("rg", C["gnd"], P("RR", "b"), R("GND"))

    lx2 = 1850
    s.add(lib.led("LEDG", lx2, 560, (45, 185, 75), "GREEN LED", "D3 · right", anode="W"))
    s.add(lib.resistor("RG", lx2, 700, orient="V"))
    s.connect("d3", C["led"], P("U1", "D3"), P("LEDG", "anode"))
    s.connect("gc", C["gnd"], P("LEDG", "cathode"), P("RG", "a"))
    s.connect("gg", C["gnd"], P("RG", "b"), R("GND"))

    if not plus:
        # relay pair + water pump (far right) — V4 only; V4.5 removed both
        s.add(lib.relay_module("RLY", 2650, 560))
        s.add(lib.water_pump("PUMP", 2650, 780))
        s.connect("d5", C["relay"], P("U1", "D5"), P("RLY", "IN1"))
        s.connect("d6", C["relay"], P("U1", "D6"), P("RLY", "IN2"))
        s.connect("rlyv", C["v5"], P("RLY", "VCC"), R("5V"))
        s.connect("rlyg", C["gnd"], P("RLY", "GND"), R("GND"))
        s.connect("pump", C["relay"], P("RLY", "OUT"), P("PUMP", "in"))

    # game-select (D4) · buzzer (D8) · restart (D7)
    s.add(lib.spdt_switch("SEL", 2050, 900, "D4", com_facing="W"))
    s.connect("d4", C["ctrl"], P("U1", "D4"), P("SEL", "com"))
    s.connect("selv", C["v5"], P("SEL", "p5"), R("5V"))
    s.connect("selg", C["gnd"], P("SEL", "pg"), R("GND"))

    _buzzer(s, 2250, 900)
    _button(s, "RST", 2430, 900, "D7", "RESTART", "re-reads game", C["ctrl"], (70, 120, 200))

    if plus:
        _button(s, "MUP", 2620, 1010, "D10", "MARGIN +", "less sensitive", C["margin"], (220, 60, 150))
        _button(s, "MDN", 2820, 1010, "D11", "MARGIN −", "more sensitive", C["margin"], (220, 60, 150))

    entries = [
        (C["key"], "Lemon keys (7)", ["A0..A6 · 220 Ω each · 5 V through the body"]),
        (C["v5"], "+5 V / GND rails", ["board 5V/GND · button pulldowns"
                                       + ("" if plus else " · relay VCC/GND")]),
        (C["led"], "Feedback LEDs", ["RED (D2) = wrong · GREEN (D3) = right · 220 Ω"]),
        (C["ctrl"], "Controls", ["GAME SELECT (D4, SPDT) · RESTART (D7)"]),
        (C["buzz"], "Buzzer", ["D8 passive buzzer (key notes + tunes)"]),
    ]
    if plus:
        entries.append((C["margin"], "MARGIN +/− (D10/D11)",
                        ["nudge the touch margin live, no reflash needed"]))
        notes = [("Hardware delta vs V4: the relay pair and the water pump come OFF "
                  "(D5/D6 now unused) and two MARGIN buttons go on. A miss from note 7 "
                  "still counts a penalty and groans — it just doesn't spray you.",
                  (180, 40, 120))]
    else:
        entries.insert(3, (C["relay"], "Relay + water pump",
                           ["D5 (IN1) · D6 (IN2) → pump on a late miss"]))
        notes = [("Hardware delta vs V3: the clip moves to +5 V (touch now RAISES the "
                  "reading), a second relay channel and a RESTART button arrive.",
                  C["muted"])]
    s.decorations.append(deco.legend(entries=entries, notes=notes, **_legend_box(RAIL_GND_Y)))
    return s, out[0], out[1]


# ── V5 ───────────────────────────────────────────────────────────────────────
def build_v5():
    """The 2026-07-28 rebuild: GND-clip keyboard (pull-ups), ten-LED bar, two
    sensitivity buttons, no restart and no game-select switch."""
    s = _board("Lemon Piano V5 — ten-LED bar, GND clip, live sensitivity",
               "7 fruit keys pulled UP through 220 Ω (player holds GND) · ten green "
               "LEDs on D2..D11 in order · SENS + (D12) / SENS − (A7) tune the touch "
               "margin live · buzzer on D13. "
               "No relay, no pump, no restart, no game-select.",
               "ATmega328P · Nano only (A6 = key 7, A7 = a button)",
               h=1860, gnd_y=GND_2019)   # taller page: this legend carries four notes
    keyboard_2019(s, fruit="lemon")

    # ten-LED progress bar — also the calibration display and the sensitivity meter
    pins = ["D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "D10", "D11"]
    x0, dx, ybar = 1700, 100, 950
    for i, pin in enumerate(pins):
        cx = x0 + i * dx
        cid = f"L{i}"
        s.add(lib.led(cid, cx, ybar, (45, 185, 75), str(i + 1), pin, anode="N", cathode="S"))
        s.add(lib.resistor(f"{cid}R", cx, ybar + 90, orient="V"))
        s.connect(f"a{i}", C["led"], P("U1", pin), P(cid, "anode"))
        s.connect(f"c{i}", C["gnd"], P(cid, "cathode"), P(f"{cid}R", "a"))
        s.connect(f"g{i}", C["gnd"], P(f"{cid}R", "b"), R("GND"))

    _buzzer(s, 1560, 420, pin="D13")

    # SENS + on D7: plain digital pin, internal pull-up, button straight to GND.
    _button_to_gnd(s, "SUP", 2350, 420, "D12", "SENS +", "more sensitive",
                   C["margin"], (60, 170, 90))

    # SENS − on A7: analog-in only, so it needs an EXTERNAL pull-up — the one part
    # on this board that is not just a button.
    s.add(lib.push_button("SDN", 2850, 420, "SENS −", "less sensitive", cap=(200, 60, 60)))
    s.add(lib.resistor("SDNPU", 2750, 250, orient="V", label="10k"))
    s.connect("sdnsig", C["margin"], P("U1", "A7"), P("SDN", "pin"), P("SDNPU", "b"))
    s.connect("sdnpu", C["v5"], P("SDNPU", "a"), R("5V"))
    s.connect("sdngnd", C["gnd"], P("SDN", "v5"), R("GND"))

    entries = [
        (C["key"], "Lemon keys (7)", ["A0..A6 · 220 Ω pull-up to +5 V each",
                                      "idle ≈ 1022 · a touch drags the pin DOWN"]),
        (C["v5"], "+5 V / GND rails", ["key pull-ups · SENS − pull-up · LED cathodes",
                                       "player's clip is on GND"]),
        (C["led"], "Bar of 10 green LEDs",
         ["ONE ASCENDING RUN: LED n on pin n+1 (D2..D11) · 220 Ω each",
          "game progress · calibration display · sensitivity meter"]),
        (C["margin"], "Sensitivity buttons",
         ["SENS + (D12, to GND) · SENS − (A7, to GND + 10 kΩ pull-up)",
          "both held 1 s while touching a lemon = smart adjust"]),
        (C["buzz"], "Buzzer", ["D13 · key notes, victory themes and UI chirps"]),
    ]
    notes = [("Rebuilt 2026-07-28. The keyboard went back to the 2019 arrangement — "
              "220 Ω pull-ups and a GND clip — because floating +5 V-clip pins could "
              "not be read at all: one touch pushed all seven channels to the rail, "
              "and the idle level drifted ~170 counts on a ~25 s cycle.", C["muted"]),
             ("Pin order is deliberate: the bar is D2..D11 in order, so LED n sits on "
              "pin n+1 and you can wire it left to right without looking anything up. "
              "The buzzer takes D13 (its on-board LED just blinks along) and the button "
              "takes D12, because on many Nanos D13's on-board LED fights an internal "
              "pull-up and would read as permanently pressed.", C["muted"]),
             ("GAME SELECT and RESTART are gone: A7 and D12 are the two buttons now. "
              "The game starts at 1 and auto-advances on every win; recalibration is "
              "the smart-adjust gesture or a board reset.", C["muted"]),
             ("A7 is analog-in only and has no internal pull-up — hence the external "
              "10 kΩ on SENS −. SENS + uses the AVR's own pull-up and needs none.",
              C["muted"])]
    s.decorations.append(deco.legend(entries=entries, notes=notes,
                                     **_legend_box(GND_2019, h=460)))
    return s, "v5-led-bar", "wiring-v5.png"


# ── V5.5 ─────────────────────────────────────────────────────────────────────
def build_v5_5():
    """V5 + a power-entry filter. USB-powering V5 straight from a PC or a wall
    wart lets every light-switch transient in the house ride the 5 V rail into
    the keyboard — whose touch margin is 3-4 ADC counts (~15-20 mV). The board
    itself is byte-for-byte V5; the new hardware is the supply chain in the
    top-left corner: TVS clamp → series Schottky → 470 µF ‖ 100 nF → 100 µH →
    470 µF ‖ 100 nF → the +5 V rail."""
    s = _board("Lemon Piano V5.5 — V5 + filtered 5 V supply",
               "Same board as V5 (7 pulled-up keys, ten-LED bar, SENS ± buttons, "
               "buzzer on D13) — but the 5 V comes in through a transient clamp and "
               "an LC pi filter, because the touch margin is 3-4 ADC counts and a "
               "light switch anywhere in the house used to play the piano.",
               "ATmega328P · Nano only (A6 = key 7, A7 = a button)",
               h=1960, gnd_y=GND_2019)
    keyboard_2019(s, fruit="lemon")

    # ── the power-entry filter, left to right across the top band ────────────
    s.add(lib.power_jack("J1", 130, 110, title="5 V IN",
                         sub="USB charger pigtail — not the PC"))
    s.add(lib.diode("DTVS", 560, 250, orient="V", flip=True, label="P6KE6.8A",
                    sub="TVS clamp"))
    s.add(lib.diode("DS", 700, 140, orient="H", label="1N5817",
                    sub="reverse + USB backfeed"))
    s.add(lib.capacitor("CF1", 850, 250, orient="V", polarized=True, label="470 µF"))
    s.add(lib.capacitor("CF2", 950, 250, orient="V", label="100 nF"))
    s.add(lib.inductor("LF1", 1090, 140, orient="H", label="100 µH",
                       sub="≥ 1 A · low DCR"))
    s.add(lib.capacitor("CF3", 1210, 250, orient="V", polarized=True, label="470 µF"))
    s.add(lib.capacitor("CF4", 1310, 250, orient="V", label="100 nF"))

    s.connect("vin", C["ctrl"], P("J1", "vout"), P("DTVS", "cathode"), P("DS", "anode"))
    s.connect("vraw", C["ctrl"], P("DS", "cathode"), P("CF1", "a"), P("CF2", "a"), P("LF1", "a"))
    s.connect("vfilt", C["v5"], P("LF1", "b"), P("CF3", "a"), P("CF4", "a"), R("5V"))
    s.connect("pgnd", C["gnd"], P("J1", "gnd"), P("DTVS", "anode"), P("CF1", "b"),
              P("CF2", "b"), P("CF3", "b"), P("CF4", "b"), R("GND"))

    # ── everything below is V5, unchanged ────────────────────────────────────
    pins = ["D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "D10", "D11"]
    x0, dx, ybar = 1700, 100, 950
    for i, pin in enumerate(pins):
        cx = x0 + i * dx
        cid = f"L{i}"
        s.add(lib.led(cid, cx, ybar, (45, 185, 75), str(i + 1), pin, anode="N", cathode="S"))
        s.add(lib.resistor(f"{cid}R", cx, ybar + 90, orient="V"))
        s.connect(f"a{i}", C["led"], P("U1", pin), P(cid, "anode"))
        s.connect(f"c{i}", C["gnd"], P(cid, "cathode"), P(f"{cid}R", "a"))
        s.connect(f"g{i}", C["gnd"], P(f"{cid}R", "b"), R("GND"))

    _buzzer(s, 1560, 420, pin="D13")

    _button_to_gnd(s, "SUP", 2350, 420, "D12", "SENS +", "more sensitive",
                   C["margin"], (60, 170, 90))

    s.add(lib.push_button("SDN", 2850, 420, "SENS −", "less sensitive", cap=(200, 60, 60)))
    s.add(lib.resistor("SDNPU", 2750, 250, orient="V", label="10k"))
    s.connect("sdnsig", C["margin"], P("U1", "A7"), P("SDN", "pin"), P("SDNPU", "b"))
    s.connect("sdnpu", C["v5"], P("SDNPU", "a"), R("5V"))
    s.connect("sdngnd", C["gnd"], P("SDN", "v5"), R("GND"))

    entries = [
        (C["ctrl"], "Power-entry filter (NEW)",
         ["P6KE6.8A TVS across the input · 1N5817 in series",
          "470 µF ‖ 100 nF → 100 µH → 470 µF ‖ 100 nF (fc ≈ 700 Hz)"]),
        (C["key"], "Lemon keys (7)", ["A0..A6 · 220 Ω pull-up to +5 V each",
                                      "idle ≈ 1022 · a touch drags the pin DOWN"]),
        (C["v5"], "+5 V / GND rails", ["the rail is the FILTERED node — everything",
                                       "hangs off it exactly as in V5"]),
        (C["led"], "Bar of 10 green LEDs",
         ["ONE ASCENDING RUN: LED n on pin n+1 (D2..D11) · 220 Ω each"]),
        (C["margin"], "Sensitivity buttons",
         ["SENS + (D12, to GND) · SENS − (A7, to GND + 10 kΩ pull-up)"]),
        (C["buzz"], "Buzzer", ["D13 · key notes, victory themes and UI chirps"]),
    ]
    notes = [("Why: the V5 touch margin is 3-4 ADC counts ≈ 15-20 mV (220 Ω pull-up vs "
              "~1 MΩ of body). Any conducted transient bigger than that IS a key press: "
              "a light switch, a PC's shared supply, a neighbour's fridge.", C["muted"]),
             ("The chain: the TVS eats the big spikes (clamps ~10 V) · the Schottky adds "
              "reverse protection and keeps a programming USB cable from back-feeding the "
              "filter · the CLC pi (fc ≈ 700 Hz, 2nd order) kills conducted switching noise "
              "before it reaches AVcc — which is also the ADC reference.", C["muted"]),
             ("Cost: the Schottky + choke drop ~0.3 V, so the rail sits at ≈ 4.7 V. The ADC "
              "is ratiometric (thresholds scale with AVcc), so calibration does not care.",
              C["muted"]),
             ("Feed the filter from a USB wall charger or a bench supply — NOT from the PC "
              "that has twenty other loads on its 5 V. Loop the input lead 3-4 turns through "
              "a clip-on ferrite for the common-mode path the filter cannot touch.", C["muted"]),
             ("Still ghosting? The radiated path remains: 10 nF from each key pin to GND "
              "(with 220 Ω that is a 2 µs pole — invisible to a 70 ms note) is the "
              "documented next step in HARDWARE.md.", C["muted"])]
    s.decorations.append(deco.legend(entries=entries, notes=notes,
                                     **_legend_box(GND_2019, h=560)))
    return s, "v5.5-power-filter", "wiring-v5.5.png"


# ── V6 — battery power + LM386 speaker stage ─────────────────────────────────
W_V6, H_V6, GND_V6 = 5060, 2260, 1400
DX_V6 = 820                 # V5.5 content shifts east; the cell + module take the corner
V5_X0_V6, V5_X1_V6 = 1250, 3760   # the FILTERED rail spans only its own consumers


def build_v6():
    """V5.5 + a battery power source + an amplified speaker.

    Two things are deliberately NOT on the filtered rail, and that is the whole
    point of the drawing:

    * the amplifier is fed from the UNFILTERED 5 V (`vbus`), straight off the
      module. An LM386 driving 4 Ω pulls audio-rate current in the hundreds of
      mA; if that current flowed through L1/C3 it would modulate the rail —
      i.e. AVcc, i.e. the ADC reference the 3-4-count touch margin is measured
      against. The filter's job is to keep the keyboard's reference quiet, so
      the loudest load in the build must sit on its dirty side.
    * D13 cannot drive a 4 Ω coil (PCB ADR-034). It drives a divider, so the
      amp sees ~450 mV instead of a 5 V square, and the amp drives the coil."""
    s = _board("Lemon Piano V6 — battery power + amplified speaker",
               "V5.5's board and filter, unchanged — plus a 1S LiPo on an IP5356 "
               "power-bank module (portable OR wall-fed, same connector) and an "
               "LM386 stage that turns the D13 piezo line into a real speaker. "
               "The amp hangs off the UNFILTERED 5 V on purpose.",
               "ATmega328P · Nano only (A6 = key 7, A7 = a button)",
               w=W_V6, h=H_V6, gnd_y=GND_V6, bx=NANO_X + DX_V6, by=NANO_Y,
               v5_x0=V5_X0_V6, v5_x1=V5_X1_V6)
    keyboard_2019(s, fruit="lemon", dx=DX_V6)

    # ── NEW: the power source — cell → power-bank module → the board's 5 V in ─
    s.add(lib.battery("BAT", 170, 118, label="LiPo 3.7 V",
                      sub="10 000 mAh · 1260110"))
    s.add(lib.power_bank_module("IP1", 800, 190, chip="IP5356",
                                label="power-bank driver module",
                                sub="USB-A out → the board · USB-C = charge in"))
    s.connect("batp", C["relay"], P("BAT", "pos"), P("IP1", "batp"))
    s.connect("batn", C["gnd"], P("BAT", "neg"), P("IP1", "batn"))

    # ── V5.5's power-entry filter, shifted east by DX_V6, otherwise identical ─
    # The series row (Schottky, choke) sits 70 px lower than in V5.5: `vbus` has
    # to reach the amplifier at the far east, and y≈140 is the only straight lane
    # for it. Leaving the Schottky there forced the router up into the subtitle.
    s.add(lib.diode("DTVS", 560 + DX_V6, 250, orient="V", flip=True,
                    label="P6KE6.8A", sub="TVS clamp"))
    # No `sub` captions on the series row: dropped 70 px, they would land on the
    # shunt row's values. The ratings are in the legend and in V5.5's own diagram.
    # The ceramics also move 50 px further from their electrolytic so the "470 µF"
    # caption clears the disc.
    s.add(lib.diode("DS", 700 + DX_V6, 210, orient="H", label="1N5817"))
    s.add(lib.capacitor("CF1", 850 + DX_V6, 250, orient="V", polarized=True, label="470 µF"))
    s.add(lib.capacitor("CF2", 1000 + DX_V6, 250, orient="V", label="100 nF"))
    s.add(lib.inductor("LF1", 1090 + DX_V6, 210, orient="H", label="100 µH"))
    s.add(lib.capacitor("CF3", 1210 + DX_V6, 250, orient="V", polarized=True, label="470 µF"))
    s.add(lib.capacitor("CF4", 1360 + DX_V6, 250, orient="V", label="100 nF"))

    # ONE node: the module's 5 V, the board's input header, and the amp supply.
    s.connect("vbus", C["ctrl"], P("IP1", "vout"), P("DTVS", "cathode"),
              P("DS", "anode"), P("AMP", "vcc"))
    s.connect("vraw", C["ctrl"], P("DS", "cathode"), P("CF1", "a"), P("CF2", "a"),
              P("LF1", "a"))
    s.connect("vfilt", C["v5"], P("LF1", "b"), P("CF3", "a"), P("CF4", "a"), R("5V"))
    s.connect("pgnd", C["gnd"], P("IP1", "gnd"), P("DTVS", "anode"), P("CF1", "b"),
              P("CF2", "b"), P("CF3", "b"), P("CF4", "b"), R("GND"))

    # ── V5's game board, shifted east ────────────────────────────────────────
    pins = ["D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "D10", "D11"]
    x0, dx, ybar = 1700 + DX_V6, 100, 950
    for i, pin in enumerate(pins):
        cx = x0 + i * dx
        cid = f"L{i}"
        s.add(lib.led(cid, cx, ybar, (45, 185, 75), str(i + 1), pin, anode="N", cathode="S"))
        s.add(lib.resistor(f"{cid}R", cx, ybar + 90, orient="V"))
        s.connect(f"a{i}", C["led"], P("U1", pin), P(cid, "anode"))
        s.connect(f"c{i}", C["gnd"], P(cid, "cathode"), P(f"{cid}R", "a"))
        s.connect(f"g{i}", C["gnd"], P(f"{cid}R", "b"), R("GND"))

    s.add(lib.buzzer("BUZ", 1560 + DX_V6, 420, label="passive buzzer",
                     pin_label="D13 · keep or omit"))
    s.connect("buzgnd", C["gnd"], P("BUZ", "gnd"), R("GND"))

    _button_to_gnd(s, "SUP", 2350 + DX_V6, 420, "D12", "SENS +", "more sensitive",
                   C["margin"], (60, 170, 90))

    s.add(lib.push_button("SDN", 2850 + DX_V6, 420, "SENS −", "less sensitive",
                          cap=(200, 60, 60)))
    s.add(lib.resistor("SDNPU", 2750 + DX_V6, 250, orient="V", label="10k"))
    s.connect("sdnsig", C["margin"], P("U1", "A7"), P("SDN", "pin"), P("SDNPU", "b"))
    s.connect("sdnpu", C["v5"], P("SDNPU", "a"), R("5V"))
    s.connect("sdngnd", C["gnd"], P("SDN", "v5"), R("GND"))

    # ── NEW: the audio stage — divider → coupling cap → LM386 → speaker ──────
    s.add(lib.resistor("R19", 3900, 500, orient="H", label="10k"))
    s.add(lib.resistor("R20", 3990, 610, orient="V", label="1k"))
    s.add(lib.capacitor("C5", 4060, 500, orient="H", label="1 µF", sub="DC block"))
    s.add(lib.amp_module("AMP", 4300, 880, chip="LM386",
                         label="LM386 amp module",
                         sub="unfiltered 5 V"))
    s.add(lib.speaker("SPK", 4780, 880, label="speaker", sub="4 Ω · 3 W"))

    # D13 drives the piezo AND the divider — one node, exactly like PCB J5.
    s.connect("buzsig", C["buzz"], P("U1", "D13"), P("BUZ", "sig"), P("R19", "a"))
    s.connect("attn", C["buzz"], P("R19", "b"), P("R20", "a"), P("C5", "a"))
    s.connect("attng", C["gnd"], P("R20", "b"), R("GND"))
    s.connect("ampin", C["buzz"], P("C5", "b"), P("AMP", "sig"))
    s.connect("ampgnd", C["gnd"], P("AMP", "gnd"), R("GND"))
    s.connect("spk", C["buzz"], P("AMP", "out"), P("SPK", "p"))
    s.connect("spkret", C["gnd"], P("SPK", "n"), R("GND"))

    # The three ways this board can be powered, written where the power block is.
    s.decorations.append(deco.panel(
        170, 470, 520, 600, "The three power modes", accent=C["ctrl"], rows=[
            ("h", "1 · Portable — on the cell"),
            "Unplug everything, press ON on the module.",
            "The board then floats WITH the player, so the",
            "common-mode path the series filter cannot",
            "touch disappears. Quietest of the three.",
            "",
            ("h", "2 · Wall-fed — same connector"),
            "Move the two-wire pigtail from the module's",
            "USB-A to a USB wall charger. Electrically this",
            "is V5.5 exactly; the cell is not involved.",
            "",
            ("h", "3 · Playing while charging"),
            "USB-C into the module, keep taking 5 V off",
            "USB-A. Only works if this module does TRUE",
            "pass-through — many alternate between",
            "charging and output, which drops the rail and",
            "reboots the Nano. MEASURE before relying on",
            "it. It is also the noisiest mode: mains is back",
            "in the loop, so modes 1 and 2 stay preferable.",
            "",
            ("h", "Star ground — build note"),
            ("The amp GND and the speaker return carry", (170, 60, 60)),
            ("hundreds of mA at audio rate. Land them on", (170, 60, 60)),
            ("the MODULE's G, never through the board's", (170, 60, 60)),
            ("ground: shared copper with the key returns", (170, 60, 60)),
            ("puts the music straight onto the sense node.", (170, 60, 60)),
        ]))

    entries = [
        (C["relay"], "Battery source (NEW)",
         ["1S LiPo 3.7 V / 10 000 mAh → IP5356 B+ / B−",
          "0.03C at the piano's 200 mA worst case · days of runtime"]),
        (C["ctrl"], "Unfiltered 5 V — `vbus` (NEW)",
         ["module USB-A out → TVS/Schottky (the board) AND → LM386 V+",
          "the amp's audio-rate current NEVER crosses the filter"]),
        (C["v5"], "Filtered +5 V rail",
         ["stops short on purpose: Nano, key pull-ups and the A7 pull-up only",
          "≈ 4.7 V · this node is AVcc, the ADC reference"]),
        (C["key"], "Lemon keys (7)", ["A0..A6 · 220 Ω pull-up to the filtered rail",
                                      "idle ≈ 1022 · a touch drags the pin DOWN 3-4 counts"]),
        (C["led"], "Bar of 10 green LEDs",
         ["ONE ASCENDING RUN: LED n on pin n+1 (D2..D11) · 220 Ω each"]),
        (C["margin"], "Sensitivity buttons",
         ["SENS + (D12, to GND) · SENS − (A7, to GND + 10 kΩ pull-up)"]),
        (C["buzz"], "Audio: D13 → LM386 → speaker (NEW)",
         ["D13 → 10 kΩ / 1 kΩ divider (≈ ÷11) → 1 µF → amp IN → 4 Ω 3 W",
          "the on-board piezo stays in parallel on the same node (PCB J5)"]),
    ]
    notes = [("Why a battery at all: the filter is a SERIES filter, so it cannot touch the "
              "common-mode path — the player's body is capacitively coupled to the mains "
              "while the board is referenced to earth through the charger. On battery the "
              "whole board floats WITH the player and that difference cancels in the ADC. "
              "It also removes the charger's Y-cap leakage, which today flows through the "
              "player's hand and straight into the sense node.", C["muted"]),
             ("Why the amp is on the DIRTY side: an LM386 into 4 Ω draws hundreds of mA at "
              "audio rate. Through L1/C3 that current would modulate the rail — which is "
              "AVcc, measured against a 15-20 mV touch margin. Tap it before the TVS, and "
              "run its ground back to the MODULE, not through the board's ground: the "
              "speaker return must not share copper with the key returns (star ground).",
              C["muted"]),
             ("Why the divider: D13 is a 5 V square wave and the LM386's input expects "
              "millivolts. 10 kΩ / 1 kΩ gives ~450 mV pp (0.45 mA off the pin — nothing) "
              "and the module's gain pot does the rest. The 1 µF blocks D13's 2.5 V DC "
              "average; most LM386 boards already have a coupling cap, this one makes the "
              "circuit independent of which board you got. NEVER wire a 4 Ω coil to D13 "
              "directly — that is PCB ADR-034.", C["muted"]),
             ("Charge input: USB-C on the module, drawn but not wired — nothing on the "
              "board connects to it. Take the board's 5 V from the module's USB-A with a "
              "TWO-WIRE pigtail only (D+/D− absent → no QC/PD handshake → it stays at "
              "5 V). A port that negotiated 9 V would burn the P6KE6.8A in continuous "
              "conduction and then kill the ATmega.", C["muted"]),
             ("Open risk, to be measured before building: the piano idles at 25-35 mA "
              "(free play = all ten LEDs dark) and IP5356-class modules cut the output "
              "below ~45-75 mA. If it self-shuts in the silences, the fixes are the "
              "module's low-current mode, a keep-alive LED in firmware, or a 100 Ω bleeder "
              "on `vbus`. Also check it starts into the 940 µF of C1+C3 without hiccuping "
              "— if it does not, drop C1 to 220 µF (fc is set by C3, not C1).", C["muted"])]
    s.decorations.append(deco.legend(entries=entries, notes=notes,
                                     **_legend_box(GND_V6, w=W_V6, h=620)))
    return s, "v6-battery-amp", "wiring-v6.png"


# version key -> builder (add a row per hardware revision)
TARGETS = {
    "v0": build_v0,
    "v1": build_v1,
    "v2": build_v2,
    "v2.5": build_v2_5,
    "v3": build_v3,
    "v4": lambda: build_v4(False),
    "v4.5": lambda: build_v4(True),
    "v5": build_v5,
    "v5.5": build_v5_5,
    "v6": build_v6,
}


if __name__ == "__main__":
    wanted = sys.argv[1:] or list(TARGETS)
    for key in wanted:
        if key not in TARGETS:
            sys.exit(f"unknown version '{key}' — known: {', '.join(TARGETS)}")
        s, vdir, fname = TARGETS[key]()
        res = build(s)
        out = VERSIONS / vdir / "images"
        out.mkdir(parents=True, exist_ok=True)
        print(f"{key:8s} nets={len(s.nets):3d} → {save(s, str(out / fname))}")
