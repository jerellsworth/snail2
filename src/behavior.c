#include "bh.h"

bool snail_would_collide(Phy *p1) {
    fixx p1x = p1->x + p1->buffer_dx;
    fixy p1y = p1->y + p1->buffer_dy;
    if (p1x < FIXX(8)) return FALSE;
    if (p1x > FIXX(264)) return FALSE;
    if (p1y < FIXY(8)) return FALSE;
    if (p1y > FIXY(200)) return FALSE;
    for (int i = 0; i < MAX_PHYSICS_OBJS; ++i) {
        Phy *p2 = ALL_PHYSICS_OBJS[i];
        if (p2 == NULL) continue;
        if (p2->what != WHAT_WALL) continue;
        fixx p2x = p2->x;
        fixx p2y = p2->y;
        if (p1x < p2x + FIXX(p2->spriteDef->w) &&
            p1x + FIXX(p1->spriteDef->w) > p2x &&
            p1y < p2y + FIXY(p2->spriteDef->h) &&
            p1y + FIXY(p1->spriteDef->h) > p2y) {
            return TRUE;
        }
    }
    return FALSE;
}

void behave(Encounter *e, Physics *p) {
    switch (p->what) {
        case WHAT_SNAIL:
            if (
                ((p->x - FIXX(8)) % FIXX(32) == 0) &&
                ((p->y - FIXY(8)) % FIXY(24) == 0)
            ) {
                if ((p->buffer_dx != 0) || (p->buffer_dy != 0)) {
                    if (!snail_would_collide(p)) {
                        p->dx = p->buffer_dx;
                        p->dy = p->buffer_dy;
                        p->buffer_dx = 0;
                        p->buffer_dy = 0;
                    }
                }
            }
            break;
        default:
            return;
    }
}

bool interact(Enc *e, Physics *pi, Physics *pj) {
    // sort by WHAT so we don't always have to try both directions
    Phy *p1, *p2;
    if (pi->what <= pj->what) {
        p1 = pi;
        p2 = pj;
    } else {
        p1 = pj;
        p2 = pi;
    }
    return FALSE;
}
