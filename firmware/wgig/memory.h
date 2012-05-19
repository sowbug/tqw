/* memory.h */

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "rand.h"

#define ARSIZE(x) ((short) (sizeof(x)/sizeof(x[0])))

typedef enum {false = 0, true = 1} bool;
typedef unsigned char UChar;
typedef unsigned short UShort;
typedef unsigned long ULong;

/* these types go into the text memory as read-only global data. */
#ifdef SIMULATING
#include <string.h>
#define PROGMEM
typedef void const *PGM_VOID_P;
#define memcpy_P(a,b,c) memcpy(a,b,c)
#define PByte(x) (*(x))
#else
#include <pgmspace.h>

#define PByte(x) __lpm_inline((UShort) x)
#endif

typedef const UChar CChar PROGMEM;
typedef const UShort CShort PROGMEM;
typedef const signed char CSChar PROGMEM;
typedef const signed short CSShort PROGMEM;
typedef const ULong CLong PROGMEM;

typedef struct {
    ULong dispconsts; /* (rot_count << 16) | screen_width) */
    void (*figureData)(void);
    void (*init)(void);
    void (*preinc)(void);
    void (*inc)(void);
} DispSpec;

#ifdef SIMULATING
typedef DispSpec DisplaySpec;
#else
typedef DispSpec DisplaySpec PROGMEM;
#endif

#define SCREEN_WIDTH(spec) (((spec)->dispconsts) >> 16)
#define DELAY(spec) (((spec)->dispconsts) & 0xFFFF)
#define DISP_SPEC(del, sw) ((((long) sw) << 16) | del)

/* ======== display specific definitions ======== */

#define FIRE_SCREEN_WIDTH 128
#define LIFE_SCREEN_WIDTH 128

/* ---- sprites ---- */

#define MAXSPRITES 30 /* must be < 255 */

typedef struct {
    UChar width;      /* width */
    signed char rmin, rmax; /* vertical movement limits. */
} Guy;

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
    UChar width;      /* width */
    signed char rmin, rmax; /* vertical movement limits. */
    UChar vcn, vcd;   /* velocity numerator/denomenator */
} Fish;

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

/* ---- zipper ---- */

#define MAXZIPPERS 25 /* must be < 255 */

typedef struct {
    UChar cmin, cmax, color;
    UChar pv[8];
    UChar currow, curcol;
    signed char vn;
    UChar vd, delc, phase;
} Zipper;

/* ---- ---- */

typedef struct {
    UShort rot_count, cur_col;
    UChar rbyte, gbyte, bbyte;
    RandState randstate;
    union {
        struct {
            FSprite sprites[MAXFISH];
            Fish fish, fish_int;
        } b; /* bowl */
        struct {
            Chord chords[MAXCHORDS];
        } c; /* chords */
        struct {
            /* 4 bits /pixel, index into ctable.
               column 0 goes from ftable[0]..ftable[4] (bottom to top) */
            UChar ftable[FIRE_SCREEN_WIDTH * 4];
            UChar work[8];
        } f; /* fire */
        struct {
            Hoop hoops[MAXHOOPS];
        } k; /* krypton */
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
            UChar row;
            UChar delc;
            UChar color;
        } r; /* ringer */
        struct {
            Sprite sprites[MAXSPRITES];
            Guy guy; /* for interrupt routine */
            Guy guyusr; /* for user level code */
        } s; /* sprites */
        struct {
            UChar fg, bg;
            UShort amp, freq; /* 10.6 fixed point */
        } w; /* wave */
        struct {
            Zipper zips[MAXZIPPERS];
        } z; /* zipper */
    } d;
} MemoryBlock;

extern MemoryBlock mem;

/* -------- */

extern const DisplaySpec bowl_spec;
extern const DisplaySpec chord_spec;
extern const DisplaySpec fire_spec;
extern const DisplaySpec krypton_spec;
extern const DisplaySpec life_spec;
extern const DisplaySpec ringer_spec;
extern const DisplaySpec sprite_spec;
extern const DisplaySpec testpat_spec;
extern const DisplaySpec wave_spec;
extern const DisplaySpec zipper_spec;

/* call back to the memory module. */
void changeDisplay(void);

#endif
