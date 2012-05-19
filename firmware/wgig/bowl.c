/* bowl.c */

#include "rand.h"
#include "memory.h"

#define SCREEN_SIZE 256 /* 128 XXX */

/* ---------------------------------------- */
/* fish */

/* movement rules:
   moves vcn forward every vcd time units.
   move rand(-1, +1) up/down every vcd time units.

   Fish may not move such that row 3 is outside the vertical limits
   imposed be rmin, rmax.  If at limit, rand limits of vertical motion
   are adjusted appropriately.
*/

static CChar f0dat[] = {0x1c, 0x08, 0x18, 0x18, 0x3c, 0x18, 0x18, 0x08};
static CChar f1dat[] = {
    0x66, 0x18, 0x08, 0x18, 0x1c, 0x5c, 0x7c, 0x3e, 0x1e, 0x1c, 0x1c, 0x08};
static CChar f2dat[] = {0x2e, 0x70, 0x7e, 0x70, 0x2e};
static CChar f3dat[] = {
    0x28, 0x10, 0x10, 0x10, 0x10, 0x38, 0x10, 0x10, 0x10, 0x10};
static CChar f4dat[] = {
    0x66, 0x3c, 0x18, 0x1c, 0x3c, 0x3c, 0x3c, 0x18, 0x10};
static CChar f5dat[] = {
    0x42, 0x24, 0x98, 0x99, 0x7b, 0x7f, 0x7e, 0x3c, 0x1c, 0x08};
static CChar f6dat[] = {
    0x20, 0x30, 0x18, 0x0c, 0x04, 0x04, 0x04, 0x0c,
    0x18, 0x30, 0x20, 0x20, 0x30, 0x18, 0x18};
static CChar f7dat[] = {
    0x3c, 0x18, 0x18, 0x38, 0x78, 0x7e, 0x38, 0x38,
    0x18, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};

static CChar *Masks[] PROGMEM = {
    f0dat,
    f1dat,
    f2dat,
    f3dat,
    f4dat,
    f5dat,
    f6dat,
    f7dat
};

static const Fish Fishes[] PROGMEM = {
    {ARSIZE(f0dat), -2, 3, 1, 4},
    {ARSIZE(f1dat), -3, 4, 1, 2},
    {ARSIZE(f2dat), -1, 2, 1, 20},
    {ARSIZE(f3dat), -3, 4, 1, 1},
    {ARSIZE(f4dat), -2, 2, 1, 6},
    {ARSIZE(f5dat), -2, 3, 1, 8},
    {ARSIZE(f6dat), -2, 2, 1, 2},
    {ARSIZE(f7dat), -2, 3, 1, 4}
};

#define NFISH ARSIZE(Fishes)

/* ---------------------------------------- */

static void figureData(void)
{
    short col; /* signed. */
    UChar si, fi, db, m, m2, color;

    mem.rbyte = mem.gbyte = mem.bbyte = 0;

    for (si = 0; si < ARSIZE(mem.d.b.sprites); si++) {
        fi = mem.d.b.sprites[si].fidx;
        memcpy_P(&mem.d.b.fish_int, &Fishes[fi], sizeof(Fish));
        col = mem.d.b.sprites[si].c;
        if (col > (short) mem.cur_col) col -= SCREEN_SIZE;
        if (col <= (short) mem.cur_col &&
            col + mem.d.b.fish_int.width > (short) mem.cur_col) {
            color = mem.d.b.sprites[si].color;
            col = mem.cur_col - col;
            if (mem.d.b.sprites[si].revx) col = mem.d.b.fish_int.width - col - 1;
            db = PByte(((UChar **) Masks)[fi] + col);
            if (mem.d.b.sprites[si].r > 0) db >>= mem.d.b.sprites[si].r;
            else db <<= -mem.d.b.sprites[si].r;
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

static void inc(void)
{
    UShort col;
    UChar si, fi;
    signed char mn, mx;

    for (si = 0; si < ARSIZE(mem.d.b.sprites); si++) {
        fi = mem.d.b.sprites[si].fidx;
        memcpy_P(&mem.d.b.fish, &Fishes[fi], sizeof(Fish));
        if (++mem.d.b.sprites[si].vc >= mem.d.b.fish.vcd) {
            mem.d.b.sprites[si].vc = 0;
            if (mem.d.b.sprites[si].revx) {
                col = mem.d.b.sprites[si].c;
                if (col < mem.d.b.fish.vcn) col += SCREEN_SIZE;
                mem.d.b.sprites[si].c = col - mem.d.b.fish.vcn;
            } else {
                col = (UShort) mem.d.b.sprites[si].c + mem.d.b.fish.vcn;
                if (col >= SCREEN_SIZE) col -= SCREEN_SIZE;
                mem.d.b.sprites[si].c = col;
            }
            mx = (mem.d.b.sprites[si].r >= mem.d.b.fish.rmax) ? 0 : 5;
            mn = (mem.d.b.sprites[si].r <= mem.d.b.fish.rmin) ? 0 : -5;
            mem.d.b.sprites[si].r += rand_num(mn, mx) / 5;
            if (rand_num(0, 40) == 40)
                mem.d.b.sprites[si].revx ^= 1;
        }
    }
}

static void init(void)
{
    UChar i, fi;

    for (i = 0; i < ARSIZE(mem.d.b.sprites); i++) {
        mem.d.b.sprites[i].fidx = fi = rand_num(0, NFISH - 1);
        memcpy_P(&mem.d.b.fish, &Fishes[fi], sizeof(Fish));
        mem.d.b.sprites[i].r = rand_num(mem.d.b.fish.rmin, mem.d.b.fish.rmax);
        mem.d.b.sprites[i].c = rand_num(0, SCREEN_SIZE);
        mem.d.b.sprites[i].color = rand_num(1, 7);
        mem.d.b.sprites[i].vc = 0;
        mem.d.b.sprites[i].revx = rand_num(0, 1);
    }
}

const DisplaySpec bowl_spec = {
    DISP_SPEC(1, SCREEN_SIZE), figureData, init, 0, inc
};
