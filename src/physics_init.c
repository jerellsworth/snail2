#include "bh.h"

Phy *Physics_new_goblin(Enc *e, u8 goblin_no, fixx x, fixy y) {
    const SpriteDefinition* spr_def;
    u8 pal;
    Facing f;
    bool hflip;

    switch (goblin_no) {
        case 0:
            spr_def = &SPR_GOBLIN;
            pal = PAL1;
            f = RIGHT;
            hflip = FALSE;
            break;
        case 1:
            spr_def = &SPR_GOBLIN2;
            pal = PAL3;
            f = LEFT;
            hflip = TRUE;

            break;
        case 2:
            spr_def = &SPR_GOBLIN3;
            pal = PAL3;
            f = RIGHT;
            hflip = FALSE;
            break;
        case 3:
            spr_def = &SPR_GOBLIN4;
            pal = PAL3;
            f = LEFT;
            hflip = TRUE;
            break;
        default:
            return NULL;
    }
    Physics *p = Physics_new(e, spr_def, pal, x, y, FALSE);
    if (!p) return NULL;
    SPR_setDepth(p->sp, 0x7FFF); // send to bottom
    SPR_setHFlip(p->sp, hflip);
    p->what = WHAT_GOBLIN;
    p->mass = 1;
    p->f = f;
    p->calc_collisions = TRUE;
    p->collision = TRUE;
    p->hbox_offset_y = FIXY(26);
    p->h = 5;
    p->hbox_offset_x = FIXX(4);
    p->w = 19;
    p->instance_counter = &(e->goblin_counter);
    ++e->goblin_counter;

    // TODO dbg
    /*
    if (goblin_no == 1) {
        p->cursor = SPR_addSprite(
                &SPR_CURSOR,
                0,
                0,
                TILE_ATTR(PAL1, TRUE, FALSE, FALSE)
                );
    }
    */

    return p;
}

Phy *Physics_new_dynamite(Enc *e, fixx x, fixy y, fixz z, fix16 dx, fix16 dy, fix16 dz, u8 *counter) {
    Physics *p = Physics_new(e, &SPR_DYNAMITE, PAL1, x, y, FALSE);
    if (!p) return NULL;
    p->shadow = SPR_addSprite(
            &SPR_SHADOW_1,
            fixxToRoundedInt(x),
            fixyToRoundedInt(y),
            TILE_ATTR(PAL1, TRUE, FALSE, FALSE)
            );
    p->dx = dx;
    p->dy = dy;
    p->dz = dz;
    p->z = z;
    p->what = WHAT_DYNAMITE;
    p->collision = TRUE;
    p->mass = 1;
    p->f = RIGHT;
    ++*counter;
    p->instance_counter = counter;
    p->calc_collisions = TRUE;
    p->grav_model = TRUE;
    p->elastic = TRUE;
    p->bouncy = TRUE;
    return p;
}

Phy *Physics_new_explosion(Enc *e, fixx x, fixy y) {
    Physics *p = Physics_new(e, &SPR_EXPLOSION, PAL1, x, y, FALSE);
    if (!p) return NULL;
    p->what = WHAT_EXPLOSION;
    p->ttl = 7 * 8;
    p->collision = TRUE;
    p->calc_collisions = TRUE;
    return p;
}

Phy *Physics_new_prop(Enc *e, fixx x, fixy y) {
    fixx hbox_offset_x;
    fixy hbox_offset_y;
    u16 w;
    u16 h;
    const SpriteDefinition* spr_def;
    switch (random_with_max(9)) {
        case 0:
            spr_def = &SPR_PROP_1;
            hbox_offset_x = FIXX(1);
            hbox_offset_y = FIXY(10);
            w = 14;
            h = 8;
            break;
        case 1:
            spr_def = &SPR_PROP_2;
            hbox_offset_x = FIXX(0);
            hbox_offset_y = FIXY(16);
            w = 11;
            h = 7;
            break;
        case 2:
            spr_def = &SPR_PROP_3;
            hbox_offset_x = FIXX(0);
            hbox_offset_y = FIXY(12);
            w = 32;
            h = 13;
            break;
        case 3:
            spr_def = &SPR_PROP_4;
            hbox_offset_x = FIXX(0);
            hbox_offset_y = FIXY(13);
            w = 32;
            h = 13;
            break;
        case 4:
            spr_def = &SPR_PROP_5;
            hbox_offset_x = FIXX(0);
            hbox_offset_y = FIXY(13);
            w = 32;
            h = 13;
            break;
        case 5:
            spr_def = &SPR_PROP_6;
            hbox_offset_x = FIXX(0);
            hbox_offset_y = FIXY(13);
            w = 32;
            h = 13;
            break;
        case 6:
            spr_def = &SPR_PROP_7;
            hbox_offset_x = FIXX(0);
            hbox_offset_y = FIXY(12);
            w = 12;
            h = 8;
            break;
        case 7:
            spr_def = &SPR_PROP_8;
            hbox_offset_x = FIXX(0);
            hbox_offset_y = FIXY(2);
            w = 32;
            h = 13;
            break;
        case 8:
            spr_def = &SPR_PROP_9;
            hbox_offset_x = FIXX(0);
            hbox_offset_y = FIXY(0);
            w = 16;
            h = 26;
            break;
        case 9:
            spr_def = &SPR_PROP_10;
            hbox_offset_x = FIXX(0);
            hbox_offset_y = FIXY(5);
            w = 16;
            h = 10;
            break;
        default:
            return NULL;
    }

    Physics *p = Physics_new(e, spr_def, PAL2, x, y, TRUE);
    if (!p) return NULL;
    p->what = WHAT_PROP;
    p->collision = TRUE;
    p->calc_collisions = FALSE;
    p->hbox_offset_x = hbox_offset_x;
    p->hbox_offset_y = hbox_offset_y;
    p->w = w;
    p->h = h;
    return p;
}

Phy *Physics_new_prop_particle(Enc *e, fixx x, fixy y, fixz z, fix16 dx, fix16 dy, fix16 dz) {
    Physics *p = Physics_new(e, &SPR_PROP_PARTICLES, PAL2, x, y, FALSE);
    if (!p) return NULL;
    SPR_setAnim(p->sp, random_with_max(7));
    p->dx = dx;
    p->dy = dy;
    p->dz = dz;
    p->z = z;
    p->what = WHAT_PARTICLE;
    p->collision = FALSE;
    p->calc_collisions = FALSE;
    p->grav_model = TRUE;
    p->ttl = 60;
    return p;
}

Phy *Physics_new_goblin_particle(Enc *e, fixx x, fixy y, fixz z, fix16 dx, fix16 dy, fix16 dz) {
    Physics *p = Physics_new(e, &SPR_GOBLIN_PARTICLES, PAL1, x, y, FALSE);
    if (!p) return NULL;
    SPR_setAnim(p->sp, random_with_max(2));
    p->dx = dx;
    p->dy = dy;
    p->dz = dz;
    p->z = z;
    p->what = WHAT_PARTICLE;
    p->collision = FALSE;
    p->calc_collisions = FALSE;
    p->grav_model = TRUE;
    p->ttl = 60;
    return p;
}

Phy *Physics_new_cash(Enc *e, fixx x, fixy y) {
    Physics *p = Physics_new(e, &SPR_CASH, PAL1, x, y, FALSE);
    if (!p) return NULL;
    p->what = WHAT_CASH;
    p->collision = TRUE;
    p->calc_collisions = FALSE;
    p->grav_model = TRUE;
    p->w = 12;
    p->h = 15;
    return p;
}
