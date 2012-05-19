/* fire.c */

#include "rand.h"
#include "memory.h"

/* ---------------------------------------- */
/* global definitions */

#define SCREEN_SIZE FIRE_SCREEN_WIDTH
#define RGB(r, g, b) ((((r)) << 2) | (((g)) << 1) | ((b)))

static CChar ctable[] = {
    RGB(0, 0, 1),
    RGB(0, 1, 1),
    RGB(0, 1, 0),
    RGB(1, 1, 0),
    RGB(1, 0, 0),
    RGB(1, 0, 0),
    RGB(0, 0, 0)
};

/* ---------------------------------------- */

static void figureData(void)
{
    short i, ci = 4 * mem.cur_col;
    UChar b1, b2;

    for (i = 0; i < 4; i++, ci++) {
        b1 = PByte(&ctable[mem.d.f.ftable[ci] >> 4]);
        b2 = PByte(&ctable[mem.d.f.ftable[ci] & 0x0F]);
        mem.rbyte = (mem.rbyte << 2) | (((b1 >> 2) & 1) << 1) | ((b2 >> 2) & 1);
        mem.gbyte = (mem.gbyte << 2) | (((b1 >> 1) & 1) << 1) | ((b2 >> 1) & 1);
        mem.bbyte = (mem.bbyte << 2) | ((b1 & 1) << 1) | (b2 & 1);
    }
}

static void fire_inc(void)
{
    short c, r, i, ci;

    for (i = c = 0; c < SCREEN_SIZE; c++, i += 4) {
        for (r = 0; r < 4; r++) {
            mem.d.f.work[r * 2] = mem.d.f.ftable[i + r] >> 4;
            mem.d.f.work[r * 2 + 1] = mem.d.f.ftable[i + r] & 0x0F;
        }
        for (r = 7; r > 0; r--) {
            ci = 1 + mem.d.f.work[r - 1];
            mem.d.f.work[r] = ci == ARSIZE(ctable) ? ci - 1 : ci;
        }

        ci = mem.d.f.work[0];
        if (ci + 1 == ARSIZE(ctable))
            ci = rand_num(0, ARSIZE(ctable) - 1);
        else if (!ci) ci = rand_num(0, 2);
        else ci = rand_num(ci - 1,  ci + 1);
        mem.d.f.work[0] = ci >= ARSIZE(ctable) ?
            ARSIZE(ctable) - 1 : ci;

        for (r = 0; r < 4; r++) {
            mem.d.f.ftable[i + r] =
                (mem.d.f.work[r * 2] << 4) | mem.d.f.work[r * 2 + 1];
        }
    }
}

static void fire_init(void)
{
    UChar v = ARSIZE(ctable) - 1;
    short i;

    v |= (v << 4);
    for (i = 0; i < ARSIZE(mem.d.f.ftable); i++) mem.d.f.ftable[i] = v;
}

const DisplaySpec fire_spec = {
    DISP_SPEC(15, SCREEN_SIZE), figureData, fire_init, 0, fire_inc
};
