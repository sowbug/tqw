/* chord.c */

#include "rand.h"
#include "memory.h"

/* ---------------------------------------- */
 /* global definitions */

#define SCREEN_SIZE 256

/* ---------------------------------------- */

static void figureData(void)
{
    signed short lowc, highc, temp;
    UChar si, m, color, t2;

    mem.rbyte = mem.gbyte = mem.bbyte = 0;

    for (si = 0; si < ARSIZE(mem.d.c.chords); si++) {
        highc = mem.d.c.chords[si].tc, lowc = mem.d.c.chords[si].bc;
        if (highc < lowc) temp = lowc, lowc = highc, highc = temp, temp = 1;
        else temp = 0;
        /* lowc is now min(tc, bc), highc is max(tc, bc);
           if temp = 1, highc = tc, else highc = bc;
         */
        while (highc < (short) mem.cur_col)
            highc += SCREEN_SIZE, lowc += SCREEN_SIZE;
        while (lowc > (short) mem.cur_col)
            highc -= SCREEN_SIZE, lowc -= SCREEN_SIZE;
        if (highc >= (short) mem.cur_col) {
            color = mem.d.c.chords[si].color;
            if (highc == lowc) m = 0xFF;
            else if (temp) {
                if (highc - lowc >= 8) {
                    m = 0x80 >> ((8 * (mem.cur_col - lowc)) / (highc - lowc));
                } else {
                    temp = 14 * (mem.cur_col - lowc) + 7;
                    t2 = 2 * (highc - lowc);
                    temp = 8 - ((temp + t2 - 1) / t2);
                    m = (1 << temp) - 1;
                    temp = 14 * (mem.cur_col - lowc) - 7;
                    t2 = 2 * (highc - lowc);
                    temp = 8 - ((temp + t2 - 1) / t2);
                    m = ((1 << temp) - 1) & ~m;
                }
            } else {
                if (highc - lowc >= 8) {
                    m = 1 << ((8 * (mem.cur_col - lowc)) / (highc - lowc));
                } else {
                    temp = 14 * (mem.cur_col - lowc) - 7;
                    t2 = 2 * (highc - lowc);
                    temp = (temp + t2 - 1) / t2;
                    m = (1 << temp) - 1;
                    temp = 14 * (mem.cur_col - lowc) + 7;
                    t2 = 2 * (highc - lowc);
                    temp = ((temp + t2 - 1) / t2);
                    m = ((1 << temp) - 1) & ~m;
                }
            }
            if (color & 0x04) mem.rbyte |= m;
            else mem.rbyte &= ~m;
            if (color & 0x02) mem.gbyte |= m;
            else mem.gbyte &= ~m;
            if (color & 0x01) mem.bbyte |= m;
            else mem.bbyte &= ~m;
        }
    }
}

static void init_chord(UChar idx)
{
    short tc = rand_num(0, SCREEN_SIZE);

    mem.d.c.chords[idx].tc = tc;
    mem.d.c.chords[idx].bc = tc + rand_num(-20, 20);
    mem.d.c.chords[idx].color = rand_num(1, 7);
    mem.d.c.chords[idx].life = rand_num(20, 100);
    mem.d.c.chords[idx].tvn = rand_num(0, 1) ? 1 : -1;
    mem.d.c.chords[idx].bvn = rand_num(0, 1) ? 1 : -1;
    mem.d.c.chords[idx].tvd = rand_num(1, 6);
    mem.d.c.chords[idx].bvd = rand_num(1, 6);
    mem.d.c.chords[idx].tvc = mem.d.c.chords[idx].bvc = 0;
}

static void inc(void)
{
    UChar si;

    for (si = 0; si < ARSIZE(mem.d.c.chords); si++) {
        if (mem.d.c.chords[si].life) {
            mem.d.c.chords[si].life--;
            if (++mem.d.c.chords[si].tvc >= mem.d.c.chords[si].tvd) {
                mem.d.c.chords[si].tc += mem.d.c.chords[si].tvn;
                mem.d.c.chords[si].tvc = 0;
            }
            if (++mem.d.c.chords[si].bvc >= mem.d.c.chords[si].bvd) {
                mem.d.c.chords[si].bc += mem.d.c.chords[si].bvn;
                mem.d.c.chords[si].bvc = 0;
            }
        } else if (rand_num(0, 3) == 3) {
            init_chord(si);
        }
    }
}

static void init(void)
{
    UChar i;

    for (i = 0; i < ARSIZE(mem.d.c.chords); i++) {
        init_chord(i);
    }
}

const DisplaySpec chord_spec = {
    DISP_SPEC(1, SCREEN_SIZE), figureData, init, 0, inc
};
