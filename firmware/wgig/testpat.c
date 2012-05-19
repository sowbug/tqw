/* testpat.c */

#include "rand.h"
#include "memory.h"

/* ---------------------------------------- */
/* global definitions */

#define SCREEN_WIDTH 128

/* ---------------------------------------- */

static void figureData(void)
{
    UChar color = mem.cur_col & 7;

    if (mem.cur_col < SCREEN_WIDTH / 2) {
        mem.rbyte = 0xF0;
        mem.gbyte = 0xCC;
        mem.bbyte = 0xAA;
    } else {
        mem.rbyte = color & 4 ? 0xFF : 0;
        mem.gbyte = color & 2 ? 0xFF : 0;
        mem.bbyte = color & 1 ? 0xFF : 0;
    }
}

static void inc(void) {}

static void init(void) {}

const DisplaySpec testpat_spec = {
    DISP_SPEC(255, SCREEN_WIDTH), figureData, init, 0, inc
};
