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

Phy *Physics_new_slime(Enc *e, fixx x, fixy y, Facing f) {
    const SpriteDefinition *spr_def;
    if (f == LEFT || f == RIGHT) {
        spr_def = &SPR_VSLIME;
    } else {
        spr_def = &SPR_HSLIME;
    }
    Phy *p = Physics_new(
        e,
        spr_def,
        PAL2,
        x,
        y,
        FALSE
    );
    if (!p) return NULL;
    SPR_setVFlip(p->sp, f == UP);
    SPR_setHFlip(p->sp, f == RIGHT);
    SPR_setAnim(p->sp, random_with_max(1));
    p->what = WHAT_PARTICLE;
    p->ttl = 30;
    return p;
}

Phy *Physics_new_brainguy(Enc *e, fixx x, fixy y) {
    Phy *p = Physics_new(
        e,
        &SPR_BRAINGUY,
        PAL3,
        x,
        y,
        FALSE
    );
    if (!p) return NULL;
    p->what = WHAT_BRAINGUY;
    return p;
}

Phy *Physics_new_ball(Enc *e, fixx x, fixy y, fix16 dx, fix16 dy) {
    Phy *p = Physics_new(
        e,
        &SPR_BALL,
        PAL3,
        x,
        y,
        FALSE
    );
    if (!p) return NULL;
    p->dx = dx;
    p->dy = dy;
    p->what = WHAT_BALL;
    p->collision = TRUE;
    p->calc_collisions = TRUE;
    p->grav_model = TRUE;
    p->ttl = 60;
    return p;
}

Phy *Physics_new_banana(Enc *e, fixx x, fixy y) {
    Phy *p = Physics_new(
        e,
        &SPR_BANANA,
        PAL1,
        x,
        y,
        FALSE
    );
    p->what = WHAT_BANANA;
    p->collision = TRUE;
    p->start_x = x;
    p->w = 10;
    return p;
}
