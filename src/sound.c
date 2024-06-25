#include "bh.h"

void snd_init(void) {
    XGM_setPCM(SND_SAMPLE_PAUSE, WAV_PAUSE, sizeof(WAV_PAUSE));
    XGM_setPCM(SND_SAMPLE_EXPLOSION, WAV_EXPLOSION, sizeof(WAV_EXPLOSION));
}
