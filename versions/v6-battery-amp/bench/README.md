# V6 bench — characterise the power source before it meets the piano

Two instruments and a protocol, for the state the project is actually in: the
LiPo is on the IP5356 (micro-USB variant), a KWS-X1 meter is on hand, and
**there is no piano PCB yet**. That order is the right one — every risk in
[../README.md](../README.md#open-risks--measure-before-building) lives in the
module, not the board, so the module gets measured first.

| Tool | What it is for |
|---|---|
| [`psu-probe/`](psu-probe/) | rail voltage (AVcc, via the ATmega bandgap), cell voltage, dropout counting |
| [`touch-noise/`](touch-noise/) | the V5.5 bench recipe on one channel: is battery quieter than mains? |

Both build with `pio run`. Flash with the Nano's **own USB and the module
unplugged**, then move to the module rig.

## The rig

The module's USB-A output already has the right cable in the parts bin:
`cbl-usba-m-pelado-30cm` (USB-A male → two bare wires). It carries **only** V+
and GND — no D+/D−, so no QC/PD handshake can happen and the output is pinned at
5 V. That is the safe configuration by construction, and it is also the one the
piano will use.

```
 module USB-A ──[USB-A→bare-wire pigtail]──┬── Nano 5V pin
                                           ├── load bank (see below)
                                           └── (KWS-X1: see the cable note)
 module GND (pigtail black) ───────────────┴── Nano GND
 cell B+ ──────────────────────────────────── Nano A1     (direct, ≤ 4.2 V)
 USB-TTL TX/RX/GND ───────────────────────── Nano RX0/TX1/GND   (NOT its VCC)
```

> ⚠ **Never connect the Nano's own USB while feeding its 5V pin.** On the real
> board the 1N5817 makes that safe; on this breadboard there is no diode. Serial
> comes from the `mod-cp2102-usb-ttl` / `mod-ch340g-usb-ttl` adapter, TX/RX/GND
> only, leaving its VCC pin in the air.

### The load bank

1/4 W 1 % metal-film resistors straight off the pigtail, added one at a time.
Both values stay comfortably inside their rating at 5 V:

| Value | Current each | Dissipation each | Use |
|---|---|---|---|
| **470 Ω** | 10.6 mA | 53 mW | fine steps — hunting the cut-off |
| **220 Ω** | 22.7 mA | 114 mW | coarse steps — piano-sized loads |

| n × 470 Ω | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
|---|---|---|---|---|---|---|---|---|
| mA | 10.6 | 21.3 | 31.9 | 42.6 | 53.2 | 63.8 | 74.5 | 85.1 |

| n × 220 Ω | 1 | 2 | 3 | 4 | 6 | 9 |
|---|---|---|---|---|---|---|
| mA | 22.7 | 45.5 | 68.2 | 90.9 | 136 | 205 |

**The Nano running `psu-probe` is itself the most valuable load**: at ~25-35 mA
it *is* the piano's idle current (free play, all ten LEDs dark). So the headline
test is simply "does the module stay on with nothing but the Nano attached?"

### KWS-X1 cable note

The KWS-X1 is USB-C male (in) → USB-C female (out), and every output on this
module is USB-A. So it **cannot** sit in the USB-A path without a `USB-A male →
USB-C female` adapter (not in the parts bin; ~2 €).

This is not blocking. With 1 % resistors the load current is *known* (I = V/R)
and `psu-probe` measures V, so T1-T3 and T5 need no meter at all. The KWS earns
its place on:

- **T6 ripple** — its oscilloscope page samples at 2-5 MHz, which is the only
  way here to see a boost's switching ripple.
- **capacity / mAh totalising** for a real runtime figure.
- the **charge path** (T0), where it plugs straight into a USB-C charger's
  receptacle and needs only a `USB-C → micro-USB` cable on the far side.

> ⚠ Do **not** use the KWS's protocol-test / PD-decoy page against this module's
> output. Triggering 9 V is exactly the failure mode that burns the
> P6KE6.8A (a 600 W *transient* part) and then the ATmega.

## The protocol

Fill the last column in as you go. Anything marked 🔴 is a go/no-go for V6.

### T0 — the cell is already connected: is it healthy?

The module was wired up before this protocol existed, so start by proving no
damage was done (the IP5356's battery input has no reverse protection).

1. Press the module's button. **The display must light and show a plausible %.**
   Nothing should be warm. If the display is dead, stop — suspect reversed cell
   polarity and check with `dev-lcr-ar1-tester` / `dev-aneng-bt189-tester`
   before applying any charger.
2. Charge via micro-USB and confirm it **terminates** (% stops climbing, board
   cools). Note the time from x % to 100 % — with a 5 V/2 A input expect roughly
   5-6 h from low.
3. With the module powering the Nano, read `cell_mV` from `psu-probe`: a rested
   1S cell sits **3.6-4.2 V**. Below 3.0 V or above 4.3 V → do not proceed.

| Reading | Expected | Yours |
|---|---|---|
| display on button press | plausible % | |
| charge terminates | yes | |
| `cell_mV` rested | 3600-4200 | |

### T1 — 🔴 output voltage, and no fast-charge surprise

`psu-probe` on the module, pigtail only, no extra load.

| Reading | Expected | Yours |
|---|---|---|
| `rail_mV` | **4900-5250** | |
| `rail_mV` ever > 5500 | **never** | |
| `rail_max_mV` after 10 min | still < 5500 | |

Anything above ~5.5 V means the port negotiated a fast-charge profile, which the
two-wire pigtail should make impossible. If it happens, the module is not
usable for this project on that port.

### T2 — 🔴 the low-load cut-off (the test that decides V6)

The piano idles at 25-35 mA; IP5356-class parts cut the output below roughly
45-75 mA. If it cuts, the piano switches itself off during the silences.

1. Nano alone (≈25-35 mA). Wait **5 minutes**. Does the output survive?
2. If it died: press the button, add one 470 Ω, wait 5 min, repeat. Record the
   smallest resistor count that survives.
3. `boots` climbing while you watch = hiccup. Board dark and staying dark =
   latching auto-off (the expected behaviour).

| Load | Survives 5 min? |
|---|---|
| Nano only (~30 mA) | |
| + 1 × 470 Ω (~41 mA) | |
| + 2 × 470 Ω (~51 mA) | |
| + 3 × 470 Ω (~62 mA) | |
| + 4 × 470 Ω (~73 mA) | |

**Reading the result.** Survives on the Nano alone → the biggest risk to V6 is
gone. Needs +1 or +2 → a keep-alive of one or two LEDs from the progress bar
covers it in firmware (15 mA each), no hardware. Needs +3 or more → either the
module's low-current mode (T3) or a 100 Ω bleeder, and the "one LED" fix is not
enough.

### T3 — is there a low-current mode?

Only if T2 failed. Try a **double-press** of the module's button (the usual gesture
on IP53xx boards for a low-current / earbud mode), then re-run T2 step 1. Also
try a long press. Record what each gesture does — this is undocumented on these
boards, so it is worth writing down.

| Gesture | Effect | Survives ~30 mA? |
|---|---|---|
| single press | | |
| double press | | |
| long press | | |

### T4 — inrush into 940 µF — ⛔ BLOCKED, parts missing

C1 + C3 present ~940 µF to the module, whose short-circuit protection reacts in
< 50 µs; it may hiccup or refuse to start. **Cannot be tested yet** — the parts
bin has no 470 µF electrolytic (largest is `cap-mkt-1uf-160v`), no 100 µH ≥ 1 A
choke and no TVS. Shopping list for the V5.5 filter:

| Part | Qty | Note |
|---|---|---|
| 470 µF / 16 V electrolytic, low ESR | 2 | C1, C3 |
| 100 µH power choke, ≥ 1 A, DCR ≤ 0.3 Ω | 1 | L1 — a 100 mA signal inductor will saturate |
| P6KE6.8A axial TVS (or SA5.0A) | 1 | D1 |
| 1N5817 | 1 | D2 — **`semi-ss14` can stand in** (Vf ~0.5 V vs 0.32 V, so the rail lands at ≈4.55 V instead of 4.7 V; harmless, the ADC is ratiometric) |
| 100 nF X7R | 2 | C2, C4 — `cap-mlcc-470nf` is close enough for a bench mock-up |

When the parts arrive: build the filter on a breadboard, power it from the
module, and watch `boots`. Hiccup → drop **C1** to 220 µF. Never C3: `fc` is set
by C3, not C1.

### T5 — playing while charging (true pass-through?)

Micro-USB charging **and** the USB-A load running, together. Watch `boots` and
`rail_min_mV` for 10 minutes.

| Reading | True pass-through | Alternating (bad) |
|---|---|---|
| `boots` | unchanged | climbing |
| `rail_min_mV` | within ~100 mV of nominal | deep periodic dips |

Climbing `boots` or deep dips → **do not play while charging**; the module
alternates between charging and output, and each handover reboots the Nano.

### T6 — ripple (KWS, needs the adapter)

Ripple page, at the Nano-only load and again at ~200 mA.

| Load | Ripple p-p | Verdict |
|---|---|---|
| ~30 mA | | |
| ~200 mA | | |

Note the number, but do not panic at 20-60 mV: the V5.5 CLC pi has fc ≈ 730 Hz
against a boost switching near 1 MHz — over 3 decades at −40 dB/dec, so tens of
mV arrive at the rail as tens of µV. This measurement is a sanity check on the
module, not a pass/fail for the piano.

### T7 — 🔴 the noise comparison (`touch-noise`)

The one that validates V6's actual premise. Full protocol in
[`touch-noise/src/main.cpp`](touch-noise/src/main.cpp). Three runs, same room,
same switch, 20 flips each, **PC unplugged** (a PC ground re-creates the very
common-mode path under test — count D13 flashes instead, or film it).

| Run | Flashes / 20 flips | `worst_dev` |
|---|---|---|
| A — wall charger direct | | |
| B — battery + module | | |
| C — battery + module, charging | | |

**Prediction: B < A, and C > B.** If B is not better than A, the central claim
for going battery-powered is wrong for this build, and V6 becomes a
portability-only change — still valid, but stop quoting the noise argument.

## After the runs

Copy the filled tables into `../HARDWARE.md` when V6 becomes a real version,
and append the outcome to `../../../CHANGELOG.md`. Measured numbers replacing
computed ones is exactly the gap
[V5.5](../../v5.5-power-filter/HARDWARE.md#bench-validation-recipe) has been
carrying since it was designed.
