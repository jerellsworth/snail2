#include "bh.h"

Physics *_all_physics_objs[MAX_PHYSICS_OBJS];
Physics **ALL_PHYSICS_OBJS = _all_physics_objs;

s16 Physics_register(Physics *p) {
    for (int i = 0; i < MAX_PHYSICS_OBJS; ++i) {
        if (ALL_PHYSICS_OBJS[i] == NULL) {
            ALL_PHYSICS_OBJS[i] = p;
            return i;
        }
    }
    return -1;
}

Physics *Physics_new(
        Enc *e,
        const SpriteDefinition *spriteDef,
        u8 pal,
        fixx x,
        fixy y,
        bool bg_element
    ) {
    Physics *p = ct_calloc(1, sizeof(Physics));
    p->reg = Physics_register(p);
    if (p->reg < 0) {
        free(p);
        return NULL;
    }
    p->uid = random();
    p->x = x;
    p->y = y;
    p->spriteDef = spriteDef;
    p->pal = pal;
    p->h = p->spriteDef->h;
    p->w = p->spriteDef->w;
    p->center_offset_x = FIXX(p->w) >> 1;
    p->center_offset_y = FIXY(p->h) >> 1;
    if (bg_element) {
        p->bg_element = TRUE;
        p->bg_tile_idx = SpriteDefinition_VDP_idx(e->tm, spriteDef);
        p->start_x = x;
        p->start_y = y;
        Physics_bg_element_redraw(p, FALSE);
        p->n_tiles = (p->h >> 3) * (p->w >> 3);
    } else {
        p->sp = SPR_addSprite(
            spriteDef,
            0,
            0,
            TILE_ATTR(pal, TRUE, FALSE, FALSE)
            );
        if (!p->sp) {
            free(p);
            SPR_defragVRAM();
            return NULL;
        }
        SPR_setPriority(p->sp, TRUE);
        SPR_setVisibility(p->sp, HIDDEN);
    }
    p->dx = FIX16(0);
    p->dy = FIX16(0);
    p->ddx = FIX16(0);
    p->ddy = FIX16(0);
    p->f = LEFT;
    p->ttl = -1;
    p->col_x = p->x + p->center_offset_x;
    p->col_y = p->y + p->center_offset_y;
    p->iframes = 0;
    p->grav_model = FALSE;
    p->frames_alive = 0;
    p->terminal_velocity_up = FIX16(10);
    p->terminal_velocity_down = FIX16(8);
    
    return p;
}

void Physics_del(Physics *p, Enc *e) {

    if (p->partner) p->partner->partner = NULL;
    if (p->sp) SPR_releaseSprite(p->sp);
    if (p->shadow) SPR_releaseSprite(p->shadow);
    if (p->pl) p->pl->p = NULL;
    if (p->instance_counter) --(*p->instance_counter);
    if (p->bg_element) {
        VDP_clearTileMapRect(
            BG_B,
            fixxToInt(p->start_x) / 8,
            fixyToInt(p->start_y) / 8,
            p->spriteDef->w / 8,
            p->spriteDef->h / 8
            );
    }
    ALL_PHYSICS_OBJS[p->reg] = NULL;
    free(p);
}

void Physics_del_all(Enc *e) {
    for (u8 i = 0; i < MAX_PHYSICS_OBJS; ++i) {
        Physics *p = ALL_PHYSICS_OBJS[i];
        if (p) Physics_del(p, e);
    }
}

void Physics_update(Encounter *e, Physics *p) {

    if (!p) return;

    if (e->state != ENC_RUNNING) return;

    if (p->ttl == 0) {
        Physics_del(p, e);
        return;
    }

    if (p->ttl > 0 && (!p->pl)) {
        --p->ttl;
    }

    if (p->iframes > 0) {
        --p->iframes;
    }

    ++p->state_frames;
    ++p->frames_alive;

    if (p->bg_element) {

        behave(e, p);
        return;
    }

    if (p->grav_model) p->ddz = -GRAVITY;

    p->dx += p->ddx;
    p->dy += p->ddy;
    p->dz += p->ddz;

    if (p->grav_model) {
        if (p->dz >= p->terminal_velocity_down) p->dz = p->terminal_velocity_down;
        if (p->dz <= -p->terminal_velocity_up) p->dz = -p->terminal_velocity_up;
    }


    if (abs(p->dx) < FIX16(0.1)) {
        p->dx = 0;
    }
    
    p->x += fix16ToFixx(p->dx);
    p->y += fix16ToFixy(p->dy);
    p->z += fix16ToFixz(p->dz);

    if (p->collision && (!p->ignore_walls)) {
        BG_collide(e->bg, p);
    }
    p->z = max(p->z, 0);

    behave(e, p);

    p->col_x = p->x + p->center_offset_x;
    p->col_y = p->y + p->center_offset_y;


    if (p->sp) {
        SPR_setVisibility(p->sp, VISIBLE);
        SPR_setPosition(p->sp, fixxToRoundedInt(p->x - BG_x(e->bg)), fixyToRoundedInt(p->y - BG_y(e->bg) - p->z));
    }
    if (p->shadow) {
        SPR_setVisibility(p->shadow, VISIBLE);
        SPR_setPosition(p->shadow, fixxToRoundedInt(p->x - BG_x(e->bg)), fixyToRoundedInt(p->y - BG_y(e->bg)) + p->h - 8);
    }
}

u32 Physics_dist(Physics *p1, Physics *p2) {
    s16 dx = fixxToRoundedInt(p1->col_x) - fixxToRoundedInt(p2->col_x);
    s16 dy = fixyToRoundedInt(p1->col_y) - fixyToRoundedInt(p2->col_y);
    u32 dist = dx * dx + dy * dy;

    return dist;
}

bool collision_box(Phy *p1, Phy *p2) {
    fixx p1x = p1->x + fix16ToFixx(p1->dx) + p1->hbox_offset_x;
    fixx p2x = p2->x + fix16ToFixx(p2->dx) + p2->hbox_offset_x;
    fixy p1y = p1->y + fix16ToFixy(p1->dy) + p1->hbox_offset_y;
    fixy p2y = p2->y + fix16ToFixy(p2->dy) + p2->hbox_offset_y;
    return (
        p1x < p2x + FIXX(p2->w) &&
        p1x + FIXX(p1->w) > p2x &&
        p1y < p2y + FIXY(p2->h) &&
        p1y + FIXY(p1->h) > p2y &&
        abs(p1->z - p2->z) < FIXZ(8)
        );
}

bool collision_box_visual(Phy *p1, Phy *p2) {
    fixx p1x = p1->x;
    fixx p2x = p2->x;
    fixy p1y = p1->y;
    fixy p2y = p2->y;
    return (
        p1x < p2x + FIXX(p2->spriteDef->w) &&
        p1x + FIXX(p1->spriteDef->w) > p2x &&
        p1y < p2y + FIXY(p2->spriteDef->h) &&
        p1y + FIXY(p1->spriteDef->h) > p2y
        );
}

void Physics_update_all(Encounter *enc) {
    for (int i = 0; i < MAX_PHYSICS_OBJS; ++i) {
        Physics_update(enc, ALL_PHYSICS_OBJS[i]);
    }

    if (enc->state != ENC_RUNNING) return;

    fixx interval = FIX16(90);
    fixx xmin = FIXX(0);
    fixx xmax = interval;
    while (xmin < FIXX(320)) {
        Phy *phys_in_range[MAX_PHYSICS_OBJS];
        u8 n_phys_in_range = 0;

        for (u8 i = 0; i < MAX_PHYSICS_OBJS; ++i) {
            Phy *p = ALL_PHYSICS_OBJS[i];
            if (!p) continue;
            if (!p->collision) continue;
            if (p->iframes > 0) continue;
            if (p->x < xmin || p->x >= xmax) continue;
            phys_in_range[n_phys_in_range] = p;
            ++n_phys_in_range;
        }
        for (u8 i = 0; i < n_phys_in_range; ++i) {
            Phy *pi = phys_in_range[i];
            if (!pi->calc_collisions) continue;
            for (u8 j = 0; j < n_phys_in_range; ++j) {
                if (i == j) continue;
                Phy *pj = phys_in_range[j];
                if (!collision_box(pi, pj)) continue;

                if (interact(enc, pi, pj)) continue;
                // interact was false, so handling isn't complete. execute bounce

                // don't let objects actually get entangled
                pi->x -= pi->dx;
                pj->x -= pj->dx;
                pi->y -= pi->dy;
                pj->y -= pj->dy;

                if ((!pi->bouncy) && (!pj->bouncy)) continue;
                // Nobody is moving anywhere. Don't bother calculating

                if (pi->mass == 0 || pj->mass == 0) continue;
                // interaction doesn't make sense without mass

                fix16 total_mass, diff_mass, rat_mass;
                if (pi->bouncy && pj->bouncy) {
                    total_mass = pi->mass + pj->mass;
                    diff_mass = pi->mass - pj->mass;
                    rat_mass = fix16Div(diff_mass, total_mass);
                    pi->dx = fix16Mul(rat_mass, pi->dx) + fix16Mul(fix16Div(pj->mass << 1, total_mass), pj->dx);
                    pi->dy = fix16Mul(rat_mass, pi->dy) + fix16Mul(fix16Div(pj->mass << 1, total_mass), pj->dy);
                    pj->dx = fix16Mul(rat_mass, pj->dx) + fix16Mul(fix16Div(pi->mass << 1, total_mass), pi->dx);
                    pj->dy = fix16Mul(rat_mass, pj->dy) + fix16Mul(fix16Div(pi->mass << 1, total_mass), pi->dy);
                } else if (pi->bouncy) {
                    fixx new_x = pi->x + fix16ToFixx(pi->dx);
                    fixy new_y = pi->y + fix16ToFixy(pi->dy);
                    if (new_x + FIXX(pi->w) >= pj->x && new_x <= pj->x + FIXX(pj->w)) {
                        pi->dx = -pi->dx;
                    }
                    if (new_y + FIXY(pi->h) >= pj->y && new_y <= pj->y + FIXY(pj->h)) {
                        pi->dy = -pi->dy;
                    }
                } else { // only pj->bouncy
                    fix16 new_x = pj->x + fix16ToFixx(pj->dx);
                    fix32 new_y = pj->y + fix16ToFixy(pj->dy);
                    if (new_x + FIXX(pj->w) >= pi->x && new_x <= pi->x + FIXX(pi->w)) {
                        pj->dx = -pj->dx;
                    }
                    if (new_y + FIXY(pj->h) >= pi->y && new_y <= pi->y + FIXY(pi->h)) {
                        pj->dy = -pj->dy;
                    }
                }
            }
        }
        xmin = xmax - FIXX(16);
        xmax = xmin + interval;
    }
}

void Physics_bg_element_redraw(Physics *p, bool hflip) {
    if (!p->bg_element) return;
    VDP_fillTileMapRectIncT(
        BG_B,
        TILE_ATTR_FULL(p->pal, TRUE, FALSE, hflip, p->bg_tile_idx + p->anim_no * p->n_tiles),
        fixxToInt(p->start_x) / 8,
        fixyToInt(p->start_y) / 8,
        p->spriteDef->w / 8,
        p->spriteDef->h / 8
        );
}

Phy *Physics_collides_with_anything(Phy *p) {
    for (int i = 0; i < MAX_PHYSICS_OBJS; ++i) {
        Phy *p2 = ALL_PHYSICS_OBJS[i];
        if (p2 == NULL) continue;
        if (p == p2) continue;
        if (!(p2->collision)) continue;
        if (collision_box(p, p2)) return p2;
    }
    return NULL;
}

Phy *Physics_collides_with_anything_visual(Phy *p) {
    for (int i = 0; i < MAX_PHYSICS_OBJS; ++i) {
        Phy *p2 = ALL_PHYSICS_OBJS[i];
        if (p2 == NULL) continue;
        if (p == p2) continue;
        if (collision_box_visual(p, p2)) return p2;
    }
    return NULL;
}

u8 Physics_count(Thing t) {
    u8 ret = 0;
    for (int i = 0; i < MAX_PHYSICS_OBJS; ++i) {
        Phy *p = ALL_PHYSICS_OBJS[i];
        if (p == NULL) continue;
        if (p->what == t) ++ret;
    }
    return ret;
}
