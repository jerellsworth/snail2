#include "bh.h"

Score *Score_new(u8 pl_no) {
    Score *s = ct_calloc(1, sizeof(Score));
    s->player_no = pl_no;
    return s;
}

void Score_del(Score *s) {
    free(s);
}

void Score_tally(Score *s, Player *pl) {
    s->previous_score = s->previous_score + s->cash_score + s->rank_score;
    s->cash_score = pl->round_score;
    s->rank = pl->round_rank == 0 ? 1 : pl->round_rank;
    switch (s->rank) {
        case 1:
            s->rank_score = 3;
            break;
        case 2:
            s->rank_score = 1;
            break;
        default:
            s->rank_score = 0;
            break;
    }
}
