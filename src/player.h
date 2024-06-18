#ifndef PLAYER_H
#define PLAYER_H

#include "bh.h"

#define BUTTON_ACTION (BUTTON_A | BUTTON_B | BUTTON_C)

#define PLAYER_AI_PREV_TARGET_MEMORY 3

struct Player_s {
    u8 joy;
    u8 player_no;
    u8 cooldown;
    u8 cooldown2;
    Physics *p;

    u8 round_score;
    u8 round_rank;

    u8 ai_level;
    Phy *ai_prev_tgts[PLAYER_AI_PREV_TARGET_MEMORY];
    u8 ai_prev_tgts_idx;
    u8 ai_state;
    u8 ai_next_decision;
    u8 walking_away_frames;
};

Player *Player_new(u8 joy);
void Player_del(Player *pl);
void Player_input(Player *pl, Enc *e);
void Player_clear_all(void);

#endif
