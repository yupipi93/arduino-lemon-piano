# Backlog: arduino-lemon-piano

Open task state between sessions. Newest first.

Moved verbatim from `quantumpc-management-agent/docs/agent/BACKLOG.md` on 2026-10-24, when Sergio asked
for each project to keep its own backlog. Some tasks name tools in that repo's `bin/` (`qpc-*`): the task
lives here, the fix lands where the code is.

---

## 🍋 The lemon piano's enclosure v2 is designed and gated; printing is Sergio's call (2026-09-06)

**Estado:** both parts designed and gated in `~/code/3d-modeling-agent`: `recetas/piano-limones-caja-v2` (base + lid, 205,1 x 56,4 x 32,8, gate PASS) and `recetas/piano-limones-luna-v2` (fruit arc, gate PASS), renders and STLs copied to the piano repo's `model-design/enclosure-v2/`. Nothing printed.
**Siguiente:** needs Sergio: printing is a human decision in that repo. Before he does, three parameters deserve a caliper: speaker frame height (`alt_y` 24.5, taken from a photo), speaker ear hole diameter (M3 in 2.8 assumed), and the socketed Nano's ICSP header height, which sets the 22 mm over the PCB.
Built on pcb v0.7.1; panel buttons are the 6 mm threaded ones from his inventory.

Detalle completo: `docs/agent/backlog-detail.md`
