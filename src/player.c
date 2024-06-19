# include "bh.h"

Player *Player_new(
        u8 joy
    ) {
    Player *pl = ct_calloc(1, sizeof(Player));
    pl->joy = joy;
    pl->cooldown = 0;
    return pl;
}

void Player_del(Player *p) {
    free(p);
}

void Player_input(Player *pl, Enc *e) {
    Phy *p = pl->p;
    if (!p) return;
    /*
    if (pl->ai_level > 0) {
        ai(pl, e);
        return;
    }
    */
    u16 joy = JOY_readJoypad(pl->joy);
    if (pl->cooldown > 0) --pl->cooldown;
    if (pl->cooldown2> 0) --pl->cooldown2;

    if (e->state == ENC_RUNNING) {

        if (!p) return;
        if ((joy & BUTTON_START) && (pl->cooldown == 0)) {
            e->state = ENC_PAUSED;
            pl->cooldown = 10;
            XGM_startPlayPCMNextCh(SND_SAMPLE_PAUSE, 14);
            return;
        }
        switch (p->what) {
            default:
                return;
        }
    } else if (e->state == ENC_PAUSED) {
        if ((joy & BUTTON_START) && (pl->cooldown == 0)) {
            e->state = ENC_RUNNING;
            pl->cooldown = 10;
            XGM_startPlayPCMNextCh(SND_SAMPLE_PAUSE, 14);
            return;
        }
    }
}
