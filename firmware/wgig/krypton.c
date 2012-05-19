/* krypton.c */

#include "rand.h"
#include "memory.h"

#define SCREEN_SIZE 256

/* ---------------------------------------- */

static void figureData(void)
{
    signed short lowc, highc, temp;
    UChar si, m, color;

    mem.rbyte = mem.gbyte = mem.bbyte = 0;

    for (si = 0; si < ARSIZE(mem.d.k.hoops); si++) {
        lowc = mem.d.k.hoops[si].c, highc = lowc + SCREEN_SIZE / 2;
        while (highc < (short) mem.cur_col)
            highc += SCREEN_SIZE, lowc += SCREEN_SIZE;
        while (lowc > (short) mem.cur_col)
            highc -= SCREEN_SIZE, lowc -= SCREEN_SIZE;
        if (highc < (short) mem.cur_col)
            lowc += SCREEN_SIZE / 2, highc += SCREEN_SIZE / 2, temp = 1;
        else temp = 0;
        color = mem.d.k.hoops[si].color;
        if (temp) {
            m = 0x80 >> ((8 * (mem.cur_col - lowc)) / (highc - lowc));
        } else {
            m = 1 << ((8 * (mem.cur_col - lowc)) / (highc - lowc));
        }
        if (color & 0x04) mem.rbyte |= m;
        else mem.rbyte &= ~m;
        if (color & 0x02) mem.gbyte |= m;
        else mem.gbyte &= ~m;
        if (color & 0x01) mem.bbyte |= m;
        else mem.bbyte &= ~m;
    }
}

static void init_hoop(UChar idx)
{
    mem.d.k.hoops[idx].c = rand_num(0, SCREEN_SIZE);
    mem.d.k.hoops[idx].life = rand_num(50, 200);
    mem.d.k.hoops[idx].vn = 1;
    mem.d.k.hoops[idx].vd = 2;
    mem.d.k.hoops[idx].vc = 0;
    mem.d.k.hoops[idx].color = rand_num(1, 7);
}

static void inc(void)
{
    UChar si;

    for (si = 0; si < ARSIZE(mem.d.k.hoops); si++) {
        if (!--mem.d.k.hoops[si].life) {
            mem.d.k.hoops[si].life = rand_num(50, 200);
            mem.d.k.hoops[si].color = rand_num(1, 7);
        }

        if (++mem.d.k.hoops[si].vc >= mem.d.k.hoops[si].vd) {
            mem.d.k.hoops[si].c += mem.d.k.hoops[si].vn;
            mem.d.k.hoops[si].vc = 0;
        }
    }
}

static void init(void)
{
    UChar i;

    for (i = 0; i < ARSIZE(mem.d.k.hoops); i++) {
        init_hoop(i);
    }
}

const DisplaySpec krypton_spec = {
    DISP_SPEC(1, SCREEN_SIZE), figureData, init, 0, inc
};
