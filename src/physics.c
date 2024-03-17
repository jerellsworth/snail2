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
        const SpriteDefinition *spriteDef,
        u8 pal
    ) {
    Physics *p = ct_calloc(1, sizeof(Physics));
    p->reg = Physics_register(p);
    if (p->reg < 0) {
        free(p);
        return NULL;
    }
    p->uid = random();
    u16 x = 0;
    u16 y = 0;
    p->spriteDef = spriteDef;
    p->pal = pal;
    p->center_offset_x = FIXX(0);
    p->center_offset_y = FIXY(0);
    if (p->spriteDef) {
        p->sp = SPR_addSprite(
            spriteDef,
            x,
            y,
            TILE_ATTR(pal, TRUE, FALSE, FALSE)
            );
        if (!p->sp) {
            free(p);
            SPR_defragVRAM();
            return NULL;
        }
        SPR_setPriority(p->sp, TRUE);
        SPR_setVisibility(p->sp, HIDDEN);
        p->h = p->spriteDef->h;
        p->w = p->spriteDef->w;
        p->center_offset_x = FIXX(p->w) >> 1;
        p->center_offset_y = FIXY(p->h) >> 1;
    }
    p->x = FIXX(x);
    p->y = FIXY(y);
    p->dx = FIX16(0);
    p->dy = FIX16(0);
    p->ddx = FIX16(0);
    p->ddy = FIX16(0);
    p->f = LEFT;
    p->ttl = -1;
    p->col_x = FIXX(0);
    p->col_y = FIXY(0);
    p->iframes = 0;
    p->grav_model = FALSE;
    p->frames_alive = 0;
    p->terminal_velocity_up = FIX16(3);
    p->terminal_velocity_down = FIX16(3);
    p->get_in_able = TRUE;
    return p;
}

void Physics_del(Physics *p, Enc *e) {

    if (p->partner) p->partner->partner = NULL;
    if (p->sp) SPR_releaseSprite(p->sp);
    if (p->pl) p->pl->p = NULL;
    if (p->instance_counter) --(*p->instance_counter);
    ALL_PHYSICS_OBJS[p->reg] = NULL;
    free(p);
}

void Physics_del_all(Enc *e) {
    for (u8 i = 0; i < MAX_PHYSICS_OBJS; ++i) {
        Physics *p = ALL_PHYSICS_OBJS[i];
        if (p) Physics_del(p, e);
    }
}

void Physics_del_all_thing(Enc *e, Thing t) {
    for (u8 i = 0; i < MAX_PHYSICS_OBJS; ++i) {
        Physics *p = ALL_PHYSICS_OBJS[i];
        if (p && p->what == t) Physics_del(p, e);
    }
}

void Physics_update(Encounter *enc, Physics *p) {

    if (!p) return;

    if (enc->state != ENC_RUNNING) return;

    if (p->ttl == 0) {
        Physics_del(p, enc);
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

    /*
    if (!BG_in_range(enc->bg, p)) {
        if (p->sp) {
            SPR_setVisibility(p->sp, HIDDEN);
        }
        return;
    }
    */

    if (p->dash_frames > 0) {
        --p->dash_frames;
        if (p->dash_frames == 0) {
            p->dx = p->dx_after_dash;
            p->dy = p->dy_after_dash;
        } else {
            p->dx = p->dash_dx;
            p->dy = p->dash_dy;
        }
    }

    /*
    if (p->lateral_resistance > 0) {
        if (p->dx < 0) {
            p->ddx = p->lateral_resistance;
        } else if (p->dx > 0) {
            p->ddx = -p->lateral_resistance;
        }
    }
    */

    /*
    if (p->grav_model) p->ddy = GRAVITY;
    if (p->dash_frames) p->ddy = 0;
    */

    p->dx += p->ddx;
    p->dy += p->ddy;

    /*
    if (p->grav_model && (!p->dash_frames)) {
        if (p->dy >= p->terminal_velocity_down) p->dy = p->terminal_velocity_down;
        if (p->dy <= -p->terminal_velocity_up) p->dy = -p->terminal_velocity_up;
        if (p->dx <= -p->terminal_velocity_down) p->dx = -p->terminal_velocity_down;
        if (p->dx >= p->terminal_velocity_down) p->dx = p->terminal_velocity_down;
    }
    */

    if (p->collision && (!p->ignore_walls)) {
        fixy h = FIXY(p->h);
        fixx w = FIXX(p->w);
        p->blocked = FALSE;

        if (p->dy > 0 && BG_collide(enc->bg, p->col_x, p->y + h + p->dy, p->dx, p->dy, FALSE)) {
            p->blocked = TRUE;
            if (p->dash_frames > 0) p->dash_frames = 0;
            if (p->drop_on_collide) {
                p->dx = 0;
                p->dy = 0;
            } else if (p->bouncy) {
                p->dy = min(-abs(p->dy) + (p->elastic ? 0 : FIXY(2)), 0);
            } else {
                p->dy = 0;
            }
        } else if (p->dy < 0 && BG_collide(enc->bg, p->col_x, p->y - p->dy, p->dx, p->dy, FALSE)) {
            p->blocked = TRUE;
            if (p->dash_frames > 0) p->dash_frames = 0;
            if (p->drop_on_collide) {
                p->dx = 0;
                p->dy = 0;
            } else if (p->bouncy) {
                p->dy = max(abs(p->dy) - (p->elastic? 0 : FIXY(2)), 0);
            } else {
                p->dy = 0;
            }
        }
        if (p->dx < 0 && BG_collide(enc->bg, p->x - p->dx, p->col_y, p->dx, p->dy, FALSE)) {
            p->blocked = TRUE;
            if (p->dash_frames > 0) p->dash_frames = 0;
            if (p->drop_on_collide) {
                p->dx = 0;
                p->dy = 0;
            } else if (p->bouncy) {
                p->dx = max(abs(p->dx) - (p->elastic ? 0 :  FIXX(2)), 0);
            } else {
                p->dx = 0;
            }
        } else if (p->dx > 0 && BG_collide(enc->bg, p->x + w + p->dx, p->col_y, p->dx, p->dy, FALSE)) {
            p->blocked = TRUE;
            if (p->dash_frames > 0) p->dash_frames = 0;
            if (p->drop_on_collide) {
                p->dx = 0;
                p->dy = 0;
            } else if (p->bouncy) {
                p->dx = min(-abs(p->dx) + (p->elastic ? 0 : FIXX(2)), 0);
            } else {
                p->dx = 0;
            }
        }
    }

    if (abs(p->dx) < FIX16(0.1)) {
        p->dx = 0;
    }
    
    p->x += fix16ToFixx(p->dx);
    p->y += fix16ToFixy(p->dy);

    behave(enc, p);

    p->col_x = p->x + p->center_offset_x;
    p->col_y = p->y + p->center_offset_y;

    /*
    if (p->update_direction) {
        p->update_direction = FALSE;
        if (p->theta <= 256) {
            // upper right quadrant
            SPR_setHFlip(p->sp, FALSE);
            SPR_setAnim(p->sp, 4 - (p->theta >> 6));
        } else if (p->theta <= 768) {
            // upper left quadrant
            SPR_setHFlip(p->sp, TRUE);
            SPR_setAnim(p->sp, (p->theta - 256) >> 6);
        } else if (p->theta <= 1024) {
            SPR_setHFlip(p->sp, FALSE);
            SPR_setAnim(p->sp, 8 - ((p->theta - 768) >> 6));
        }
    }
    */

    if (p->sp) {
        SPR_setVisibility(p->sp, VISIBLE);
        SPR_setPosition(p->sp, fixxToRoundedInt(p->x - BG_x(enc->bg)), fixyToRoundedInt(p->y - BG_y(enc->bg)));
    }
}

void Physics_spr_set_position_all(Enc *enc) {
    for (int i = 0; i < MAX_PHYSICS_OBJS; ++i) {
        Phy *p = ALL_PHYSICS_OBJS[i];
        if (!p) continue;
        if (BG_in_range(enc->bg, p)) {
            SPR_setVisibility(p->sp, VISIBLE);
            SPR_setPosition(p->sp, fixyToRoundedInt(p->x - BG_x(enc->bg)), fixyToRoundedInt(p->y - BG_y(enc->bg)));
            p->col_x = p->x + p->center_offset_x;
            p->col_y = p->y + p->center_offset_y;
        } else {
            SPR_setVisibility(p->sp, HIDDEN);
        }
    }
}

u32 Physics_dist(Physics *p1, Physics *p2) {
    s16 dx = fixxToRoundedInt(p1->col_x) - fixxToRoundedInt(p2->col_x);
    s16 dy = fixyToRoundedInt(p1->col_y) - fixyToRoundedInt(p2->col_y);
    u32 dist = dx * dx + dy * dy;

    /*
    char buf[16];
    sprintf(buf, "%ld", dist);
    VDP_drawText(buf, 30, 0);
    */

    return dist;
}

bool collision(Physics *p1, Physics *p2, u32 thresh) {
    return Physics_dist(p1, p2) <= thresh;
}

bool collision_box(Phy *p1, Phy *p2) {
    fixx p1x = p1->x + fix16ToFixx(p1->dx);
    fixx p2x = p2->x + fix16ToFixx(p2->dx);
    fixy p1y = p1->y + fix16ToFixy(p1->dy);
    fixy p2y = p2->y + fix16ToFixy(p2->dy);
    return (
        p1x < p2x + FIXX(p2->w) &&
        p1x + FIXX(p1->w) > p2->x &&
        p1y < p2y + FIXY(p2->h) &&
        p1y + FIXY(p1->h) > p2->y
        );
}

void Physics_update_all(Encounter *enc) {
    for (int i = 0; i < MAX_PHYSICS_OBJS; ++i) {
        Physics_update(enc, ALL_PHYSICS_OBJS[i]);
    }

    if (enc->state != ENC_RUNNING) return;

    fixx xmin = FIXX(0);
    fixx xmax = FIXX(40);
    while (xmin < FIXX(320)) {
        Phy *phys_in_range[MAX_PHYSICS_OBJS];
        u8 n_phys_in_range = 0;

        for (u8 i = 0; i < MAX_PHYSICS_OBJS; ++i) {
            Phy *p = ALL_PHYSICS_OBJS[i];
            if (!p) continue;
            if (!p->collision) continue;
            if (p->iframes > 0) continue;
            if (p->x < xmin || p->x >= xmax) continue;
            //if (!BG_in_range(enc->bg, p)) continue;
            phys_in_range[n_phys_in_range] = p;
            ++n_phys_in_range;
        }
        for (u8 i = 0; i < n_phys_in_range - 1; ++i) {
            Phy *pi = phys_in_range[i];
            for (u8 j = i + 1; j < n_phys_in_range; ++j) {
                Phy *pj = phys_in_range[j];
                if (!(pi->calc_collisions || pj->calc_collisions)) continue;
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

                if (pi->dash_frames) {
                    pi->dash_dx = pi->dx;
                    pi->dash_dy = pi->dy;
                }
                if (pj->dash_frames) {
                    pj->dash_dx = pj->dx;
                    pj->dash_dy = pj->dy;
                }
            }
        }
        xmin = xmax;
        xmax += FIXX(40);
    }
}

void Physics_direction_to(Physics *from, Physics *to, fix16 *norm_x, fix16 *norm_y, u16 *theta) {
    fix16 dx = fixxToFix16(to->x - from->x);
    fix16 dy = fixyToFix16(to->y - from->y);

    normalize(dx, dy, FIX16(1), norm_x, norm_y);

    *theta = arccossin(*norm_x, *norm_y);
}

void Physics_set_visibility_all(Thing t, u16 newVisibility) {
    for (int i = 0; i < MAX_PHYSICS_OBJS; ++i) {
        Physics *p = ALL_PHYSICS_OBJS[i];
        if (p && (p->what & t)) SPR_setVisibility(p->sp, newVisibility);
    }
}
