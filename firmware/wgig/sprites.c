/* sprites.c */

#include "rand.h"
#include "memory.h"

/* ---------------------------------------- */
/* sprites */

/* movement rules:
   moves cvn forward every cvd time units.
   moves rvn (+/-1) up/down every rvd time units.
   If we reach either vertical limit, change sign of rvn
*/

static CChar s0dat[] = {0x18, 0x18};             /* square */
static CChar s1dat[] = {0x10, 0x38, 0x10};       /* + */
static CChar s2dat[] = {0x08, 0x18, 0x18, 0x08}; /* hat */
static CChar s3dat[] = {0x24, 0x18, 0x18, 0x24}; /* x */
static CChar s4dat[] = {0x18, 0x24, 0x24, 0x18}; /* o */

static CChar *Masks[] PROGMEM = {
    s0dat, s1dat, s2dat, s3dat, s4dat
};

static Guy Guys[] PROGMEM = {
    {ARSIZE(s0dat), -3, 3},
    {ARSIZE(s1dat), -3, 2},
    {ARSIZE(s2dat), -3, 3},
    {ARSIZE(s3dat), -2, 2},
    {ARSIZE(s4dat), -2, 2}
};

#define NGUYS ARSIZE(Guys)

/* ---------------------------------------- */
 /* global definitions */

#define SCREEN_SIZE 256 /* 128 XXX */

/* ---------------------------------------- */

static void figureData(void)
{
    signed short col;
    UChar si, gi, db, m, m2, color;

    mem.rbyte = mem.gbyte = mem.bbyte = 0;

    for (si = 0; si < ARSIZE(mem.d.s.sprites); si++) {
        if (mem.d.s.sprites[si].life) {
            gi = mem.d.s.sprites[si].gidx;
            memcpy_P(&mem.d.s.guy, &Guys[gi], sizeof(Guy));
            col = mem.d.s.sprites[si].c;
            if (col > (short) mem.cur_col) col -= SCREEN_SIZE;
            if (col <= (short) mem.cur_col &&
                col + mem.d.s.guy.width > (short) mem.cur_col) {
                color = mem.d.s.sprites[si].color;
                db = PByte(((CChar **) Masks)[gi] + (mem.cur_col - col));
                if (mem.d.s.sprites[si].r > 0) db >>= mem.d.s.sprites[si].r;
                else db <<= -mem.d.s.sprites[si].r;
                for (m = 0x80, m2 = 1; db && m; m >>= 1, m2 <<= 1) {
                    if (db & m) {
                        if (color & 0x04) mem.rbyte |= m2;
                        else mem.rbyte &= ~m2;
                        if (color & 0x02) mem.gbyte |= m2;
                        else mem.gbyte &= ~m2;
                        if (color & 0x01) mem.bbyte |= m2;
                        else mem.bbyte &= ~m2;
                    }
                }                    
            }
        }
    }
}

static void init_sprite(UChar idx)
{
    mem.d.s.sprites[idx].gidx = rand_num(0, NGUYS - 1);
    mem.d.s.sprites[idx].r = 0;
    mem.d.s.sprites[idx].c = rand_num(0, SCREEN_SIZE);
    mem.d.s.sprites[idx].color = rand_num(1, 7);
    mem.d.s.sprites[idx].life = rand_num(20, 100);
    mem.d.s.sprites[idx].cvn = rand_num(0, 1) ? 1 : -1;
    mem.d.s.sprites[idx].rvn = rand_num(0, 1) ? 1 : -1;
    mem.d.s.sprites[idx].cvd = rand_num(1, 5);
    mem.d.s.sprites[idx].rvd = rand_num(8, 13);
    mem.d.s.sprites[idx].cvc = mem.d.s.sprites[idx].rvc = 0;
}

static void inc(void)
{
    UChar si, gi;

    for (si = 0; si < ARSIZE(mem.d.s.sprites); si++) {
        if (mem.d.s.sprites[si].life) {
            if (++mem.d.s.sprites[si].cvc >= mem.d.s.sprites[si].cvd &&
                --mem.d.s.sprites[si].life) {
                mem.d.s.sprites[si].c += mem.d.s.sprites[si].cvn;
                mem.d.s.sprites[si].cvc = 0;
            }
            if (++mem.d.s.sprites[si].rvc >= mem.d.s.sprites[si].rvd) {
                gi = mem.d.s.sprites[si].gidx;
                mem.d.s.sprites[si].rvc = 0;
                mem.d.s.sprites[si].r += mem.d.s.sprites[si].rvn;
                memcpy_P(&mem.d.s.guyusr, &Guys[gi], sizeof(Guy));
                if (mem.d.s.sprites[si].r >= mem.d.s.guyusr.rmax ||
                    mem.d.s.sprites[si].r <= mem.d.s.guyusr.rmin) {
                    mem.d.s.sprites[si].rvn = -mem.d.s.sprites[si].rvn;
                }
            }
        } else if (rand_num(0, 3) == 3) {
            init_sprite(si);
        }
    }
}

static void init(void)
{
    UChar i;

    for (i = 0; i < ARSIZE(mem.d.s.sprites); i++) {
        init_sprite(i);
    }
}

const DisplaySpec sprite_spec = {
    DISP_SPEC(1, SCREEN_SIZE), figureData, init, 0, inc
};
