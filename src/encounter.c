#include "bh.h"

void Enc_reset_pc(Enc *e, Player *pl, bool death, u8 iframes) {
    if (pl->p) {
        for (int i = 0; i < MAX_PHYSICS_OBJS; ++i) {
            Physics *p = ALL_PHYSICS_OBJS[i];
            if (p && p->partner == pl->p) {
                Physics_del(p, e);
            }
        }
        XGM_startPlayPCM(SND_SAMPLE_PLAYER_HIT, 14, SOUND_PCM_CH4);
        Physics_del(pl->p, e);
    }
    if (death) {
        --e->lives;
        if (e->lives == 0) {
            BG_reset_fx(e->bg);
            e->state = ENC_COMPLETE;
            return;
        }
        e->bg->degradation += 3;
    }
    Phy *hunter;
    fixx x;
    const SpriteDefinition *spriteDef;
    if (pl->player_no == 1) {
        x = FIXX(32);
        spriteDef = &SPR_HUNTER;
    } else {
        x = FIXX(320 - 32);
        spriteDef = &SPR_HUNTER2;
    }
    hunter = Physics_new_hunter(x, FIXY(64), spriteDef);
    hunter->pl = pl;
    hunter->iframes = iframes;
    if (iframes == 0) {
        SPR_setPalette(hunter->sp, PAL3);
    }
    pl->p = hunter;
    if (pl->player_no == 1) {
        e->hunter = hunter;
    } else {
        e->hunter2 = hunter;
    }
    e->level_frames = 0;
}

bool Enc_check_position(Enc *e, fixx x, fixy y, u16 w, u16 h) {
    fixx fix_w = FIXX(w - 1);
    fixy fix_h = FIXY(h - 1);
    if (BG_collide(e->bg, x, y, 0, 0, FALSE)) return FALSE;
    if (BG_collide(e->bg, x + fix_w, y, 0, 0, FALSE)) return FALSE;
    if (BG_collide(e->bg, x + fix_w, y + fix_h, 0, 0, FALSE)) return FALSE;
    if (BG_collide(e->bg, x, y + fix_h, 0, 0, FALSE)) return FALSE;
    return TRUE;
}


Enc *Enc_new(u8 n_players) {
    Enc *e = ct_calloc(1, sizeof(Encounter));
    e->n_players = n_players;
    e->state = ENC_RUNNING;

    //PAL_setPalette(PAL1, PAL_SKULL.data, DMA);

    e->bg = BG_init(
        &MAP_BG,
        &TLS_BG,
        &COLLISION_BG,
        PAL_BG.data
        );

    e->players[0] = Player_new(JOY_1);
    e->players[0]->player_no = 1;
    e->players[1] = Player_new(JOY_2);
    e->players[1]->player_no = 2;
    e->lives = 5;
    Enc_load_level(e);

    return e;
}

void Enc_del(Enc *e) {
    BG_del(e->bg);
    free(e);
}

void Enc_cleanup(Enc *e) {
    u16 white[16];
    memset(white, 255, 32);
    PAL_fadeTo(0, 63, white, 60, FALSE);
    Physics_del_all(e);
    SPR_reset();
    SPR_update();
    SYS_doVBlankProcess();
    XGM_stopPlayPCM(SOUND_PCM_CH1);
}

void Enc_update(Enc *e) {
    if (e->music_on && !(e->frames & 15) && !XGM_isPlayingPCM(SOUND_PCM_CH1_MSK)) {
        switch (e->song) {
            case 0:
                if (e->song_frames >= 300) {
                    XGM_startPlayPCM(SND_SAMPLE_SONG_1, 15, SOUND_PCM_CH1);
                    e->song = 1;
                    e->song_frames = 0;
                }
                break;
            case 1:
                e->song = 2;
                e->song_frames = 0;
                break;
            case 2:
                if (e->song_frames >= 600) {
                    XGM_startPlayPCM(SND_SAMPLE_SONG_2, 15, SOUND_PCM_CH1);
                    e->song = 3;
                    e->song_frames = 0;
                }
                break;
            case 3:
                e->song = 0;
                e->song_frames = 0;
                break;
            default:
                break;
        }
    }
    ++e->song_frames;
    if (e->state == ENC_PAUSED) return;
    if (e->enemy_counter <= 0) {
        ++e->level;
        Enc_load_level(e);
    }
    if (e->ms) {
        MSEG_update(e->ms, e);
    }
    if (!(e->frames & 1023) &&  e->lives < 4 && (!(e->ms))) {
        if (TRUE) {
            e->ms = MSEG_new(0, FIXY(16 + random_with_max(224 - 16 - 32)));
            XGM_startPlayPCM(SND_SAMPLE_WORMCALL, 15, SOUND_PCM_CH4);
        }
    }
    if (e->level >= 1 && e->ball_counter == 0 && e->level_frames >= 20 * 60) {
        Enc_make_ball(e, 1);
        XGM_startPlayPCM(SND_SAMPLE_BALL, 15, SOUND_PCM_CH4);
    }
    ++e->frames;
    ++e->state_frames;
    ++e->level_frames;
}

Enc *Enc_run(Menu *m) {
    Enc *e = Enc_new(m->first->option_selected + 1);

    e->music_on = !(m->first->next->option_selected);
    Player *p1 = e->players[0];
    Player *p2 = e->players[1];
    e->hunter->pl = p1;
    p1->p = e->hunter;

    if (e->n_players > 1) {
        Player *p2 = e->players[1];
        e->hunter2->pl = p2;
        p2->p = e->hunter2;
    }

    //u32 fps = 60;
    u8 skipped = 0;
    while (e->state != ENC_COMPLETE) {
        Enc_update(e);
        Player_input(p1, e);
        Player_input(p2, e);
        if (e->state != ENC_PAUSED) {
            Physics_update_all(e);
            BG_update(e->bg);
        }
        //VDP_showFPS(FALSE); // TODO dbg
        //if (fps > 50 || skipped > 0) {
        if (TRUE) {
            SPR_update();
            SYS_doVBlankProcess();
            skipped = 0;
        } else {
            ++skipped;
        }
        //fps = SYS_getFPS();
    }
    Enc_cleanup(e);
    return e;
}

void Enc_update_score(Enc *e) {
}
