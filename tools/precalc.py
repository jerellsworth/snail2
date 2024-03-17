#!/usr/bin/env python3

from math import sqrt
from io import StringIO
from pathlib import Path

from numpy import arccos, pi

REPO = Path(__file__).parent.parent
RES = REPO / 'res'
SRC = REPO / 'src'
TGT_C = SRC / 'precalc.c'

buf = StringIO()

buf.write('#include "bh.h"\n\n')

# find PC_ACOS_POS[some_fix_16 & FIX16_FRAC_MASK]
buf.write('u16 PC_ACOS_POS[64] = {\n  ')
vals = []
for i in range(64):
    cos = float(i)/64
    acos = arccos(cos) / (2 * pi) * 1024
    vals.append(str(round(acos)))
buf.write(',\n  '.join(vals));
buf.write("\n};\n\n")

# find PC_ACOS_POS[some_fix_16 & FIX16_FRAC_MASK]
buf.write('u16 PC_ACOS_NEG[64] = {\n  ')
vals = []
for i in range(64):
    cos = -1 + (float(i)/64)
    acos = arccos(cos) / (2 * pi) * 1024
    vals.append(str(round(acos)))
buf.write(',\n  '.join(vals));
buf.write("\n};\n\n")
TGT_C.write_text(buf.getvalue())

""" fix16 represenation!
1          -> 00000000 01000000

-1         -> 11111111 11000000

1 - eps    -> 00000000 00111111

-(1 - eps) -> 11111111 11000001

start at 0 -> 00000000 00000000
advance by 1/32...
go to 1    -> 00000000 01000000
"""
