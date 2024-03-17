#ifndef BEHAVIOR_H
#define BEHAVIOR_H

#include "bh.h"

void behave(Encounter *enc, Physics *p);

// return False iff objects should also bounce off each other
bool interact(Enc *enc, Physics *p1, Physics *p2);

#endif
