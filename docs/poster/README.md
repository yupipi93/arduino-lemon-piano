# Poster — the sign that goes behind the piano

`super-limon-piano-cartel.png` is the poster Sergio put behind the lemon piano so
that people at the event walk over and play it. **He made it with Gemini on
2026-09-13** and asked for it to be kept here with no loss of quality.

It is stored **byte for byte as it came**, never re-encoded:

```
sha256  b8db4412b7b9a77b31b51f8a2a371763dbf7a310eab1b599980af863dad18f26
bytes   1 010 859
```

Anything that opens and re-saves this file — an editor, a "compress PNG" tool, a
chat app — will produce different bytes. If that hash stops matching, the file
was touched and the original is gone.

## What is on it

Pixel-art Super Mario Bros. styling: the **SUPER LEMON PIANO** wordmark, Mario,
seven lemons on a brick ledge with a note over each one, **TOUCH THE FRUIT!**,
and the four levels as numbered cards — 1 mushroom, 2 Goomba, 3 star, 4 Bowser,
which is the order the firmware plays them in (Overworld, Underworld, Starman,
Castle). Footer: `MADE WITH ❤ BY MULTITEC UA`.

## Printing it — the honest numbers

```
848 x 1264 px, 8-bit RGB, no alpha
```

That resolution is fixed, and there is no clean way to enlarge it. It is pixel
art in *style* only — **102 156 distinct colours**, with soft, anti-aliased
edges — so a nearest-neighbour upscale (which is lossless on real pixel art)
would only magnify the softness. What comes out of the printer depends entirely
on how big it is printed:

| printed at | effective | how it looks |
|---|---|---|
| 72 × 107 mm | 300 dpi | crisp at arm's length |
| 108 × 161 mm | 200 dpi | crisp |
| 144 × 214 mm | 150 dpi | fine for a poster |
| **210 × 297 mm (full A4)** | **≈ 102 dpi** | **soft up close, fine from 1 m away** |

100 dpi is normal for large-format printing — nobody reads a poster with their
nose against it. Full A4 is a reasonable choice for this one.

**One thing to expect:** the image is **1 : 1.49** and A4 is **1 : 1.41**, so it
is slightly taller than the page. "Fit to page" leaves about **10 mm of white
down each side**; "fill page" crops about **16 mm off the top and bottom**, which
would eat into the wordmark. The white margins are the safer choice.

## The other poster

There is a second, unrelated poster: a vector-quality A4 built here from HTML and
rendered at a true 2480 × 3508 px (300 dpi), with the same four levels but a
Spanish `¡TOCA LA FRUTA!` and real photographed lemons. Sergio went with the
Gemini one instead. It lives outside git at `~/Pictures/piano-limones/` on
quantumpc — say the word and it can be brought in here too.
