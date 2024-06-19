#include "bh.h"

void snd_init(void) {
    XGM_setPCM(SND_SAMPLE_PAUSE, WAV_PAUSE, sizeof(WAV_PAUSE));
    XGM_setPCM(SND_SAMPLE_HORN, WAV_HORN, sizeof(WAV_HORN));
    XGM_setPCM(SND_SAMPLE_EXPLOSION, WAV_EXPLOSION, sizeof(WAV_EXPLOSION));
    XGM_setPCM(SND_SAMPLE_CASH, WAV_CASH, sizeof(WAV_CASH));
    XGM_setPCM(SND_SAMPLE_LAUGH, WAV_LAUGH, sizeof(WAV_LAUGH));
    XGM_setPCM(SND_SAMPLE_TICK, WAV_TICK, sizeof(WAV_TICK));
}