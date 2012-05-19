/* memory.c */

/* The following are only for the simulator */
//#define SIMULATING 1
#if SIMULATING
#include <time.h>
static const struct timespec ts_req = {0, 100000};
#define YIELD() nanosleep(&ts_req, 0);
#else
#define YIELD()
#endif

/* ---------------------------------------- */

#include <memory.h>
#include <tqw.h>

/* ---------------------------------------- */
/* global definitions */

MemoryBlock mem;
static const DisplaySpec *spec;
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

    if (++mem.cur_col < spec->screen_width) spec->figureData();
    else mem.rbyte = mem.gbyte = mem.bbyte = 0;
}

static void rot_intr(void)
{
    mem.cur_col = 0;
    mem.rot_count++;
    spec->figureData();

    TQWSetColumnInterruptTimer
        (TQWGetAndResetRotationInterruptTime() / spec->screen_width);
}

static const DisplaySpec *spex[] = {
    &fire_spec};/*,
    &life_spec,
    &bowl_spec,
    &chord_spec,
    &sprite_spec,
    &krypton_spec};*/

int main(int argc, char *argv[])
{
    TQWInit();

    mem.rot_count = 0;
    spidx = 0;
    spec = spex[spidx];
    spec->init();
    spchange = false;

    TQWSetRotationInterrupt(rot_intr);
    TQWSetColumnInterrupt(col_intr);

    while (true) {
        if (spec->preinc) spec->preinc();
        while (mem.rot_count < spec->rot_count && !spchange) YIELD();
        if (spchange) {
            spchange = false;
            continue;
        }
        mem.rot_count = 0;
        if (spec->inc) spec->inc();
    }

    return 0;
}

/* Call this to change which display is being viewed.  Eventually
   attached to the remote control.
*/
void changeDisplay(void)
{
    TQWSetColumnInterrupt(0);
    mem.rot_count = 0;
    if (++spidx >= ARSIZE(spex)) spidx = 0;
    spec = spex[spidx];
    spec->init();
    spchange = true;
}
