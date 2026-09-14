# The organiser's booklet

One A4 sheet, black and white, printed on both sides and **folded down the
middle into an A5**. It exists so that someone who has never seen the lemon
piano can switch it on, explain it, and rescue it — without Sergio in the room.

```
./build.sh            # -> cartilla.pdf
./build.sh --print    # ...and send it to the laser
```

## The four panels

| | |
|---|---|
| **Front** (right half of side 1) | the title, the seven lemons, *Guía del organizador* |
| **Back** (left half of side 1) | what the two buttons do · **si algo va mal** |
| **Inside left** | how the game works · the two questions everyone asks about the finger · what the sounds mean |
| **Inside right** | **the four codes**, and how to tell which lemon is number 1 |

Almost no prose. Everything that can be a picture is one, and the picture is the
**LED bar itself** — ten boxes, filled or empty — because that is the only thing
the piano can say and the only thing a helper has to read.

## The codes are derived, never typed

`seqs.py` is the point of this directory. A level's code lives in the firmware as
note **frequencies**, and which lemon plays a given note is different on every
level — each one has its own seven-note keyboard row. So the answer a player
needs ("touch 6, then 5, then 6…") exists nowhere in the source: it has to be
resolved, one table against the other.

Typing the result into the template by hand would be a second source of truth
that nothing checks, and it would go stale the first time a melody changed.
Instead `seqs.py` reads both tables out of `main.cpp`, resolves them, and
**cross-checks level 1 against the sequence hard-coded in the host test**, which
was written independently. It exits non-zero rather than guess: a level whose
code asks for a note that is not on its own keyboard is unplayable, and that is
a failure, not a warning.

**So: change a melody in `main.cpp`, re-run `./build.sh`, and the booklet is
right again.** The committed `cartilla.pdf` is a convenience for reprinting —
if the firmware's melodies have moved since, rebuild before printing.

## Printing it

The content is **A4 landscape**, so the side that reads like a book is the one
flipped about the paper's **short** edge:

```bash
lp -d laser -n 1 -o media=A4 -o ColorModel=Gray -o sides=two-sided-short-edge cartilla.pdf
```

Long-edge duplex puts the inside spread upside down relative to the cover. If a
copy comes out that way, that is the setting to change, not the file.

## Two notes on the type

- **Press Start 2P has no accented capitals** — it draws `Í` as `í`. So the
  pixel face is used only where Spanish needs no accent (the cover title,
  `NIVEL n`, the code digits) and everything else is DejaVu in caps. The first
  draft ignored this and printed `CóMO SE JUEGA`.
- Everything is **pure black on white**, no greys except the fold line. A laser
  printer renders a 50 % grey as a dot screen, which at 8 pt turns body text to
  mush.
