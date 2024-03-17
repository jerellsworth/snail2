#!/usr/bin/env python3

from io import StringIO
from pathlib import Path

REPO = Path(__file__).parent.parent
RES = REPO / 'res'
RESOURCES = RES / 'resources.res'
SRC = REPO / 'src'
TGT_C = SRC / 'sound.c'
TGT_H = SRC / 'sound.h'

sample_no = 64

buf_c = StringIO()
buf_h = StringIO()

buf_h.write('#ifndef SOUND_H\n')
buf_h.write('#define SOUND_H\n\n')
buf_h.write('#include "bh.h"\n\n')

buf_c.write('#include "bh.h"\n\n')
buf_c.write('void snd_init(void) {\n')

with open(RESOURCES) as fin:
    for row in fin:
        if not row.startswith("WAV"):
            continue
        const = row.split()[1]
        suffix = const.split('_', 1)[-1]
        buf_h.write(f'#define SND_SAMPLE_{suffix} {sample_no}\n')
        buf_c.write(f'    XGM_setPCM(SND_SAMPLE_{suffix}, {const}, sizeof({const}));\n')
        sample_no += 1

buf_h.write('\nvoid snd_init(void);\n\n')
buf_h.write('#endif\n')

buf_c.write('}\n')

TGT_H.write_text(buf_h.getvalue())
TGT_C.write_text(buf_c.getvalue())
