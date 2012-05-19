/* memory.c */

#include "memory.h"
#include "tqw.h"

/* ---------------------------------------- */
/* global definitions */

MemoryBlock mem;
static DispSpec spec;
static UChar spidx;
static UChar spchange;

/* ---------------------------------------- */

static void col_intr(void)
{
    TQWLoadLatch(mem.rbyte);
    TQWStrobeRed();
    TQWLoadLatch(mem.gbyte);
    TQWStrobeGreen();
    TQWLoadLatch(mem.bbyte);
    TQWStrobeBlue();

    if (++mem.cur_col < SCREEN_WIDTH(&spec)) spec.figureData();
    else mem.rbyte = mem.gbyte = mem.bbyte = 0;
}

static void rot_intr(void)
{
    mem.cur_col = 0;
    spec.figureData();

    TQWSetColumnInterrupt(col_intr);
    TQWSetColumnInterruptTimer
        (TQWGetAndResetRotationInterruptTime() / SCREEN_WIDTH(&spec));
}

static DisplaySpec const * const spex[] PROGMEM = {
#if ((MODEL == 1) || (MODEL == 0))
    &fire_spec,
    &sprite_spec,
    &life_spec,
    &wave_spec,
#endif
#if ((MODEL==2) || (MODEL==0))
    &bowl_spec,
    &chord_spec,
    &krypton_spec,
    &zipper_spec,
    &ringer_spec
#endif
};

int main(int argc, char *argv[])
{
    TQWInit();

    spidx = ARSIZE(spex);
    changeDisplay();
    spchange = false;

    TQWSetRotationInterrupt(rot_intr);

    while (true) {
        if (spec.preinc) spec.preinc();
        TQWDelay(DELAY(&spec));
        if (spchange) {
            spchange = false;
            continue;
        }
        mem.rot_count = 0;
        if (spec.inc) spec.inc();
    }

    return 0;
}

/* Call this to change which display is being viewed.  Eventually
   attached to the remote control.
*/
void changeDisplay(void)
{
    DispSpec const *sptr;

    TQWSetColumnInterrupt(0);
    mem.rot_count = 0;
    if (++spidx >= ARSIZE(spex)) spidx = 0;
    memcpy_P(&sptr, &spex[spidx], sizeof(sptr));
    memcpy_P(&spec, sptr, sizeof(spec));
    spec.init();
    spchange = true;
}
