#ifndef UTILS_H
#define UTILS_H

#include "bh.h"

#define sgn(x) ((x > 0) - (x < 0))
#define LN2 FIX16(0.69)

void trajectory(
    u16 angle,
    fix16 *dx,
    fix16 *dy
    );

void normalize(fix16 x, fix16 y, fix16 v, fix16 *norm_x, fix16 *norm_y);
fix16 norm1d(fix16 dx);
u16 arccossin(fix16 norm_x, fix16 norm_y);
void fix32print(char *label, fix32 x);
u16 intsqrt(s16 x);

/* Cool, Totally good Calloc */
void *ct_calloc(u16 nitems, u16 size);
u16 random_with_max(u16 max);
void change_song(const u8 *song);
char *heap_str(char *str);
#endif
