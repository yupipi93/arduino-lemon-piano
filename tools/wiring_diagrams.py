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
           bx=NANO_X, by=NANO_Y):
    s = Schematic(w, h, title=title, subtitle=subtitle)
    s.add_rail(Rail("5V", y=RAIL_5V_Y, x0=RAIL_X0, x1=w - 110, color=C["v5"], label="+5 V"))
    s.add_rail(Rail("GND", y=gnd_y, x0=RAIL_X0, x1=w - 110, color=C["gnd"], label="GND"))
    s.add(lib.arduino_nano("U1", bx, by, board=board))
    s.connect("nano5v", C["v5"], P("U1", "5V"), R("5V"))
    s.connect("nanognd", C["gnd"], P("U1", "GND"), R("GND"))
    return s


# ── keyboard, 2019 wiring (player holds GND, pins biased HIGH) ───────────────
def keyboard_2019(s, fruit="banana"):
    """Each analog pin is pulled UP to +5 V through 220 Ω and reads ~1023 idle;
    the fruit sits directly on the pin, so touching it drags the pin down
    through the player's body to the hand-held GND clip (`analogRead <= 1019`).

    Layout: one horizontal run per key (fruit → pin, straight east) and the
    pull-up comb hanging above it in a staircase, so no run has to detour."""
    s.add(lib.clip_box("CLIP", 130, 1130, w=430, title="hand-held GND clip",
                       sub=f"one hand on the clip, one on the {fruit} = the player"))
    s.connect("clipgnd", C["gnd"], P("CLIP", "out"), R("GND"))

    key_ys = [470 + i * 100 for i in range(7)]
    order = [6, 5, 4, 3, 2, 1, 0]                    # analog index per row
    PULLUP_Y = 420                                   # comb band, just under the +5 V rail
    for row, ai in enumerate(order):
        ky = key_ys[row]
        s.add(lib.lemon_key(f"K{ai}", KEY_X, ky, ai + 1, f"A{ai}"))
        rx = 470 + row * 100                        # own channel per key in the pull-up comb
        s.add(lib.resistor(f"RK{ai}", rx, PULLUP_Y, orient="V", length=50))
        # ONE node: the pin, the fruit clip and the pull-up's low side.
        s.connect(f"kn{ai}", C["key"], P("U1", f"A{ai}"), P(f"K{ai}", "clip"), P(f"RK{ai}", "b"))
        s.connect(f"kp{ai}", C["v5"], P(f"RK{ai}", "a"), R("5V"))

    s.decorations.append(deco.dashed_arrow(KEY_X + 780, key_ys[-1] + 90, KEY_X + 360,
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
    s = _board("Lemon Piano V5 — ten-LED progress bar (no pump)",
               "7 lemon keys · ten green LEDs (progress bar) · game-select on A7 · "
               "restart · buzzer. No relay, no pump, no red LED.",
               "ATmega328P · Nano only (needs A6+A7)")
    keyboard_2026(s)

    # ten-LED progress bar
    pins = ["D2", "D3", "D4", "D5", "D6", "D9", "D10", "D11", "D12", "D13"]
    x0, dx, ybar = NANO_X + NANO_W_ + 120, 92, 980
    for i, pin in enumerate(pins):
        cx = x0 + i * dx
        cid = f"L{i}"
        s.add(lib.led(cid, cx, ybar, (45, 185, 75), str(i + 1), pin, anode="N", cathode="S"))
        s.add(lib.resistor(f"{cid}R", cx, ybar + 90, orient="V"))
        s.connect(f"a{i}", C["led"], P("U1", pin), P(cid, "anode"))
        s.connect(f"c{i}", C["gnd"], P(cid, "cathode"), P(f"{cid}R", "a"))
        s.connect(f"g{i}", C["gnd"], P(f"{cid}R", "b"), R("GND"))

    # game-select on A7 (analog), left of the board near A7
    s.add(lib.spdt_switch("SEL", 830, 250, "A7", com_facing="E", analog=True))
    s.connect("a7", C["ctrl"], P("SEL", "com"), P("U1", "A7"))
    s.connect("selv", C["v5"], P("SEL", "p5"), R("5V"))
    s.connect("selg", C["gnd"], P("SEL", "pg"), R("GND"))

    # restart + buzzer in the upper-right band
    _button(s, "RST", NANO_X + NANO_W_ + 420, 560, "D7", "RESTART", "re-reads game",
            C["ctrl"], (70, 120, 200))
    _buzzer(s, NANO_X + NANO_W_ + 700, 560)

    entries = [
        (C["key"], "Lemon keys (7)", ["A0..A6 · 220 Ω each · 5 V through the body"]),
        (C["v5"], "+5 V / GND rails", ["board 5V/GND · LED cathodes · button pulldown"]),
        (C["led"], "Progress bar (10 LEDs)",
         ["D2,3,4,5,6,9,10,11,12,13 · 220 Ω each", "1 LED per correct note · all 10 = win"]),
        (C["ctrl"], "Controls", ["GAME SELECT on A7 (SPDT, analog-in) · RESTART (D7)"]),
        (C["buzz"], "Buzzer", ["D8 passive buzzer (key notes + victory tunes)"]),
    ]
    notes = [("Hardware delta vs V4.5: the red LED and the MARGIN buttons come off "
              "(the relays and pump went with V4.5); ten green LEDs go on. Every I/O "
              "line is now used: A0–A7 + D2–D13. "
              "A7 is analog-in only — drive it with an SPDT to 5 V / GND. A classic Uno "
              "lacks A6/A7.", C["muted"])]
    s.decorations.append(deco.legend(entries=entries, notes=notes, **_legend_box(RAIL_GND_Y)))
    return s, "v5-led-bar", "wiring-v5.png"


# version key -> builder (add a row per hardware revision)
TARGETS = {
    "v0": build_v0,
    "v1": build_v1,
    "v2": build_v2,
    "v3": build_v3,
    "v4": lambda: build_v4(False),
    "v4.5": lambda: build_v4(True),
    "v5": build_v5,
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
