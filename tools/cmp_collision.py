#!/usr/bin/env python3

import csv
from pathlib import Path

from PIL import Image

REPO = Path(__file__).parent.parent
RES = REPO / 'res'
SRC = REPO / 'src'
TGT_C = SRC / 'collision.c'
TGT_H = SRC / 'collision.h'


with open(TGT_H, 'w') as fouth:
    with open(TGT_C, 'w') as foutc:
        fouth.write('#ifndef COLLISION_H\n')
        fouth.write('#define COLLISION_H\n\n')
        fouth.write('#include <genesis.h>\n')
        fouth.write('\n')

        foutc.write('#include "collision.h"\n')
        foutc.write('\n')

        for fpath in RES.rglob('collision.png'):
            map_name = "bg"
            with Image.open(fpath) as img:
                assert img.height % 8 == 0 and img.width & 8 == 0
                rows = int(img.height / 8)
                cols = int(img.width / 8)
                array_decl = f'u8 COLLISION_{map_name.upper()}[{rows}][{cols}]'
                fouth.write(f'extern {array_decl};\n\n')
                foutc.write(f'{array_decl} = {{\n')
                t_array = []
                for ri in range(rows):
                    r_array = []
                    for ci in range(cols):
                        left = ci * 8
                        upper = ri * 8
                        right = left + 7
                        lower = upper + 7
                        tile = img.crop((left, upper, right, lower))
                        if all(p == 0 for p in tile.getdata()):
                            r_array.append(0)
                        else:
                            r_array.append(1)
                    t_array.append(r_array)
            foutc.write('    ')
            foutc.write(',\n    '.join('{' + ','.join(str(c) for c in r) + '}' for r in t_array))
            foutc.write('\n};\n\n')

        fouth.write('#endif')
