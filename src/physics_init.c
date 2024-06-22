#include "bh.h"

Phy *Physics_new_wall(Enc *e, fixx x, fixy y, bool is_horizontal) {
    Phy *p = Physics_new(
        e,
        is_horizontal ? &SPR_HWALL : &SPR_VWALL,
        PAL1,
        x,
        y,
        TRUE
    );
    if (!p) return NULL;
    p->what = WHAT_WALL;
    p->collision = TRUE;
    p->calc_collisions = FALSE;
    return p;
}

Phy *Physics_new_snail(Enc *e, fixx x, fixy y) {
    Phy *p = Physics_new(
        e,
        &SPR_SNAIL,
        PAL2,
        x,
        y,
        FALSE
    );
    if (!p) return NULL;
    p->what = WHAT_SNAIL;
    p->collision = TRUE;
    p->calc_collisions = TRUE;
    return p;
}
