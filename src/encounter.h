#ifndef ENCOUNTER_H
#define ENCOUNTER_H

#include "bh.h"

typedef enum {
    ENC_STARTING,
    ENC_RUNNING,
    ENC_COMPLETE,
    ENC_PAUSED
} Enc_State;

struct Encounter_s{
    Enc_State state;
    Player *players[MAX_N_PLAYERS];
    Physics *pcs[MAX_N_PLAYERS];
    u8 n_players;

    BG *bg;
    Tile_Manager *tm;

    u16 frames;
    u16 state_frames;
    u16 level_frames;

    u8 goblin_counter;
    u16 seconds_remaining;
    u16 frames_remaining;

    u8 song;
    u16 song_frames;
    bool music_on;

    Sprite *countdown;
};

Enc *Enc_new(u8 n_players);

void Enc_cleanup(Enc *enc);
void Enc_del(Enc *enc);

Enc *Enc_run(Menu *m);
void Enc_reset_pc(Enc *, Player *, bool death, u8 iframes);
void Enc_update_score(Enc *e);

#endif
