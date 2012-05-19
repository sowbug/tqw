/* tqw.h */

#ifndef _TQW_H_
#define _TQW_H_ 1

#include "memory.h"

void TQWInit(void); /* Just call this before whatever else. */

typedef void (*vfunc)(void);

void TQWSetRotationInterrupt(vfunc f);
unsigned short TQWGetAndResetRotationInterruptTime(void);
 
void TQWSetColumnInterrupt(vfunc f);
void TQWSetColumnInterruptTimer(UChar t);

void TQWLoadLatch(UChar c);
void TQWStrobeRed(void);
void TQWStrobeGreen(void);
void TQWStrobeBlue(void);

/* units of d are about 4.762 milliseconds. */
void TQWDelay(UChar d);

#endif
