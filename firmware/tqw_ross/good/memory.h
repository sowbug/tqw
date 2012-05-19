/* memory.h */

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "rand.h"

#define ARSIZE(x) (sizeof(x)/sizeof(x[0]))

typedef enum {false = 0, true = 1} bool;
typedef unsigned char UChar;
typedef unsigned short UShort;

typedef struct {
    UShort rot_count; /* call inc() every rot_count rotations */
    UShort screen_width;
    void (*figureData)(void);
    void (*init)(void);
    void (*preinc)(void);
    void (*inc)(void);
} DisplaySpec;

/* ======== display specific definitions ======== */

#define FIRE_SCREEN_WIDTH 128
#define LIFE_SCREEN_WIDTH 128

/* ---- sprites ---- */

#define MAXSPRITES 30 /* must be < 255 */

/* kill a sprite when life counter gets to zero.
   life counter decremented every time cvc=cvn. (horizontal motion)
*/
typedef struct {
    signed char r;  /* signed, could be negative */
    UChar c, gidx, color, life;
    signed char  cvn, rvn; /* signed velocity numerators */
    UChar cvd, rvd; /* unsigned velocity denomenators */
    UChar cvc, rvc; /* col/row velocity counts */
} Sprite;

/* ---- bowl ---- */

#define MAXFISH 10 /* must be < 255 */

typedef struct {
    signed char r;  /* signed, could be negative */
    UChar c, fidx, color;
    UChar vc;       /* velocity counts */
    UChar revx;     /* true if this fish is meant to be flipped horizontally */
} FSprite;

/* ---- chord ---- */

#define MAXCHORDS 20
typedef struct {
    signed short bc, tc; /* wraps around, may be negative */
    signed char tvn, bvn;
    UChar bvd, tvd;
    UChar tvc, bvc;
    UChar life, color;
} Chord;

/* ---- krypton ---- */

#define MAXHOOPS 1
typedef struct {
    UChar short c;
    signed char vn;
    UChar vd, vc;
    UChar life, color;
} Hoop;

/* ---- ---- */

typedef struct {
    UShort rot_count, cur_col;
    UChar rbyte, gbyte, bbyte;
    RandState randstate;
    union {
        struct {
            /* 4 bits /pixel, index into ctable.
               column 0 goes from ftable[0]..ftable[4] (bottom to top) */
            UChar ftable[FIRE_SCREEN_WIDTH * 4];
            UChar work[8];
        } f; /* fire */
#if 0
        struct {
            FSprite sprites[MAXFISH];
        } b; /* bowl */
        struct {
            Sprite sprites[MAXSPRITES];
        } s; /* sprites */
        struct{
            /* 2 bits /pixel, index into ctable.
               column 0 goes from ftable[0]..ftable[4] (bottom to top)
            */
            UShort state1[LIFE_SCREEN_WIDTH], state2[LIFE_SCREEN_WIDTH];
            UChar counts[8];
            UShort *curstate;
            short rule_index, ngens;
        } l; /* life */
        struct {
            Chord chords[MAXCHORDS];
        } c; /* chords */
        struct {
            Hoop hoops[MAXHOOPS];
        } k; /* krypton */
#endif
    } d;
} MemoryBlock;

extern MemoryBlock mem;

/* -------- */

extern const DisplaySpec fire_spec;
extern const DisplaySpec life_spec;
extern const DisplaySpec bowl_spec;
extern const DisplaySpec chord_spec;
extern const DisplaySpec sprite_spec;
extern const DisplaySpec krypton_spec;

#endif
