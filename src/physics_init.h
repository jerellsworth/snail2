#ifndef PHYSICS_INIT_H
#define PHYSICS_INIT_H

#include "bh.h"

enum Thing_e {
    WHAT_PROP, // NOTE: PROP should always be first
    WHAT_PARTICLE,
    WHAT_GOBLIN,
    WHAT_DYNAMITE,
    WHAT_EXPLOSION,
    WHAT_CASH
};

Phy *Physics_new_goblin(Enc *e, u8 goblin_no, fixx x, fixy y);
Phy *Physics_new_dynamite(Enc *e, fixx x, fixy y, fixz z, fix16 dx, fix16 dy, fix16 dz, u8 *counter);
Phy *Physics_new_explosion(Enc *e, fixx x, fixy y);
Phy *Physics_new_prop(Enc *e, fixx x, fixy y);
Phy *Physics_new_prop_particle(Enc *e, fixx x, fixy y, fixz z, fix16 dx, fix16 dy, fix16 dz);
Phy *Physics_new_goblin_particle(Enc *e, fixx x, fixy y, fixz z, fix16 dx, fix16 dy, fix16 dz);
Phy *Physics_new_cash(Enc *e, fixx x, fixy y);

#endif
