#include "bh.h"

void Enc_setup_room(Enc *e) {
    Room *room = Room_new(6, 0, 25);
    for (u8 r = 0; r < ROOM_H; ++r) {
        for (u8 c = 0; c < ROOM_W - 1; ++c) {
            Room_Cell *rc = room->cells[r][c];
            if (r < (ROOM_H - 1) && *(rc->down_wall)) {
                Physics_new_wall(e, FIXX(8 + c * 32), FIXY((r + 1) * 24), TRUE);
            }
            if (*(rc->right_wall)) {
                Physics_new_wall(e, FIXX((c + 1) * 32), FIXY(8 + r * 24), FALSE);
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
    Physics_new_brainguy(e, FIXX(8), FIXY(0));
}

Enc *Enc_new(u8 n_players) {
    Enc *e = ct_calloc(1, sizeof(Encounter));
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
    /*
    for (u8 player_no = n_players; player_no < 4; ++player_no) {
        e->players[player_no] = Player_new(0);
        e->players[player_no]->ai_level = 1;
        e->players[player_no]->player_no = player_no;
    }
    */

    Enc_setup_room(e);

    Phy *snail = Physics_new_snail(e, FIXX(8), FIXY(200));
    snail->pl = e->players[0];
    e->players[0]->p = snail;

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
        }
    }
}

Enc *Enc_run(Menu *m) {
    Enc *e = Enc_new(m->first->option_selected + 1);

    e->music_on = !(m->first->next->option_selected);

    while (e->state != ENC_COMPLETE) {
        Enc_update(e);
        for (u8 player_no = 0; player_no < 4; ++player_no) {
            Player_input(e->players[player_no], e);
        }
        if (e->state != ENC_PAUSED) {
            BG_update(e->bg);
            Physics_update_all(e);
        }
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
