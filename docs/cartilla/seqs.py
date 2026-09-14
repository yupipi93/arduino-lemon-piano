#!/usr/bin/env python3
"""Derive each level's ten-key code FROM THE FIRMWARE, never by hand.

A level's code is stored as note FREQUENCIES, and which lemon plays a given note
is different on every level — each level has its own seven-note keyboard row.
So the answer a player needs ("touch 6, then 5, then 6...") exists nowhere in
the source: it has to be resolved, one table against the other.

Typing the result into the booklet by hand would create a second source of
truth that nothing checks, and it would go stale the first time a melody
changes. This reads both tables out of main.cpp instead, and cross-checks
level 1 against the hard-coded sequence in the host test, which was written
independently.

    ./seqs.py [out.json]     # prints the codes, writes the JSON the booklet uses
"""
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, '..', '..'))
FW = os.path.join(REPO, 'versions', 'v5.5-power-filter', 'firmware')
SRC = os.path.join(FW, 'src', 'main.cpp')
TEST = os.path.join(FW, 'test', 'piano_sim_test.cpp')

NOTE = r'NOTE_[A-G]S?\d'


def main(dst):
    src = open(SRC).read()

    # the keyboard: 5 rows of 7 notes (4 levels + free play)
    m = re.search(r'const int keys\[[^\]]*\] = \{(.*?)\n\};', src, re.S)
    if not m:
        sys.exit('ERROR: the keys[] table moved or changed shape')
    names = re.findall(NOTE, re.sub(r'//.*', '', m.group(1)))
    if len(names) != 35:
        sys.exit('ERROR: expected 35 key notes, found %d' % len(names))
    rows = [names[i * 7:(i + 1) * 7] for i in range(5)]

    codes = {}
    for lv in (1, 2, 3, 4):
        m = re.search(r'const int sequence_%d\[[^\]]*\] = \{([^}]*)\}' % lv, src)
        if not m:
            sys.exit('ERROR: sequence_%d is missing' % lv)
        seq = re.findall(NOTE, m.group(1))
        if len(seq) != 10:
            sys.exit('ERROR: level %d has %d notes, not 10' % (lv, len(seq)))
        row = rows[lv - 1]
        keys = []
        for note in seq:
            if note not in row:
                sys.exit('ERROR: level %d asks for %s, which is not on its own '
                         'keyboard - the code would be unplayable' % (lv, note))
            keys.append(row.index(note) + 1)        # 1-based, the way a player counts
        codes[lv] = {'keys': keys, 'row': row, 'notes': seq}

    # independent cross-check: the host test hard-codes level 1 as 0-based indices
    m = re.search(r'const int code\[10\] = \{([^}]*)\}', open(TEST).read())
    if m:
        from_test = [int(x) + 1 for x in m.group(1).split(',')]
        if from_test != codes[1]['keys']:
            sys.exit('ERROR: level 1 resolves to %s but the host test plays %s'
                     % (codes[1]['keys'], from_test))
        print('cross-check against the host test, level 1: OK', file=sys.stderr)

    for lv in (1, 2, 3, 4):
        print('level %d: %s' % (lv, '  '.join(map(str, codes[lv]['keys']))),
              file=sys.stderr)
    json.dump(codes, open(dst, 'w'), indent=1)


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, 'codes.json'))
