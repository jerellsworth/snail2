#include "bh.h"

void behave(Encounter *e, Physics *p) {
    switch (p->what) {
        case WHAT_PROP:
            if (p->state == 1) {
                SPR_setFrame(p->sp, min(p->state_frames >> 2, 4)); 
                if (p->y >= FIXY(100)) {
                    p->grav_model = FALSE;
                    p->dx = 0;
                    p->dy = 0;
                    p->ddx = 0;
                    p->ddy = 0;
                    p->state = 0;
                }
            }
            return;
        case WHAT_BANANA:
            if (!(e->frames & 7)) {
                if (p->x <= p->start_x) {
                    p->x = p->start_x;
                    p->dx = FIX16(0.2);
                } else if (p->x > p->start_x + FIXX(12)) {
                    p->dx = -FIX16(0.2);
                }
            }
            return;
        case WHAT_BRAINGUY:
            u8 framecheck = max(8 >> (e->level >> 4), 2) - 1;
            if (!(e->frames & framecheck)) {
                if (p->dx == 0 || p->x <= FIXX(8)) {
                    p->x = FIXX(8);
                    p->dx = min(FIX16(0.5) << (e->level >> 1), FIX16(4));
                } else if (p->x > FIXX(228)) {
                    p->dx = -min((FIX16(0.5) << (e->level >> 1)), FIX16(4));
                }
                if (!random_with_max(3 - min((e->level >> 3), 3))) {
                    Physics_new_ball(
                        e,
                        p->x + FIXX(11),
                        p->y + FIXY(9),
                        FIX16(4 - random_with_max(8)) >> 3,
                        0
                    );
                }
            }
            return;
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
                    u8 c = fix16ToInt(xoffset) >> 5;

                    if (r >= ROOM_H) return;
                    if (c >= ROOM_W) return;
                    
                    Room_Cell *rc = e->room->cells[r][c];
                    // TODO the wall checks are innacurate
                    if (
                        (p->buffer_dx > 0) &&
                        (c < ROOM_W - 1) &&
                        !(*(rc->right_wall))
                        ) {
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
                        p->dy = p->buffer_dy;
                        p->dx = 0;
                        p->buffer_dy = 0;
                        return;
                    }
                }
            }
            if (!(e->frames & 3)) {
                u8 r = fix16ToInt(yoffset) / 24;
                u8 c = fix16ToInt(xoffset) >> 5;
                Room_Cell *rc = e->room->cells[r][c];
                if (p->buffer_dx < 0 && *(rc->left_wall)) {
                    Physics_new_slime(e, p->x, p->col_y, LEFT);
                } else if (p->buffer_dx > 0 && *(rc->right_wall)) {
                    Physics_new_slime(e, p->x + FIXX(p->w) - FIXX(8), p->col_y, RIGHT);
                } else if (p->buffer_dy < 0 && *(rc->up_wall)) {
                    Physics_new_slime(e, p->col_x, p->y, UP);
                } else if (p->buffer_dy > 0 && *(rc->down_wall)) {
                    Physics_new_slime(e, p->col_x, p->y + FIXY(p->h) - FIXY(8), DOWN);
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
        u8 r = fix16ToRoundedInt(p2->y - FIX16(8)) / 24;
        u8 c = fix16ToRoundedInt(p2->x - FIX16(8)) / 32;
        p2->x = FIXX(c * 32 + 8);
        p2->y = FIXY(r * 24 + 8);
        p2->dx = 0;
        p2->dy = 0;
        p2->buffer_dx = 0;
        p2->buffer_dy = 0;
        return TRUE;
    } else if (p1->what == WHAT_BALL && p2->what == WHAT_BALL) {
        return TRUE;
    } else if (p1->what == WHAT_WALL && p2->what == WHAT_BALL) {
        if (p1->is_horizontal) {
            *(e->room->cells[p1->cell_row][p1->cell_col]->down_wall) = FALSE;
        } else {
            *(e->room->cells[p1->cell_row][p1->cell_col]->right_wall) = FALSE;
        }
        Physics_del(p1, e);
        Physics_new_explosion(e, p2->col_x, p2->col_y);
        Physics_del(p2, e);
        XGM_startPlayPCMNextCh(SND_SAMPLE_EXPLOSION, 14);
        return TRUE;
    } else if (p1->what == WHAT_BALL && p2->what == WHAT_SNAIL) {
        Physics_del(p1, e);
        Physics_new_explosion(e, p2->col_x, p2->col_y);
        Physics_del(p2, e);
        e->state = ENC_COMPLETE;
        e->failed = TRUE;
        XGM_startPlayPCMNextCh(SND_SAMPLE_EXPLOSION, 14);
        return TRUE;
    } else if (p1->what == WHAT_BANANA && p2->what == WHAT_SNAIL) {
        p1->collision = FALSE;
        p1->what = WHAT_PROP;
        e->state = ENC_COMPLETE;
        return TRUE;
    }
    return FALSE;
}
