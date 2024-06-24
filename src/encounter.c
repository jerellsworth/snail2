#include "bh.h"

void Enc_setup_room(Enc *e) {
    Room *room = Room_new(6, 0, 25);
    for (u8 r = 0; r < ROOM_H; ++r) {
        for (u8 c = 0; c < ROOM_W; ++c) {
            Room_Cell *rc = room->cells[r][c];
            if (r < (ROOM_H - 1) && *(rc->down_wall)) {
                Physics_new_wall(e, FIXX(8 + c * 32), FIXY((r + 1) * 24), r, c, TRUE);
            }
            if (*(rc->right_wall)) {
                Physics_new_wall(e, FIXX((c + 1) * 32), FIXY(8 + r * 24), r, c, FALSE);
            }
        }
    }
    e->room = room;
    VDP_setTileMapXY(
        BG_A,
        TILE_ATTR_FULL(PAL2, TRUE, FALSE, FALSE, e->meter_tile_ind + 9),
        38,
        9
        );
    VDP_fillTileMapRect(
        BG_A,
        TILE_ATTR_FULL(PAL2, TRUE, FALSE, FALSE, e->meter_tile_ind + 8),
        38, 
        10,
        1, 
        16
        );    
    VDP_setTileMapXY(
        BG_A,
        TILE_ATTR_FULL(PAL2, TRUE, FALSE, FALSE, e->meter_tile_ind + 10),
        38,
        26 
        );
    Physics_new_banana(e, FIXX(264), FIXY(8));
    Physics_new_brainguy(e, FIXX(40), FIXY(0));
}

Enc *Enc_new(u8 n_players, u8 level) {
    Enc *e = ct_calloc(1, sizeof(Encounter));
    e->level = level;
    e->n_players = n_players;
    e->state = ENC_STARTING;

    PAL_setPalette(PAL0, PAL_BG.data, DMA);
    PAL_setPalette(PAL1, PAL_FG.data, DMA);
    PAL_setPalette(PAL2, PAL_SPRITE1.data, DMA);
    PAL_setPalette(PAL3, PAL_SPRITE2.data, DMA);

    e->bg = BG_init(
        &MAP_BG,
        &TLS_BG,
        &MAP_FG,
        &TLS_FG,
        &COLLISION_BG,
        PAL_BG.data
        );
    e->meter_tile_ind = 0x3C0; // I dunno seems empty
    VDP_loadTileSet(&TLS_METER, e->meter_tile_ind, DMA);

    e->tm = Tile_Manager_new();

    u16 joy = JOY_1;
    for (u8 player_no = 0; player_no < n_players; ++player_no) {
        if (joy == JOY_2 && JOY_getPortType(PORT_2) == PORT_TYPE_EA4WAYPLAY) ++joy;
        e->players[player_no] = Player_new(joy);
        e->players[player_no]->player_no = player_no;
        ++joy;
    }

    Enc_setup_room(e);

    Phy *snail = Physics_new_snail(e, FIXX(8), FIXY(200));
    snail->pl = e->players[0];
    e->players[0]->p = snail;

    fixy level_y = FIXY(8);
    Physics_new_number(e, FIXX(304), level_y, level / 100);
    level_y += FIXY(8);
    level %= 100;
    Physics_new_number(e, FIXX(304), level_y, level / 10);
    level_y += FIXY(8);
    level %= 10;
    Physics_new_number(e, FIXX(304), level_y, level);

    e->frames_remaining = 16 * 8;

    return e;
}

void Enc_del(Enc *e) {
    BG_del(e->bg);
    free(e);
}

void Enc_cleanup(Enc *e) {
    u16 black[16];
    memset(black, 0, 32);
    PAL_fadeTo(0, 63, black, 60, FALSE);
    Physics_del_all(e);
    SPR_reset();
    SPR_update();
    SYS_doVBlankProcess();
    XGM_stopPlayPCM(SOUND_PCM_CH1);
}

void Enc_update(Enc *e) {
    if (e->music_on && !(e->frames & 3) && !XGM_isPlaying()) {
        switch (e->song) {
            case 0:
                // TODO
                //XGM_startPlay(XGM_INGAME);
                break;
            default:
                break;
        }
    }
    ++e->song_frames;
    if (e->state == ENC_PAUSED) return;

    ++e->frames;
    ++e->state_frames;

    if (e->state == ENC_STARTING) {
        /*
        if (e->state_frames == 1 || e->state_frames == 60 || e->state_frames == 120) {
                XGM_startPlayPCMNextCh(SND_SAMPLE_TICK, 14);
        } else if (e->state_frames == 180) {
                XGM_startPlayPCMNextCh(SND_SAMPLE_LAUGH, 14);
        }
        if (e->state_frames >= 60 * 4) {
            e->state = ENC_RUNNING;
            e->state_frames = 0;
            SPR_releaseSprite(e->countdown);
        }
        */
        e->state = ENC_RUNNING;
        return;
    }

    if (!(e->frames & 7)) {
        // TODO "ticks_remaining" would be more accurate
        if (e->frames_remaining > 0) {
            --e->frames_remaining;
            u8 meter_row = 25 - (e->frames_remaining >> 3);
            u8 meter_tile_offset = e->frames_remaining & 7;
            VDP_setTileMapXY(
                BG_A,
                TILE_ATTR_FULL(PAL2, TRUE, FALSE, FALSE, e->meter_tile_ind + meter_tile_offset),
                38,
                meter_row
                );
        } else {
            e->state = ENC_COMPLETE;
            e->failed = TRUE;
        }
    }
}

Enc *Enc_run(Menu *m, u8 level) {
    Enc *e = Enc_new(1, level);

    e->music_on = !(m->first->next->option_selected);

    while (e->state != ENC_COMPLETE) {
        Enc_update(e);
        Player_input(e->players[0], e);
        if (e->state != ENC_PAUSED) {
            BG_update(e->bg);
            Physics_update_all(e);
        }
        SPR_update();
        SYS_doVBlankProcess();
    }

    // TODO play sad song or happy song depending on status
    for (int i = 0; i < MAX_PHYSICS_OBJS; ++i) {
        Phy *p = ALL_PHYSICS_OBJS[i];
        if (p == NULL) continue;
        if (p->what != WHAT_PROP) continue;
        if (e->failed) {
            p->grav_model=TRUE;
        } else {
            p->dx = -FIX16(random_with_max(8) >> 1);
            p->dy = -FIX16(random_with_max(8) >> 1);
        }
    }
    
    for (u8 i = 0; i < 120; ++i) {
        BG_update(e->bg);
        Physics_update_all(e);
        SPR_update();
        SYS_doVBlankProcess();
    }
    Enc_cleanup(e);
    return e;
}

void Enc_update_score(Enc *e) {
    // TODO
    char buf[16];
    //sprintf(buf, "%d00", e->score);
    VDP_clearTextBG(BG_A, 20, 1, 16); 
    VDP_drawTextBG(BG_A, buf, 20, 1);
}
