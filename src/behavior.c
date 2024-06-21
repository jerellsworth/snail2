#include "bh.h"

void behave(Encounter *e, Physics *p) {
    switch (p->what) {
        case WHAT_SNAIL:
            fixx xoffset = p->x - FIXX(8);
            fixy yoffset = p->y - FIXY(8);
            if (
                (xoffset % FIXX(32) == 0) &&
                (yoffset % FIXY(24) == 0)
            ) {
                if ((p->buffer_dx != 0) || (p->buffer_dy != 0)) {
                    if (xoffset < 0) return;
                    if (yoffset < 0) return;

                    u8 r = fix16ToInt(xoffset) / 32;
                    u8 c = fix16ToInt(yoffset) / 24;

                    if (r >= ROOM_H) return;
                    if (c >= ROOM_W) return;
                    
                    Room_Cell *rc = e->room->cells[r][c];
                    if (
                        (p->dx > 0) &&
                        (c < ROOM_W - 1) &&
                        !(*(rc->right_wall))
                        ) {
                        p->dx = p->buffer_dx;
                        p->buffer_dx = 0;
                        return;
                    } 
                    if (
                        (p->dx < 0) &&
                        (c > 0) &&
                        !(*(rc->left_wall))
                        ) {
                        p->dx = p->buffer_dx;
                        p->buffer_dx = 0;
                        return;
                    } 
                    if (
                        (p->dy > 0) &&
                        (r < ROOM_H - 1) &&
                        !(*(rc->down_wall))
                        ) {
                        p->dy = p->buffer_dy;
                        p->buffer_dy = 0;
                        return;
                    } 
                    if (
                        (p->dy < 0) &&
                        (r > 0) &&
                        !(*(rc->up_wall))
                        ) {
                        p->dy = p->buffer_dy;
                        p->buffer_dy = 0;
                    }
                }
            }
            return;
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
