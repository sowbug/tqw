/* random.c */

#include "memory.h"
#include "rand.h"

#define LCG_A 25173
#define LCG_C 13849

/* X(n+1) = (LCG_A*X(n) + LCG_C) modulo m */
short rand_num(short low, short high)
{
    short samp;

    mem.randstate.v *= LCG_A;
    mem.randstate.v += LCG_C;
    samp = mem.randstate.v >> 1;

    return (low >= high) ? low : (samp % ((high + 1) - low)) + low;    
}
