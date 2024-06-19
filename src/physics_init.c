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
    p->collision = TRUE;
    p->calc_collisions = FALSE;
    return p;
}
