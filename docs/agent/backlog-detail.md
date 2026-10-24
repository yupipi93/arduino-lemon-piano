# Backlog detail: arduino-lemon-piano

Full text of the open sections, verbatim, moved from quantumpc on 2026-10-24.
The condensed entry is in `BACKLOG.md` under the same heading.

---

## 🍋 The lemon piano's enclosure v2 is designed and gated; printing is Sergio's call (2026-09-06)

Same day, second session: Sergio pushed caliper-annotated photos of the speaker, the
LM386 amp and a USB-C keep-alive gadget plus a layout and an openings sketch
(`arduino-lemon-piano/versions/v5.5-power-filter/model-design/reference-images/`), and
asked for the full enclosure, autonomously, on **pcb v0.7.1**. Done in
`~/code/3d-modeling-agent`: `recetas/piano-limones-caja-v2` (base + lid,
205.1 × 56.4 × 32.8, gate PASS, 62.57 + 26.85 g) and `recetas/piano-limones-luna-v2`
(fruit arc with the wire channels underneath, gate PASS, 37.57 g). Renders + STLs in each
brief's `modelo/` and copied to the piano repo's `model-design/enclosure-v2/`. The panel
buttons are the 6 mm threaded ones from his inventory
(`gcp-to-oracle-vm-proxy/tools/inventario/data/items.json`, `sw-momentary-6mm-*`).

**Nothing printed** — printing is a human decision in that repo. Before he does, three
parameters deserve a caliper: speaker frame height (`alt_y` 24.5 came from the photo),
speaker ear hole Ø (M3 in 2.8 assumed), and the socketed Nano's ICSP header height (sets
the 22 mm over the PCB). The v1 briefs stay as history.
