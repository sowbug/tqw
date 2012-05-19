/* ringer.c */

#include "rand.h"
#include "memory.h"

#define SCREEN_SIZE 128

/* ---------------------------------------- */

static void figureData(void)
{
    UChar row, color, m;

    mem.rbyte = mem.gbyte = mem.bbyte = 0;

    row = mem.d.r.row;
    if (row < 8) {
        m = 0x80 >> row;
        color = mem.d.r.color;
        if (color & 4) mem.rbyte = m;
        if (color & 2) mem.gbyte = m;
        if (color & 1) mem.bbyte = m;
    }
}

#define IVL 4
#define DELC ((210 - 8) / IVL)

static void init(void)
{
    mem.d.r.color = rand_num(1, 7);
    mem.d.r.delc = DELC;
    mem.d.r.row = 0;
}

static void inc(void)
{
    if (mem.d.r.row < 8) {
        mem.d.r.row++;
    } else if (mem.d.r.delc) {
        mem.d.r.delc--;
    } else {
        init();
    }
}

const DisplaySpec ringer_spec = {
    DISP_SPEC(IVL, SCREEN_SIZE), figureData, init, 0, inc
};
