#include "bh.h"

void wait(u8 frames) {
    for (u8 i = 0; i < frames; ++i) {
        SYS_doVBlankProcess();
        VDP_waitVSync();
    }
}

void results(Score **scores, u8 round) {
    VDP_init();

    PAL_setPalette(PAL1, PAL_SPRITE.data, DMA);
    PAL_setPalette(PAL3, PAL_SPRITE_ALT.data, DMA);

    // TODO fade up
    VDP_drawImage(BG_B, &IMG_RESULTS, 0, 0);
    u16 black = 0;

    char buf[64];
    if (round >= 2) {
        VDP_drawTextBG(BG_B, "          FINAL ROUND RESULTS          ", 0, 0);
        XGM_startPlay(XGM_RESULTS);
    } else {
        sprintf(buf, "            ROUND %d RESULTS             ", round + 1);
        VDP_drawTextBG(BG_B, buf, 0, 0);
    }
    VDP_drawTextBG(BG_B, "     RANK    CASH   PREVIOUS  GAME TOTAL", 0, 2);
    u16 y = 3;
    for (u8 pl_no = 0; pl_no < 4; ++pl_no) {
        Score *s = scores[pl_no];
        u8 pal;
        const SpriteDefinition* spr_def;

        switch (s->player_no) {
            case 0:
                spr_def = &SPR_GOBLIN;
                pal = PAL1;
                break;
            case 1:
                spr_def = &SPR_GOBLIN2;
                pal = PAL3;
                break;
            case 2:
                spr_def = &SPR_GOBLIN3;
                pal = PAL3;
                break;
            case 3:
                spr_def = &SPR_GOBLIN4;
                pal = PAL3;
                break;
            default:
                break;
        }
        SPR_addSprite(
            spr_def,
            0,
            y * 8,
            TILE_ATTR(pal, TRUE, FALSE, FALSE)
            );
        sprintf(
            buf,
            "     %2d(+%2d)  +%2d       +%2d         %2d  ",
            s->rank,
            s->rank_score,
            s->cash_score,
            s->previous_score,
            s->rank_score + s->cash_score + s->previous_score
            );
        VDP_drawTextBG(BG_B, buf, 0, y+1);
        y += 5;

    }
    while (TRUE) {
        if (JOY_readJoypad(JOY_ALL)) break;
        SPR_update();
        SYS_doVBlankProcess();
    }
    JOY_waitPressBtn();
    PAL_fadeTo(63, 63, &black, 30, FALSE);
    SPR_init();
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

        Score *scores[4];
        for (u8 i = 0; i < 4; ++i) scores[i] = Score_new(i);
        XGM_stopPlay();

        for(u8 round = 0; round < 3; ++ round) {
            Enc *e = Enc_run(m);

            for (u8 pl_no = 0; pl_no < 4; ++pl_no) {
                Player *pl = e->players[pl_no];
                Score *s = scores[pl_no];
                Score_tally(s, pl);
            }

            results(scores, round);
            Enc_del(e);
        }
    }
	return 0;
}
