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
fix16 exp2(fix16 x);
u16 arccossin(fix16 norm_x, fix16 norm_y);
void fix32print(char *label, fix32 x);
u16 intsqrt(s16 x);
u16 bigintsqrt(u32 x);
fix16 adaptiveFix32Log2(fix32 x);

/* Cool, Totally good Calloc */
void *ct_calloc(u16 nitems, u16 size);
u16 random_with_max(u16 max);
void change_song(const u8 *song);
char *heap_str(char *str);
void SPR_ensureAnim(Sprite* sprite, s16 anim);
void VDP_fillTileMapRectIncT(VDPPlane plane, u16 basetile, u16 x, u16 y, u16 w, u16 h);
extern u16 next_pcm_channel;
u16 XGM_startPlayPCMNextCh(const u8 id, const u8 priority);

#endif
