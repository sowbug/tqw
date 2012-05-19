/* wave.c */

#include "rand.h"
#include "memory.h"

#define SCREEN_SIZE 256

#define FP_SHIFT 6
#define FP_One (1 << FP_SHIFT)
#define FP_Half (FP_One >> 1)

#define AMP_INC (FP_One >> 5)
#define FREQ_INC (FP_One >> 4)
#define MAX_FREQ ((SCREEN_SIZE / 4) << FP_SHIFT)

/* ---------------------------------------- */

/* add 4 to get bit position. */
static CSChar Wave[] = {
    0,  0,  1,  1,  2,  2,  3,  3,  3,  3,  3,  2,  2,  1,  1,  0,
    0, -1, -2, -2, -3, -3, -4, -4, -4, -4, -4, -3, -3, -2, -2, -1};

/* returns a sample number in [0..ARSIZE(Wave)] */
static UChar get_obs(UChar col)
{
    signed short ac = col - SCREEN_SIZE / 2;

    ac = (ac * mem.d.w.freq * ARSIZE(Wave)) / (SCREEN_SIZE / 2);
    ac = (ac + FP_Half) >> FP_SHIFT;

    while (ac < 0) ac += ARSIZE(Wave);
    while (ac > ARSIZE(Wave)) ac -= ARSIZE(Wave);
    return (UChar) ac;
}

static void figureData(void)
{
    signed short sl, sr, sc, ts;
    UChar m, tmp;

    tmp = mem.d.w.bg;
    mem.rbyte = tmp & 0x04 ? 0xFF : 0;
    mem.gbyte = tmp & 0x02 ? 0xFF : 0;
    mem.bbyte = tmp & 0x01 ? 0xFF : 0;
    tmp = mem.cur_col;

    sc = get_obs(tmp);
    sl = get_obs(tmp ? tmp - 1 : SCREEN_SIZE - 1);
    sr = get_obs((tmp + 1) & (SCREEN_SIZE - 1));

    if (sl > sc) sl -= ARSIZE(Wave);
    if (sr < sc) sr += ARSIZE(Wave);
    sl = sl + 1 >= sc ? sc : (sl + sc) / 2;
    sr = sc >= sr + 1 ? sc : (sc + sr) / 2;

    for (m = 0, sc = sl; sc <= sr; sc++) {
        ts = PByte(&Wave[(sc < 0 ? sc + ARSIZE(Wave) :
                          sc > ARSIZE(Wave) ? sc - ARSIZE(Wave) : sc)]);
        ts = (ts * mem.d.w.amp + FP_Half) >> FP_SHIFT;
        m |= (1 << (4 + ts));
    }

    tmp = mem.d.w.fg;
    if (tmp & 0x04) mem.rbyte |= m;
    else mem.rbyte &= ~m;
    if (tmp & 0x02) mem.gbyte |= m;
    else mem.gbyte &= ~m;
    if (tmp & 0x01) mem.bbyte |= m;
    else mem.bbyte &= ~m;
}

static void inc(void)
{
    if (mem.d.w.amp < FP_One) mem.d.w.amp += AMP_INC;

    if ((mem.d.w.freq += FREQ_INC) >= MAX_FREQ) {
        mem.d.w.bg = mem.d.w.fg;
        mem.d.w.fg = rand_num(0, 7);
        if (mem.d.w.fg == mem.d.w.bg) {
            mem.d.w.fg = (mem.d.w.bg + 1) & 0x07;
        }            
        mem.d.w.freq = FP_Half;
        mem.d.w.amp = 0;
    }
}

static void init(void)
{
    mem.d.w.fg = rand_num(1, 7);
    mem.d.w.bg = 0;
    if (mem.d.w.bg == mem.d.w.fg) {
        mem.d.w.bg = (mem.d.w.fg + 1) & 0x07;
    }
    mem.d.w.freq = FP_Half;
    mem.d.w.amp = 0;
}

const DisplaySpec wave_spec = {
    DISP_SPEC(1, SCREEN_SIZE), figureData, init, 0, inc
};
