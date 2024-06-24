#ifndef PHYSICS_INIT_H
#define PHYSICS_INIT_H

#include "bh.h"

enum Thing_e {
    WHAT_PROP,
    WHAT_PARTICLE,
    WHAT_WALL,
    WHAT_BRAINGUY,
    WHAT_BALL,
    WHAT_BANANA,
    WHAT_SNAIL
};

Phy *Physics_new_wall(Enc *e, fixx x, fixy y, u8 cell_row, u8 cell_col, bool is_horizontal);
Phy *Physics_new_snail(Enc *e, fixx x, fixy y);
Phy *Physics_new_slime(Enc *e, fixx x, fixy y, Facing f);
Phy *Physics_new_brainguy(Enc *e, fixx x, fixy y);
Phy *Physics_new_ball(Enc *e, fixx x, fixy y, fix16 dx, fix16 dy);
Phy *Physics_new_banana(Enc *e, fixx x, fixy y);
Phy *Physics_new_explosion(Enc *e, fixx x, fixy y);

#endif
