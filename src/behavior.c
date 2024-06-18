#include "bh.h"

void behave(Encounter *e, Physics *p) {
    switch (p->what) {
        case WHAT_DYNAMITE:
            if (p->z < FIXZ(0.1)) {
                Physics_new_explosion(
                    e,
                    p->x - FIXX(16),
                    p->y - FIXY(8));
                Physics_del(p, e);
                XGM_startPlayPCMNextCh(SND_SAMPLE_EXPLOSION, 14);
            }
            return;
        case WHAT_PARTICLE:
            if (p->z < FIXZ(0.1)) Physics_del(p, e);
            return;
        case WHAT_GOBLIN:
            switch (p->state) {
                case 0: //running
                    break;
                case 1: // throwing
                    if (p->state_frames == 6 * 2) {
                        u8 mock_counter; // TODO
                        fix16 dx, dy, dz;
                        if (p->ai_flag) {
                            dx = -(p->pending_throw_dx);
                            dy = p->pending_throw_dy;
                            dz = p->pending_throw_dz;
                        } else {
                            dx = FIX16(0.3) * p->charged;
                            dz = dx << 1;
                            dy = FIX16(0.2) * p->english;
                        }
                        if (p->f == RIGHT) {
                            Physics_new_dynamite(e, p->x + FIXX(24), p->y + FIXY(16), FIXZ(20), dx, dy, dz, &mock_counter); 
                        } else {
                            Physics_new_dynamite(e, p->x, p->y + FIXY(16), FIXZ(20), -dx, dy, dz, &mock_counter); 
                        }
                    } else if (p->state_frames >= 6 * 7) {
                        SPR_ensureAnim(p->sp, 0);
                        p->state = 0;
                        p->state_frames = 0;
                    }
                    break;
                default:
                    break;
            }
            return;
        case WHAT_EXPLOSION:
            if (p->frames_alive > 1) {
                p->collision = FALSE;
                p->calc_collisions = FALSE;
            }
            return;
        case WHAT_CASH:
            if (p->z == 0) p->dz = FIX16(3);
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
    if (p1->what == WHAT_PROP && p2->what == WHAT_EXPLOSION) {
        for (u8 i = 0; i < 8; ++i) {
            Physics_new_prop_particle(
                e,
                p1->col_x,
                p1->col_y,
                FIXZ(8), 
                FIXX(8 - random_with_max(16)) >> 2,
                FIXY(8 - random_with_max(16)) >> 2,
                FIXZ(random_with_max(16)) >> 2
            );
        }
        if (random_with_max(3) == 0) {
            Physics_new_cash(e, p1->col_x, p1->col_y);
        }
        Physics_del(p1, e);
        SPR_setDepth(p2->sp, 0x7FFF); // send to bottom
    } else if (p1->what == WHAT_GOBLIN && p2->what == WHAT_EXPLOSION) {

        // TODO dbg
        //return TRUE;

        for (u8 i = 0; i < 8; ++i) {
            Physics_new_goblin_particle(
                e,
                p2->col_x,
                p2->col_y,
                FIXZ(8), 
                FIXX(8 - random_with_max(16)) >> 2,
                FIXY(8 - random_with_max(16)) >> 2,
                FIXZ(random_with_max(16)) >> 2
            );
        }
        if (p1->pl->round_rank == 0) {
            p1->pl->round_rank = Physics_count(WHAT_GOBLIN);
        }
        Physics_del(p1, e);
    } else if (p1->what == WHAT_GOBLIN && p2->what == WHAT_CASH) {
        Physics_del(p2, e);
        if (p1->pl) ++(p1->pl->round_score);
        XGM_startPlayPCMNextCh(SND_SAMPLE_CASH, 14);
    } else if (p1->what == WHAT_PROP && p2->what == WHAT_GOBLIN) {
        p2->blocked = TRUE;
    } else if (p1->what == WHAT_GOBLIN && p2->what == WHAT_GOBLIN) {
        p2->blocked = TRUE;
    } else if (p1->what == WHAT_PROP && p2->what == WHAT_DYNAMITE) {
        return TRUE;
    }
    return FALSE;
}
