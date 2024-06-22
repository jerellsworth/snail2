#ifndef PHYSICS_INIT_H
#define PHYSICS_INIT_H

#include "bh.h"

enum Thing_e {
    WHAT_PROP,
    WHAT_PARTICLE,
    WHAT_WALL,
    WHAT_SNAIL
};

Phy *Physics_new_wall(Enc *e, fixx x, fixy y, bool is_horizontal);
Phy *Physics_new_snail(Enc *e, fixx x, fixy y);
Phy *Physics_new_slime(Enc *e, fixx x, fixy y, Facing f);

#endif
