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
    u16 joy = JOY_readJoypad(pl->joy);
    Phy *p = pl->p;
    if (!p) return;
    if (pl->cooldown > 0) --pl->cooldown;
    if (pl->cooldown2> 0) --pl->cooldown2;

    if (pl->cooldown == 0 && (joy & BUTTON_START)) {
        // TODO  dbg
        /*
        if (joy & BUTTON_A) {
            ++e->level;
            Enc_load_level(e);
            pl->cooldown = 10;
            return;
        }
        */

        XGM_startPlayPCM(SND_SAMPLE_PICKUP, 15, SOUND_PCM_CH2);
        e->state = e->state == ENC_PAUSED ? ENC_RUNNING : ENC_PAUSED;
        pl->cooldown = 10;
    }

    if (e->state == ENC_RUNNING) {

        if (!p) return;
        switch (p->what) {
            case WHAT_HUNTER:
                p->dx = 0;
                p->dy = 0;
                switch (p->state) {
                    case 0: // neutral
                        if (joy & BUTTON_LEFT) {
                            SPR_setHFlip(p->sp, TRUE);
                            p->f = LEFT;
                            p->dx = FIX16(-1);
                        } else if (joy & BUTTON_RIGHT) {
                            SPR_setHFlip(p->sp, FALSE);
                            p->f = RIGHT;
                            p->dx = FIX16(1);
                        }
                        if (joy & BUTTON_UP) {
                            p->dy = FIX16(-1);
                        } else if (joy & BUTTON_DOWN) {
                            p->dy = FIX16(1);
                        }

                        if ((joy & BUTTON_B) && pl->cooldown == 0 && p->dash_frames == 0) {
                            if (p->dx == 0 && p->dy == 0) p->dx = p->f == LEFT ? FIX16(-1) : FIX16(1);
                            fix16 norm_dx, norm_dy;
                            normalize(p->dx, p->dy, FIX16(8), &norm_dx, &norm_dy);
                            p->dash_dx = norm_dx;
                            p->dash_dy = norm_dy;
                            p->dash_frames = 8;
                            pl->cooldown = 20;
                            XGM_startPlayPCM(SND_SAMPLE_DASH, 15, SOUND_PCM_CH2);
                        } else if (joy & (BUTTON_A | BUTTON_C) && pl->cooldown == 0 && pl->p->live_spawn < 3) {
                            fixx x = p->f == LEFT ? p->x - FIXX(12) : p->x + FIXX(p->w) + FIXX(10);
                            fixy y = p->col_y - FIXY(4);
                            if (BG_collide(e->bg, x, y, 0, 0, FALSE))  {
                                XGM_startPlayPCM(SND_SAMPLE_NO, 15, SOUND_PCM_CH2);
                                return;
                            }
                            SPR_setAnim(p->sp, 5);
                            p->state = 1;
                            p->state_frames = 0;
                        }
                        if (pl->cooldown == 0) {
                            if (p->dx != 0 || p->dy != 0) {
                                SPR_setAnim(p->sp, 1);
                            } else {
                                SPR_setAnim(p->sp, 0);
                            }
                        }

                        return;
                    case 1: // pulling back;
                        if (p->state_frames > 5)  {
                            SPR_setAnim(p->sp, 6);
                            p->state = 2;
                            p->state_frames = 0;
                            p->charged = FALSE;
                            if (p->f == LEFT) {
                                p->theta = 512;
                            } else {
                                p->theta = 0;
                            }
                            p->partner = Physics_new_bolt(
                                p->f == LEFT ? p->x - FIXX(4) : p->x + FIXX(p->w) + FIXX(2),
                                p->col_y - FIXY(4),
                                p->theta,
                                NULL,
                                TRUE
                                );
                            p->partner->partner = p;
                            SPR_setAlwaysOnTop(p->partner->sp);
                            XGM_startPlayPCM(SND_SAMPLE_PULL_BACK, 15, SOUND_PCM_CH2);
                            return;
                        }
                        if (!(joy & (BUTTON_A | BUTTON_C))) {
                            // cancel
                            SPR_setAnim(p->sp, 0);
                            p->state = 0;
                            p->state_frames = 0;
                            pl->cooldown = 10;
                            return;
                        }
                        return;
                    case 2: //  held;
                        if (p->state_frames >= 60 && (!p->charged)) {
                            p->charged = TRUE;
                            SPR_setAnim(p->partner->sp, p->partner->sp->animInd + 9);
                            XGM_startPlayPCM(SND_SAMPLE_CHARGE, 15, SOUND_PCM_CH2);
                        }
                        if (pl->cooldown2 == 0) {
                            if (joy & (BUTTON_RIGHT | BUTTON_LEFT)) {
                                u16 next_theta = p->theta;
                                if (joy & BUTTON_RIGHT) {
                                    next_theta += 64;
                                    if (next_theta >= 1024) next_theta = 0;
                                } else if (joy & BUTTON_LEFT) {
                                    if (next_theta <= 0) {
                                        next_theta = 1024 - 64;
                                    } else {
                                        next_theta -= 64;
                                    }
                                }
                                Facing next_f;
                                if (next_theta > 256 && next_theta < 768) {
                                    next_f = LEFT;
                                } else {
                                    next_f = RIGHT;
                                }
                                if (p->f != next_f && next_theta != 256 && next_theta != 768) {
                                    fixx next_x = next_f == LEFT ? p->x - FIXX(4) : p->x + FIXX(p->w) + FIXX(2);
                                    if (BG_collide(e->bg, next_x + (next_f == LEFT ? FIXX(-8) : FIXX(8)) , p->partner->y, 0, 0, FALSE))  {
                                        XGM_startPlayPCM(SND_SAMPLE_NO, 15, SOUND_PCM_CH2);
                                        return;
                                    }
                                    p->f = next_f;
                                    SPR_setHFlip(p->sp, p->f == LEFT);
                                    p->partner->x = next_x;
                                }
                                p->theta = next_theta;
                                Physics_set_bolt_anim(p->partner, p->theta);
                                if (p->charged) {
                                    SPR_setAnim(p->partner->sp, p->partner->sp->animInd + 9);
                                }
                                pl->cooldown2 = 10;
                            }
                        }
                        if (joy & BUTTON_B) {
                            Physics_del(p->partner, e);
                            p->partner = NULL;
                            p->state = 3;
                            p->state_frames = 0;
                            pl->cooldown = 20;
                        }
                        if (!(joy & (BUTTON_A | BUTTON_C))) { // release
                            Phy *bolt = Physics_new_bolt(
                                p->partner->x,
                                p->partner->y,
                                p->theta,
                                &(pl->p->live_spawn),
                                FALSE
                                );
                            if (p->charged) {
                                SPR_setAnim(bolt->sp, bolt->sp->animInd + 9);
                            }
                            bolt->state = p->charged ? 2 : 0;
                            bolt->bouncy = !p->charged;
                            bolt->drop_on_collide = p->charged;
                            Physics_del(p->partner, e);
                            p->partner = NULL;
                            bolt->partner = p;
                            pl->cooldown = 10;
                            p->state = 3;
                            p->state_frames = 0;
                            SPR_setAnim(p->sp, 7);
                            XGM_startPlayPCM(SND_SAMPLE_ARROW_SHOOT, 15, SOUND_PCM_CH2);
                        }
                        return;
                    case 3: // released
                        if ((p->state_frames > 30) || (joy & BUTTON_ALL)) {
                            p->state = 0;
                            p->state_frames = 0;
                            SPR_setAnim(p->sp, 0);
                        }
                        return;
                    default:
                        return;
                }
            default:
                return;
        }
    }
}
