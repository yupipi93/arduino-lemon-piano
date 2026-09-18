# Changelog archive: undated

Entries moved here verbatim from `CHANGELOG.md` by `qpc-changelog-archive`.
Nothing on this page has been reworded; `--verify` checks that against a
SHA-256 taken before the split. Newest first, as in the original.

---

## theme, VU-meter colours in the emulator

Two owner-requested LED/timing changes:

- **Calibration now fills all ten LEDs, not seven.** Only seven keys exist, so
  the old one-LED-per-key loop never lit LEDs 8-10. `lit = (key_index+1) ×
  LED_COUNT / KEY_COUNT` spreads the seven measurement steps proportionally
  across all ten (1,2,4,5,7,8,10 — uneven but always ends at ten), and a new
  `CAL_STEP_PAUSE_MS` (150 ms) pause per key — after its coin, not touching
  `CAL_SAMPLES` so measurement quality is unchanged — makes the count-up read
  as deliberate instead of a blur. Calibration now takes ~1.05 s longer.
- **The win theme now progressively fills the bar instead of flashing it.**
  `playSong()` gained two optional parameters, `ledTotal`/`ledOffset`: when
  `ledTotal > 0` the bar accumulates LEDs (0 to LED_COUNT, staying lit note to
  note) proportional to position in the theme, instead of the old
  whole-bar-flash-per-note. `playVictory()` passes the tail's own note count
  as `ledTotal`, so the fill paces itself to whichever level's theme is
  playing (26-40 notes) with no separate "make it longer" tuning needed — the
  existing tail lengths already spread comfortably across ten LEDs (230-580 ms
  each). The level-start intro keeps the old flash (`ledTotal` defaults to 0,
  unchanged call site) since only calibration and the win were asked for.
- **Emulator LEDs recoloured like a VU meter**: 1-3 green, 4-6 yellow, 7-8
  orange, 9-10 red, across all five spec files that define the ten-LED bar.
  Emulation-only — the real board's LEDs are all green per the BOM.

**Emulation specs re-timed a fourth time**: the +1.05 s calibration pause is a
fixed one-time delay at boot, so it shifts every input/marker in every spec by
the same flat amount — reapplied via a script rather than hand-editing each
timestamp. All four specs green; cross-checked against a real run (`Level 4`
@54834 ms, `ALL LEVELS CLEAR` @79971 ms). All three `pio run` envs build clean
(flash 39.5%/12140 B on hardware, up from 38.8%/11908 B).

## a more recognisable Castle theme

Three more owner-requested changes:

- **Faster Uno autoplayer**: `emulation/autoplayer.yaml`'s `pressKey()` held
  each press 250 ms + 300 ms gap (550 ms/note, tuned for reliability — 90 ms
  measured flaky ~1-in-3, 250 ms measured reliable 9/9). Cut to 160 ms + 200 ms
  (360 ms/note, ~35% faster) — a middle value chosen to sit clear of the
  known-flaky 90 ms while noticeably quicker than 250/300, but **not
  re-verified live at the same 9/9 rigor** (this is an interactive-only,
  emulation-only test aid; a live Playwright re-verification was judged
  disproportionate to the ask). Compile-checked standalone via `arduino-cli
  compile --fqbn arduino:avr:uno` (clean); re-measure and pull back toward
  250/300 if this ever misfires in practice.
- **Level 3 and 4 themes swapped**: Castle now closes the game at level 4
  (makes more sense as the finale) and Starman moved to level 3. Only
  `playVictory()`/`playLevelIntro()`'s switch statements changed — each
  level's own key notes and secret code stayed exactly where they were through
  both this swap and the earlier Underwater→Castle swap.
- **Castle theme redesigned for recognisability**: a player found the first
  version (a plain repeating four-note pulse) too generic. Rebuilt around the
  two traits every description of the real piece agrees on — a fast
  alternating "pedal" hook (Super Mario Wiki's own trivia notes the opening
  echoes The Twilight Zone TV theme's repeated-note guitar riff) answered by a
  chromatic descending run (the "danger" quality every analysis calls out, and
  the only chromatic movement of the four themes, so it can't be confused with
  the others even out of context). Verse repeats, then the same verse a fourth
  higher (victory tail starts here), then a resolving coda. Still tagged 🔨
  reconstruction — no verbatim source exists for this piece, and the Twilight
  Zone connection is documented trivia used as a design anchor, not something
  verified by ear against the original recording.

**Emulation specs re-timed a third time**: Starman (short, ~7.3 s win total)
now at level 3, Castle (redesigned, ~14.4 s win total — still the longest)
now at level 4. Recomputed from the melody tables' `playSong()` math and
cross-checked against a real run's serial timestamps (measured: `Level 4`
@53772 ms, `WIN` @64175 ms, `ALL LEVELS CLEAR` @78913 ms — all inside the
scheduled margins, and the overall run is noticeably shorter than before since
the redesigned Castle theme is more compact than the original). All four
specs green; all three `pio run` envs build clean (flash unchanged at
38.8%/11908 B on hardware — one byte more than before from the swapped
`switch` cases, same tables just reordered).

## the ending loops until reset

Three more owner-requested changes:

- **Stuck-key cue**: pressing an already-locked key (same lemon again, before
  a different one unlocks it) used to be completely silent forever — no
  feedback that anything was even listening. Now it stays silent for the
  first `KEY_LOCK_COOLDOWN_MS` (500 ms) after release (`lastReleaseAt` tracks
  this, so a quick accidental double-tap still gets no cue), but a press
  after that plays `sfxKeyStuck` — a new low, rattling triple-hit, deliberately
  distinct from Bump's two-tone end-stop cue since it means a different thing
  ("this key specifically is locked" vs. "the knob can't go further").
- **Level 3: Castle replaces Underwater.** Underwater's subtle opening
  (a chromatic slide) turned out too hard to recognise as a distinct level.
  Castle — SMB1's dark, driving fortress theme, one of its most recognisable
  pieces after Overworld itself — takes its place: `castleNotes[]/
  castleTempo[]` in `main.cpp`, same full-theme treatment as the other three
  (own `CASTLE_VICTORY_FROM`/`CASTLE_INTRO_LEN`). No letter-note tab exists
  for this piece anywhere searched (unlike Overworld/Underworld/Underwater/
  Starman), so it's tagged 🔨 reconstruction, built to match the piece's
  well-documented key (G minor), tempo (90 BPM, 2/2) and driving/syncopated
  character rather than transcribed note-for-note. Level 3's key notes and
  secret code are unchanged — only the win jingle and level-start intro moved.
- **The game-complete piece now loops.** Clearing all four levels used to
  play the (still fairly new) castle-clear jingle once and move on. Extended
  it from ~1.4 s to a three-phrase, ~4.8 s piece with its own cadence
  (`sfxEnding` in `mario_sfx.h`) and added `playEndingLoop()`, which repeats
  it until the player holds both sensitivity buttons for `RECAL_HOLD_MS`
  (1 s — the same gesture/duration as smart adjust, but reaching it from here
  means something different: reset straight to level 1 with **no
  recalibration**, since the player may not be near the fruit while it plays).
  `playSfx()` gained an optional `checkAbort` callback, polled after every
  single note (not just between repeats), so the 1 s hold is honoured almost
  immediately. Emulation has no sensitivity buttons (every digital pin is
  already a LED or a key), so there it just plays the piece once.

**Emulation specs re-timed again**: Castle's victory tail (39 notes, ~16.9 s)
is longer than Underwater's was (33 notes, ~13.6 s), pushing level 4's start
in `all-levels-win.yaml` later by ~3.5 s. Recomputed from the melody tables'
`playSong()` math and cross-checked against a real run's serial timestamps
(measured: `Level 4` @68867 ms, `WIN` @76685 ms, `ALL LEVELS CLEAR` @84293 ms —
all inside the scheduled margins). `lemon-piano.yaml`, `free-play.yaml` and
`hold-and-repeat.yaml` needed no changes: none of their scripted repeats
exceed the new 500 ms stuck-key cooldown, and none of them ever reach level 3
or the ending. All four specs green; all three `pio run` envs build clean
(flash 38.7%/11888 B on hardware, up from 36.9%/11324 B).

## themes, a real castle-clear ending

Three more owner-requested changes to V5's sound design, on top of the same
day's mistake-cue/win-order fix below:

- **Level-start announce**: new `playLevelIntro()` plays the first few notes of
  the level's own theme — `MARIO_INTRO_LEN`/`UNDER_INTRO_LEN`/
  `UNDERWATER_INTRO_LEN`/`STARMAN_INTRO_LEN` notes from index 0 of the SAME
  table the win jingle already plays from, no extra flash needed. Runs once at
  boot (level 1) and once every level transition, so the player always hears
  which of the four they landed on before touching a lemon. Blocking, like
  every other melody here, so a key touched during it is lost, not queued —
  documented in the emulation specs that had to be re-timed for it.
- **Underwater and Starman are now full themes**, same treatment as Overworld/
  Underworld: `underwaterNotes[]/underwaterTempo[]` and
  `starmanNotes[]/starmanTempo[]` moved into `main.cpp` as PROGMEM tables
  played via `playSong()`, each with its own `*_VICTORY_FROM` cut-in — replacing
  the old short `playSfx()` excerpts in `mario_sfx.h` (now removed). Underwater
  grew from a 20-note opening to a 55-note piece (opening phrase-pair sourced
  from the cited tab, a second phrase-pair a step down and a closing bridge
  are this project's own extension of that tab's repeating structure — see
  `docs/MARIO-SOUNDS.md` for the honest provenance split). Starman grew from
  26 to 48 notes — the real theme is a short vamp with nothing further to
  transcribe, so "full" means the validated figure played through twice before
  the closing phrase, matching what the NES itself does (loop the vamp).
  Visual side effect: levels 3/4's win light show changed from `playSfx`'s
  step-one-LED-per-note to `playSong`'s whole-bar-flash-per-note, now matching
  levels 1/2.
- **A real castle-clear ending**: `sfxEnding` (all-four-levels-clear fanfare)
  was a generic invented descent with no connection to the actual game.
  Research turned up that SMB1 plays a *different*, more triumphant fanfare
  after a castle level than after a flagpole (Super Mario Wiki calls it "World
  Clear") — but no verbatim note-by-note transcription of that specific cue
  could be found, only a description ("Mario Cadence" idiom, C major, same
  family as the flagpole fanfare and the power-up SFX). Rebuilt to match that
  description: the same arpeggio idiom as the flagpole fanfare
  (`sfxLevelClear`), arranged as a call-and-response resolving a step higher,
  tagged 🔨 reconstruction like the fanfare it's built from — not claimed as
  verbatim, which would have been dishonest given what was actually sourceable.

**Emulation specs re-timed**: `playLevelIntro()` adds a new blocking delay at
boot and every level transition (~2.0 s for levels 1/2/4's intros, ~5.9 s for
level 3's — 13 notes at a waltz tempo), and levels 3/4's much longer full
victory themes push every later gap out further still. Recomputed every
`inputs:` timestamp in `lemon-piano.yaml`, `free-play.yaml`,
`hold-and-repeat.yaml` and `all-levels-win.yaml` from the melody tables'
`playSong()` math, then verified the computed numbers against a real run's
serial timestamps (measured: `Level 2` @18624 ms, `Level 3` @37958 ms, `Level 4`
@65537 ms, `ALL LEVELS CLEAR` @80788 ms — all comfortably inside the scheduled
margins). All four specs green, including `hold-and-repeat.yaml`'s buzzer
edge-count assertion, which incidentally now clears a threshold it fell just
short of before this change (pre-existing gap, not something this change set
out to fix). All three `pio run` envs still build clean.

## not a mistake

After v4's fixes, autoplay worked end to end — but manual lemon clicks now did
nothing at all. Live-tested (Playwright again) with `window.__spiceDebug()`:
pressing key6 changed the voltage on unrelated nodes but NEVER on key6's own
node, while the finger wire's board-to-board connection to that exact same
node stayed rock steady. Root cause: the finger pin is a driven `OUTPUT`
(idle HIGH) sitting on the SAME node as the button — in the digital
fast-path both AVR boards use, that's a hard short, and the finger's
constant HIGH always won, so a real press could never pull the node down.

Tried making the finger pin high-Z (`INPUT`) while idle instead, so it
wouldn't fight a real click — manual clicks came back, but auto-play went
dead in the other direction, even with the press held 500 ms. This simulator
doesn't re-evaluate a pin's cross-board connection when its `pinMode`
changes at runtime, so once the finger toggled back to `INPUT`, nothing it
did afterward ever reached piano again.

Fix: keep the finger pin a permanent `OUTPUT` (set once, never toggled), but
route it through a 220Ω series resistor instead of a bare wire. That turns
the hard short into an ordinary voltage divider — a real button's near-zero
contact resistance still wins over 220Ω when someone clicks a lemon, and the
finger's driven LOW still wins over piano's own pull-up when auto-playing.
Had to also raise the press hold from 90 ms to 250 ms — the SPICE-resolved
divider needs time to settle, and 90 ms measured flaky (~1-in-3 misfires
across repeated live runs); 250 ms measured 9/9 reliable. Verified live, one
continuous session: autoplay wins level 1 → piano auto-advances to level 2 →
a manual click on level 2's first note registers a fresh `OK 1/10` right
after, no interference either direction.

## driving the browser, not by guessing

First live test of `emulation/autoplayer.yaml` surfaced three symptoms: no
sound on manual lemon presses, PLAY and LEVEL SELECT both doing nothing, and
the board starting armed on level 2 instead of level 1. v2 shipped two
plausible-sounding fixes (finger pins high-Z while idle instead of driven
HIGH; real external pull-ups + a boot guard for PLAY/LEVEL SELECT instead of
`INPUT_PULLUP`) — **both were reasoned from reading Velxio's source, neither
was actually tested, and neither fixed it.** Re-tested live: still broken.

That's when this stopped being code-reading and started being an actual
investigation: a headless-Playwright probe (harness's `.venv` already has
Playwright + a downloaded Chromium) driving the real editor — import the
`.vlx`, click Run, dispatch `button-press`/`button-release` custom events on
the `wokwi-pushbutton` elements, read `wokwi-led.brightness` and each board's
own Serial Monitor tab. Three real, confirmed-live root causes:

1. **`isBoardComponent()` checks kind-prefixes, not the spec's ids.** It's a
   hardcoded list (`arduino-uno`, `arduino-nano`, `esp32`, …) checked with
   `startsWith` — the working multi-board UART *template* only works because
   it leaves `id:` unset, which defaults to the kind string. This spec gave
   its boards custom ids (`piano`, `player`) for readability, which silently
   broke every pushbutton's own pin-trace **including piano's own 7 keys** —
   not just the new ones. Fixed: board ids reverted to the default
   `arduino-nano` / `arduino-uno`.
2. **Interconnect.ts only bridges wires where *both* endpoints are literal
   board components.** The finger wires landed on a passive (the pull-up
   resistor's node), which is invisible to it — nothing ever propagated
   between the boards, regardless of firmware, regardless of the resistor
   fix from v2. Fixed: every key + the finger wires are now direct
   board-pin-to-pin or board-pin-to-button wires; any pull-up resistor is a
   side branch off the same board pin, never in the button's own path (a
   resistor in that path also tunnels the legacy pin-tracer onto the 5V bus
   and dead-ends at -1/GND — confirmed in the `[verify]` log: a multi-board
   project never gets a real SPICE solve, `nodes:["0"]` vs. 35 real nodes for
   the single-board project, so every button falls back to that legacy
   tracer instead of the correct SPICE-resolved path).
3. **Component click handlers are bound to ONE global simulator, not one per
   board** (`DynamicComponent.tsx`: `useSimulatorStore(s => s.simulator)`).
   A pushbutton wired to the second board's pin calls `setPinState` on the
   FIRST board's CPU instead of its own — a real architectural limitation of
   this Velxio version, not something fixable from spec/wiring at all. Serial
   I/O doesn't have this problem (`serialWriteToBoard(boardId, text)` is
   properly per-board), so PLAY/LEVEL SELECT became serial commands instead
   of physical buttons: type `1`-`4` into the second board's own Serial
   Monitor tab to arm a level, then `p` to play it.

Also fixed along the way, and it does hold up: the finger pins must be a
driven `OUTPUT` even while idle (idle = `HIGH`), never high-Z `INPUT` — this
simulator doesn't model true floating inputs, so an idle input reads a
spurious LOW, and once the finger wires are the direct board-to-board wires
#2 required, that floating LOW gets bridged straight onto piano's key nodes
and permanently "presses" them.

**Verified live, this time**: typed `1` then `p` into the second board's
Serial Monitor → piano's own log showed `OK 1/10` through `OK 10/10` → `WIN`,
zero `WRONG`s.

**Also fixed, in the shared harness** (`velxio-multi-board-emulator`, not this
repo): `.vlx` generation assigned wire colors by *list position*, so a single
electrical node split across multiple wire segments (any junction — which
every key node in this spec now is, with 3 wires apiece) rendered in
different colors per segment. Rewrote `_wire_color` into a net-aware pass
(union-find over every wire's endpoints, classify GND/VCC/signal per net, one
color per net) — verified against the regenerated `autoplayer.vlx`: 88 wires,
49 real nets, zero inconsistent colors. Regenerated `lemon-piano.vlx` too
(still verifies green) so the existing hand-play project picks up the fix.

