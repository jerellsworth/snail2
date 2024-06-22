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

                    u8 r = fix16ToInt(yoffset) / 24;
                    u8 c = fix16ToInt(xoffset) / 32;

                    char buf[32];
                    sprintf(buf, "%d (%d), %d (%d)", r, fix16ToInt(yoffset), c, fix16ToInt(xoffset));
                    VDP_drawText(buf, 1, 1);
                    if (r >= ROOM_H) return;
                    if (c >= ROOM_W) return;
                    sprintf(buf, "%d", random_with_max(9));
                    VDP_drawText(buf, 1, 2);
                    
                    Room_Cell *rc = e->room->cells[r][c];
                    sprintf(buf, "r:%d,l:%d,d:%d,u:%d", *(rc->right_wall), *(rc->left_wall), *(rc->down_wall), *(rc->up_wall));
                    VDP_drawText(buf, 1, 3);
                    // TODO the wall checks are innacurate
                    if (
                        (p->buffer_dx > 0) &&
                        (c < ROOM_W - 1) &&
                        !(*(rc->right_wall))
                        ) {
                        sprintf(buf, "%d", random_with_max(9));
                        VDP_drawText(buf, 1, 4);
                        p->dx = p->buffer_dx;
                        p->dy = 0;
                        p->buffer_dx = 0;
                        p->buffer_dy = 0;
                        return;
                    } 
                    if (
                        (p->buffer_dx < 0) &&
                        (c > 0) &&
                        !(*(rc->left_wall))
                        ) {
                        sprintf(buf, "%d", random_with_max(9));
                        VDP_drawText(buf, 1, 5);
                        p->dx = p->buffer_dx;
                        p->dy = 0;
                        p->buffer_dx = 0;
                        p->buffer_dy = 0;
                        return;
                    } 
                    if (
                        (p->buffer_dy > 0) &&
                        (r < ROOM_H - 1) &&
                        !(*(rc->down_wall))
                        ) {
                        sprintf(buf, "%d", random_with_max(9));
                        VDP_drawText(buf, 1, 6);
                        p->dy = p->buffer_dy;
                        p->dx = 0;
                        p->buffer_dy = 0;
                        p->buffer_dx = 0;
                        return;
                    } 
                    if (
                        (p->buffer_dy < 0) &&
                        (r > 0) &&
                        !(*(rc->up_wall))
                        ) {
                        sprintf(buf, "%d", random_with_max(9));
                        VDP_drawText(buf, 1, 7);
                        p->dy = p->buffer_dy;
                        p->dx = 0;
                        p->buffer_dy = 0;
                        return;
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
    if (p1->what == WHAT_WALL && p2->what == WHAT_SNAIL) {
        u8 r = fix16ToRoundedInt(p2->y /*- p2->dy*/ - FIX16(8)) / 24;
        u8 c = fix16ToRoundedInt(p2->x /*- p2->dx*/ - FIX16(8)) / 32;
        p2->x = FIXX(c * 32 + 8);
        p2->y = FIXY(r * 24 + 8);
        p2->dx = 0;
        p2->dy = 0;
        return TRUE;
    }
    return FALSE;
}
