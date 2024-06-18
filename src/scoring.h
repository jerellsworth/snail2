#ifndef SCORING_H
#define SCORING_H

#include "bh.h"

struct Score_s {
    u16 previous_score;
    u16 cash_score;
    u16 rank_score;
    u16 rank;
    u16 player_no;
};

Score *Score_new(u8 pl_no);
void Score_del(Score *s);
void Score_tally(Score *s, Player *pl);

#endif
