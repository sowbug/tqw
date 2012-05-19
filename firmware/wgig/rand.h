/* rand.h */

#ifndef _RAND_H_
#define _RAND_H_

typedef struct {
    unsigned short v;
} RandState;

short rand_num(short low, short high);

#endif
