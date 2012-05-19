/* zipper.c */

#include "rand.h"
#include "memory.h"

#define SCREEN_SIZE 256 /* 128 XXX */
#define Z(n) mem.d.z.zips[n]

typedef enum {ph_idle, ph_lead, ph_wait, ph_follow} Phase;

/* ---------------------------------------- */

static void figureData(void)
{
    UChar i, j, m, minr, maxr, color;

    mem.rbyte = mem.gbyte = mem.bbyte = 0;

    for (i = 0; i < ARSIZE(mem.d.z.zips); i++) {
        minr = 0, maxr = 7;
        switch (Z(i).phase) {
          case ph_idle: minr = maxr = 9; break;
          case ph_lead: maxr = Z(i).currow; break;
          case ph_wait: break;
          case ph_follow: minr = Z(i).currow; break;
        }
        if (minr < 8) {
            color = Z(i).color;
            for (j = minr, m = 0x80 >> minr; j <= maxr; j++, m >>= 1) {
                if (Z(i).pv[j] == mem.cur_col) {
                    if (color & 0x04) mem.rbyte |= m;
                    else mem.rbyte &= ~m;
                    if (color & 0x02) mem.gbyte |= m;
                    else mem.gbyte &= ~m;
                    if (color & 0x01) mem.bbyte |= m;
                    else mem.bbyte &= ~m;
                }
            }
        }
    }
}

static void init_zipper(UChar idx)
{
    UShort cmax;

    Z(idx).cmin = rand_num(0, SCREEN_SIZE);
    Z(idx).cmax = cmax = (UShort) Z(idx).cmin + rand_num(3, 8);
    Z(idx).color = rand_num(1, 7);
    Z(idx).currow = 0;
    Z(idx).curcol = rand_num(Z(idx).cmin, cmax);
    Z(idx).pv[0] = Z(idx).curcol;
    Z(idx).vn = rand_num(0, 1) ? 1 : -1;
    Z(idx).vd = rand_num(3, 8);
    Z(idx).delc = 0;
    Z(idx).phase = ph_lead;
}

static void inc(void)
{
    UChar i, limit;

    for (i = 0; i < ARSIZE(mem.d.z.zips); i++) {
        switch (Z(i).phase) {
          case ph_idle:
            if (rand_num(0, 3) == 3) init_zipper(i);
            break;                    
          case ph_lead:
            if (++Z(i).delc == Z(i).vd) {
                limit = Z(i).vn > 0 ? Z(i).cmax : Z(i).cmin;
                if (Z(i).curcol == limit) {
                    Z(i).vn = -Z(i).vn;
                }
                Z(i).curcol += Z(i).vn;
                Z(i).currow += 1;
                Z(i).pv[Z(i).currow] = Z(i).curcol;

                if (Z(i).currow == 7) {
                    Z(i).delc = rand_num(1, 50);
                    Z(i).phase = ph_wait;
                } else Z(i).delc = 0;
            }
            break;
          case ph_wait:
            if (Z(i).delc == 0) {
                Z(i).phase = ph_follow;
                Z(i).currow = 0;
            } else Z(i).delc--;
            break;
          case ph_follow:
            if (++Z(i).delc == Z(i).vd) {
                Z(i).delc = 0;
                if (++Z(i).currow > 7) {
                    Z(i).phase = ph_idle;
                }
            }
            break;
        }
    }
}

static void init(void)
{
    UChar i;

    for (i = 0; i < ARSIZE(mem.d.z.zips); i++) {
        init_zipper(i);
    }
}

const DisplaySpec zipper_spec = {
    DISP_SPEC(1, SCREEN_SIZE), figureData, init, 0, inc
};
