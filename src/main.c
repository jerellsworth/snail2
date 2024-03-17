#include "bh.h"

void wait(u8 frames) {
    for (u8 i = 0; i < frames; ++i) {
        SYS_doVBlankProcess();
        VDP_waitVSync();
    }
}

void results(Enc *enc) {
    VDP_setTextPalette(PAL3);
    u16 black = 0;
    PAL_setColor(63, 0);
    char buf[32];
    sprintf(buf, "skulls: %d", enc->score);
    VDP_drawTextBG(BG_B, buf, 14, 7);
    sprintf(buf, "frames: %d", enc->frames);
    VDP_drawTextBG(BG_B, buf, 14, 8);
    PAL_fadeTo(63, 63, &black, 30, FALSE);
    JOY_waitPressBtn();
    VDP_init();
}

int main(bool hard_reset) {
    if (!hard_reset) SYS_hardReset();

    SPR_init();
    JOY_init();
    snd_init();

    Menu *m = NULL;
    while (TRUE) {
        m = INTRO_run(m);
        Enc *e = Enc_run(m);
        results(e);
        Enc_del(e);
    }
	return 0;
}
