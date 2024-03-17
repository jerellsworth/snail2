#include "bh.h"

void behave(Encounter *e, Physics *p) {
    switch (p->what) {
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
