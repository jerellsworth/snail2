#ifndef ENCOUNTER_H
#define ENCOUNTER_H

#include "bh.h"

typedef enum {
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

    u16 frames;
    u16 state_frames;
    u16 level_frames;

    MSEG *ms;

    Phy *dragon;
    Phy *hunter;
    Phy *hunter2;

    u8 enemy_counter;
    u8 ball_counter;
    u8 level;
    u8 lives;
    u16 score;

    u8 song;
    u16 song_frames;
    bool music_on;
};

Enc *Enc_new(u8 n_players);

void Enc_cleanup(Enc *enc);
void Enc_del(Enc *enc);

Enc *Enc_run(Menu *m);
void Enc_reset_pc(Enc *, Player *, bool death, u8 iframes);
void Enc_update_score(Enc *);
void Enc_load_level(Enc *e);
void Enc_make_ball(Enc *e, u8 n);

#endif
