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

Phy *ai_find_nearby(Phy *p1, Thing what, u32 *dist, Player *pl) {
    Phy *p_winner = NULL;
    u32 dist_winner = 0xFFFFFFFF;
    u32 thresh = 112 * 80;
    for (int i = 0; i < MAX_PHYSICS_OBJS; ++i) {
        Phy *p2 = ALL_PHYSICS_OBJS[i];
        if (p2 == NULL) continue;
        if (p1 == p2) continue;
        if (p2->what != what) continue;
        bool match = FALSE;
        for (u8 tgt_idx = 0; tgt_idx < PLAYER_AI_PREV_TARGET_MEMORY; ++tgt_idx) {
            if (p2 == pl->ai_prev_tgts[tgt_idx]) {
                match = TRUE;
                break;
            }
        }
        if (match) continue;
        u32 this_dist = Physics_dist(p1, p2);
        if (this_dist <= thresh) {
            *dist = this_dist;
            return p2;
        }
        if (this_dist < dist_winner) {
            dist_winner = this_dist;
            p_winner = p2;
        }
    }
    *dist = dist_winner;
    return p_winner;
};

fix16 ai_random_jitter(void) {
    return FIX16(4 - random_with_max(8)) >> 4;
}

void ai_walk_toward(Phy *src, Phy *tgt, bool walk_away) {
    fixx tgtx = tgt->x + tgt->hbox_offset_x;
    fixy tgty = tgt->y + tgt->hbox_offset_y;
    fixx srcx = src->x + src->hbox_offset_x;
    fixy srcy = src->y + src->hbox_offset_y;

    fix16 dx = tgtx - srcx;
    fix16 dy = tgty - srcy;

    if (dx < FIX16(-2)) dx = FIX16(-2);
    if (dx > FIX16(2)) dx = FIX16(2);
    if (dy < FIX16(-2)) dy = FIX16(-2);
    if (dy > FIX16(2)) dy = FIX16(2);

    if (walk_away) {
        dx = -dx;
        dy = -dy;
    }
    SPR_setHFlip(src->sp, dx < 0);
    fix16 normx, normy;
    normalize(dx, dy, FIX16(2), &normx, &normy);
    src->dx = normx + ai_random_jitter();
    src->dy = normy + ai_random_jitter();
    SPR_ensureAnim(src->sp, 1);
}

void ai_prep_throw(Phy *src, Phy *tgt, u32 _dist_sq) {
    src->dx = 0;
    src->dy = 0;

    fix16 dx = (tgt->x + tgt->hbox_offset_x) - (src->x + src->hbox_offset_x);
    fix16 dy = (tgt->y + tgt->hbox_offset_y) - (src->y + src->hbox_offset_y);
    SPR_setHFlip(src->sp, dx < 0);
    src->f = dx < 0 ? LEFT : RIGHT;

    fix32 dx32 = fix16ToFix32(dx);
    fix32 dy32 = fix16ToFix32(dy);
    fix32 dist_sq = fix32Mul(dx32, dx32) + fix32Mul(dy32, dy32);
    fix16 dist_log_2 = adaptiveFix32Log2(dist_sq);
    fix16 v = exp2((dist_log_2 >> 2) + VELOCITY_COEF);
    fix16 vz = fix16Mul(v, SIN_PI_DIV_3);
    fix16 vxy = fix16Mul(v, COS_PI_DIV_3);

    fix16 norm_dx, norm_dy;
    normalize(dx, dy, FIX16(1), &norm_dx, &norm_dy);
    u16 T = arccossin(norm_dx, norm_dy);

    fix16 vy = fix16Mul(vxy, sinFix16(T));
    fix16 vx = fix16Mul(vxy, cosFix16(T));

    src->ai_flag = TRUE;
    src->pending_throw_dx = fix16Mul(vx, FIX16(1.1)) + ai_random_jitter();
    if (dx > 0) src->pending_throw_dx = - src->pending_throw_dx;
    if (dy < 0) src->pending_throw_dy = - src->pending_throw_dy;
    src->pending_throw_dy = fix16Mul(vy, FIX16(1.1)) + ai_random_jitter();
    src->pending_throw_dz = fix16Mul(vz, FIX16(1.1));
    src->state = 1;
    src->state_frames = 0;
    SPR_ensureAnim(src->sp, 2);
}

void ai(Player *pl, Enc *e) {
    Phy *p = pl->p;
    if (pl->walking_away_frames > 0) {
        --pl->walking_away_frames;
        return;
    }
    if (p->blocked && pl->walking_away_frames == 0) {
        p->dx = -p->dx;
        p->dy = -p->dy;
        pl->walking_away_frames = 2;
        p->blocked = FALSE;
        return;
    }
    if (pl->ai_next_decision > 0) {
        --pl->ai_next_decision;
        return;
    }
    if (p->state != 0) return; // we're in mid throw. Nothing to change

    u32 dist;

    // TODO dbg
    //Phy *p_tgt = ai_find_nearby(p, WHAT_GOBLIN, &dist);

    // TODO keep track of last 3 targets rather than just the last 1
    Phy *p_tgt = NULL;
    p_tgt = ai_find_nearby(p, WHAT_DYNAMITE, &dist, pl);
    if (dist > 100 && (random_with_max(8))) {
        p_tgt = ai_find_nearby(p, WHAT_CASH, &dist, pl);
        if (
            (!p_tgt) || 
            (dist > 112 * 80 * 2 && (random_with_max(2)))
            ) {
            p_tgt = ai_find_nearby(p, WHAT_GOBLIN, &dist, pl);
            if (
                (!p_tgt) || 
                (dist > 112 * 80 * 2 && (random_with_max(2)))
                ) {
                p_tgt = ai_find_nearby(p, WHAT_PROP, &dist, pl);
            }
        }
    }

    if (!p_tgt) {
        for (u8 tgt_idx = 0; tgt_idx < PLAYER_AI_PREV_TARGET_MEMORY; ++tgt_idx) {
            if (pl->ai_prev_tgts[tgt_idx] != NULL) {
                p_tgt = pl->ai_prev_tgts[tgt_idx]; 
                break;
            }
        }
    }
    pl->ai_next_decision = random_with_max(43);
    pl->ai_prev_tgts[pl->ai_prev_tgts_idx] = p_tgt;
    pl->ai_prev_tgts_idx = (pl->ai_prev_tgts_idx + 1) % PLAYER_AI_PREV_TARGET_MEMORY;

    // TODO dbg
    /*
    if (p->cursor) {
        SPR_setPosition(p->cursor, fix16ToRoundedInt(p_tgt->x), fix16ToRoundedInt(p_tgt->y));
    }
    */

    if (p_tgt->what == WHAT_DYNAMITE) {
        ai_walk_toward(p, p_tgt, TRUE);
        // actually walk away
        return;
    }
    if (p_tgt->what == WHAT_CASH) {
        ai_walk_toward(p, p_tgt, FALSE);
        return;
    }

    // TODO dbg
    /*
    ai_prep_throw(p, p_tgt, dist);
    return; 
    */

    if (dist < 512 && p_tgt->what == WHAT_GOBLIN) {
        if (!random_with_max(2)) {
            ai_walk_toward(p, p_tgt, TRUE);
        } else {
            ai_prep_throw(p, p_tgt, dist);
        }
    } else if (dist < 512) {
        if (!random_with_max(1)) {
            ai_walk_toward(p, p_tgt, TRUE);
        } else {
            pl->ai_next_decision = 0;
        }
    }
    else if (dist < 60 * 60 * 4) {
        ai_prep_throw(p, p_tgt, dist);
    } else {
        ai_walk_toward(p, p_tgt, FALSE);
    }
}

void Player_input(Player *pl, Enc *e) {
    Phy *p = pl->p;
    if (!p) return;
    if (pl->ai_level > 0) {
        ai(pl, e);
        return;
    }
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
            case WHAT_GOBLIN:
                p->dx = 0;
                p->dy = 0;
                if (p->state == 0) {
                    bool walking = FALSE;
                    if (joy & BUTTON_RIGHT) {
                        SPR_setHFlip(p->sp, FALSE);
                        p->f = RIGHT;
                        p->dx = FIXX(2);
                        walking = TRUE;
                    } else if (joy & BUTTON_LEFT) {
                        SPR_setHFlip(p->sp, TRUE);
                        p->f = LEFT;
                        p->dx = FIXX(-2);
                        walking = TRUE;
                    } 
                    if (joy & BUTTON_UP) {
                        p->dy = FIXY(-2);
                        walking = TRUE;
                    } else if (joy & BUTTON_DOWN) {
                        p->dy = FIXY(2);
                        walking = TRUE;
                    }
                    SPR_ensureAnim(p->sp, walking ? 1 : 0);
                    if (joy & BUTTON_ACTION && pl->cooldown == 0) {
                        p->state = 1;
                        p->state_frames = 0;
                        SPR_ensureAnim(p->sp, 2);
                        pl->cooldown = 20;
                        p->charged = 1;
                        p->english = 0;
                    }
                } else if (p->state == 1) {
                    if (joy & BUTTON_ACTION) {
                        p->charged += 1;
                    }
                    if (joy & BUTTON_UP) {
                        p->english -= 1;
                    } else if (joy & BUTTON_DOWN) {
                        p->english += 1;
                    }
                }
                return;
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
