/* life.c */

#include "memory.h"

/* ---------------------------------------- */
/* global definitions */

#define SCREEN_SIZE LIFE_SCREEN_WIDTH
#define RGB(r, g, b) ((((r)) << 2) | (((g)) << 1) | ((b)))

typedef struct {
    /* bit pattern.  If bit[n] is set, then a living
       cell will survive with n neighbors.
    */
    short live;
    /* bit pattern.  If bit[n] is set, then an empty
       cell will generate a new unit.
    */
    short born;
} Rule;

/* these are the rules of life.
   Source: http://psoup.math.wisc.edu/mcell/rullex_life.html
*/
static Rule Rules[] PROGMEM = {
    {0x000C, 0x0004}, /* 23/3, Conway's original */
    {0x0129, 0x0054}, /* 1358/357, Amoeba */
    {0x0002, 0x0002}, /* 1/1, Gnarl */
    {0x00F0, 0x0038}, /* 4567/345 */
    {0x01E6, 0x0188}, /* 235678/378 */
    {0x0001, 0x0001}  /* 0/0 Ross's mistake */
};

#define NRULES ARSIZE(Rules)

/*
  these do not seem to work very well for a wgig.
    {0x01F0, 0x0008},  45678/3 
    {0x01E0, 0x01E8}   5678/35678 
*/

static CChar ctable[] = {
    RGB(0, 0, 0), /* no guy, black */
    RGB(0, 0, 1), /* dying guy, blue */
    RGB(1, 0, 0), /* old guy, red */
    RGB(1, 1, 1)  /* new guy, white */
};

/* ---------------------------------------- */

static void figureData(void)
{
    UShort idxs = mem.d.l.curstate[mem.cur_col];
    short i;
    UChar clr;

    for (i = 0; i < 8; i++, idxs >>= 2) {
        clr = PByte(&ctable[idxs & 3]);
        mem.rbyte = (mem.rbyte >> 1) | ((clr & 0x4) << 5);
        mem.gbyte = (mem.gbyte >> 1) | ((clr & 0x2) << 6);
        mem.bbyte = (mem.bbyte >> 1) | ((clr & 0x1) << 7);
    }
}

#define ALIVE(col, i) ((((col) >> (2 * (7 - (i)))) & 3) > 1)

static void neighbors(UShort col)
{
    UShort r;

    for (r = 0; r < 8; r++) {
        if (ALIVE(col, r)) {
            mem.d.l.counts[r]++;
            if (r > 0) mem.d.l.counts[r -1]++;
            if (r < 7) mem.d.l.counts[r + 1]++;
        }
    }
}

static void gen(void)
{
    UShort c, r, prevc, nextc, ov, nv;
    UShort *old = mem.d.l.curstate;
    UShort *new = mem.d.l.curstate == mem.d.l.state1 ?
        mem.d.l.state2 : mem.d.l.state1;
    Rule rule; /* 4 bytes */

    memcpy_P(&rule, &Rules[mem.d.l.rule_index], sizeof(rule));
    for (prevc = SCREEN_SIZE - 1, c = 0; c < SCREEN_SIZE; prevc = c, c++) {
        if ((nextc = c + 1) == SCREEN_SIZE) nextc = 0;
        for (r = 0; r < 8; r++) mem.d.l.counts[r] = 0;
        neighbors(old[prevc]);
        neighbors(old[c]);
        neighbors(old[nextc]);
        for (ov = old[c], nv = r = 0; r < 8; r++) {
            nv <<= 2;
            if (ALIVE(old[c], r)) {
                mem.d.l.counts[r]--; /* exclude myself from the count. */
                nv |= (rule.live & (1 << mem.d.l.counts[r])) ? 2 : 1;
            } else {
                nv |= (rule.born & (1 << mem.d.l.counts[r])) ? 3 : 0;
            }
        }
        new[c] = nv;
    }        
}

static void gen_init(UShort *state)
{
    UChar v1, v2;
    UShort b, i, j, idx;

    b = SCREEN_SIZE / 8;

    for (idx = j = 0; j < 2; j++) {
        v1 = 0x03, v2 = 0x00;
        for (i = 0; i < b; i++) state[idx++] = 0x0300;
        for (i = 0; i < b; i++) state[idx++] = 0;
        for (i = 0; i < b; i++) state[idx++] = 0x00C0;
        for (i = 0; i < b; i++) state[idx++] = 0;
    }

    mem.d.l.ngens = 0;
}

#define MAXGENS 32

static void init(void)
{
    mem.d.l.rule_index = 0;
    mem.d.l.curstate = mem.d.l.state1;
    gen_init(mem.d.l.state1); /* before interrupts are turned on. */
}

static void preinc(void)
{
    if (mem.d.l.ngens < MAXGENS) {
        gen();
        mem.d.l.ngens++;
    } else {
        if (++mem.d.l.rule_index >= NRULES)
            mem.d.l.rule_index = 0;
        gen_init(mem.d.l.curstate == mem.d.l.state1 ?
                 mem.d.l.state2 : mem.d.l.state1);
    }
}

static void inc(void)
{
    mem.d.l.curstate = mem.d.l.curstate == mem.d.l.state1 ?
        mem.d.l.state2 : mem.d.l.state1;
}

const DisplaySpec life_spec = {
    DISP_SPEC(210, SCREEN_SIZE), figureData, init, preinc, inc
};
