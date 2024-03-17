#include "bh.h"

void trajectory(
    u16 angle,
    fix16 *dx,
    fix16 *dy
    ) {

    *dx = cosFix16(angle);
    *dy = sinFix16(angle);
}

fix16 fix16IntPow(fix16 x, u8 y) {
    fix16 ret = FIX16(1);
    for (u8 i = 0; i < y; ++i) {
        ret = fix16Mul(ret, x);
    }
    return ret;
}

fix16 exp2(fix16 x) {
    /* 
     * Estimate fractional exponent by Taylor series.
     * This is presumably the slow part, but it's only 4
     * multiplications so could be worse.
     * (https://math.stackexchange.com/a/4163478)
     */
    fix16 ln2x = fix16Mul(LN2, x);
    fix16 ln2x_2 = fix16Mul(ln2x, ln2x);
    fix16 ln2x_3 = fix16Mul(ln2x_2, ln2x);
    return FIX16(1) + \
         ln2x + \
         (ln2x_2 >> 1) + \
         fix16Mul(ln2x_3, FIX16(0.17));
}

fix16 adaptiveFix32Log2(fix32 x) {
    // Use appropriate log function depending on size of number
    if (x <= FIX32(511)) {
        return fix16Log2(fix32ToFix16(x));
    }
    return FIX16(getLog2Int(fix32ToRoundedInt(x)));
}

void normalize(fix16 x, fix16 y, fix16 v, fix16 *norm_x, fix16 *norm_y) {
    /*
     * Normalize a vector and multiply by velocity.
     * x, y are coordinates
     * v is velocity.
     * result goes in *norm_x and *norm_y
     *
     * We solve in log2 space first to help with numerical stability.
     * Fix16 really doesn't have a lot of precision, so conventional methods will
     * push normalized vectors towards 0 length.
     *
     * The equation works out to:
     * norm_x = (v * x) / (sqrt(x^2 + y^2))
     * => norm_x = 2^(log2(v) + log2(x) - log2(x^2 + y^2)/2)
     *
     * SGDK provides precalc'd log2 tables, so the only expensive part is
     * exponentiation, which we do with a small Taylor series in exp2.
     */

    // since we're in logspace, we work in the (+,+) quadrant and fix sign later
    if (x == 0 && y == 0) {
        *norm_x = 0;
        *norm_y = 0;
        return;
    } else if (x == 0) {
        *norm_x = 0;
        *norm_y = y >= 0 ? v : -v;
        return;
    } else if (y == 0) {
        *norm_x = x >= 0 ? v : -v;
        *norm_y = 0;
        return;
    }

    bool x_pos = x > 0;
    bool y_pos = y > 0;
    if (!x_pos) x = -x;
    if (!y_pos) y = -y;

    /* 
     * We can't guarantee that the sum of two squared fix16s fit in fix16, so
     * we briefly expand to fix32 math. This is more expensive on the 68k,
     * so we get back into fix16 as soon as possible
     */
    fix32 x_rd = fix16ToFix32(x);
    fix32 y_rd = fix16ToFix32(y);
    fix32 dist_sq = fix32Mul(x_rd, x_rd) + fix32Mul(y_rd, y_rd);
    /*
     * Now that we're in logspace, we can go back to fix16 since log2(x^2 + y^2)
     * is a much smaller number than x^2 + y^2
     */
    fix16 dist_exp = adaptiveFix32Log2(dist_sq) >> 1;

    /*
     * Implementing:
     * norm_x = 2^(log2(v) + log2(x) - log2(x^2 + y^2)/2)
     * Note that dist_exp has already been divided by 2 in the previous step
     */
    fix16 log2v = fix16Log2(v);
    fix16 raw_norm_x = exp2(log2v + fix16Log2(x) - dist_exp);
    fix16 raw_norm_y = exp2(log2v + fix16Log2(y) - dist_exp);

    /*
     * Here we fix the sign and clamp to velocity. A normalized vector
     * has length <= 1, so a normalized vector * velocity has a length
     * <= v. Sometimes, due to innacuracy, we end up with length > v,
     * so we just artificially fix it.
     */
    *norm_x = x_pos ? min(raw_norm_x, v) : max(-raw_norm_x, -v);
    *norm_y = y_pos ? min(raw_norm_y, v) : max(-raw_norm_y, -v);
}

fix16 norm1d(fix16 x) {
    if (x == 0) return 0;
    return x < 0 ? FIX16(-1) : FIX16(1);
}

u16 arccossin(fix16 norm_x, fix16 norm_y) {
    u16 acos;
    if (norm_x == FIX16(1)) {
        acos = 0;
    } else if (norm_x == FIX16(-1)) {
        acos = 512;    
    } else if (norm_x >= 0) {
        acos = PC_ACOS_POS[(u8)(norm_x & FIX16_FRAC_MASK)];
    } else {
        acos = PC_ACOS_NEG[(u8)(norm_x & FIX16_FRAC_MASK)];
    }
    if (norm_y >= 0) return acos;
    return 1024 - acos;
}

u16 intsqrt(s16 x) {
    // https://math.stackexchange.com/a/2469481
    if (x < 2) return x;
    u16 m = 2 * intsqrt(x >> 2);
    u16 m_plus_1 = m + 1;
    if (m_plus_1 * m_plus_1 > x) return m;
    return m_plus_1;
}

int dbgLine = 8;

void fix32print(char *label, fix32 x) {
    char buf[32];
    strcpy(buf, label);
    u8 idx = strlen(buf);
    buf[idx] = ':';
    idx++;
    buf[idx] = ' ';
    idx++;
    fix32ToStr(x, buf + idx, 4);
    VDP_drawText(buf, 10, dbgLine);
    ++dbgLine;
}

void *ct_calloc(u16 nitems, u16 size) {
    u16 bytes = nitems * size;
    void *p = malloc(bytes);
    if (!p) {
        VDP_drawTextBG(BG_A, "NULL POINTER RETURNED FROM MALLOC", 1, 1);
        while (TRUE) {
            SYS_doVBlankProcess();
            VDP_waitVSync();
        }
    }
    memset(p, 0, bytes);
    return p;
}

u16 random_with_max(u16 max) {
    u16 mask;
    if (max <= 1) {
        mask = 1;
    } else if (max <= 3) {
        mask = 3;
    } else if (max <= 7) {
        mask = 7;
    } else if (max <= 15) {
        mask = 15;
    } else if (max <= 31) {
        mask = 31;
    } else if (max <= 63) {
        mask = 63;
    } else if (max <= 127) {
        mask = 127;
    } else if (max <= 255) {
        mask = 255;
    } else if (max <= 511) {
        mask = 511;
    } else if (max <= 1023) {
        mask = 1023;
    } else if (max <= 2047) {
        mask = 2047;
    } else if (max <= 4095) {
        mask = 4095;
    } else if (max <= 8191) {
        mask = 8191;
    } else if (max <= 16383) {
        mask = 16383;
    } else if (max <= 32767) {
        mask = 32767;
    } else {
        mask = 65535;
    }
    u16 r;
    while (TRUE) {
        r = random() & mask;
        if (r <= max) return r;
    }
}

void change_song(const u8 *song) {
    if (XGM_isPlaying()) XGM_pausePlay();
    XGM_stopPlayPCM(SOUND_PCM_CH1);
    VDP_waitVSync();
    VDP_waitVSync();
    VDP_waitVSync();
    XGM_startPlay(song);
}

char *heap_str(char *str) {
    char *new = ct_calloc(sizeof(char), strlen(str) + 1);
    strcpy(new, str);
    return new;
}
