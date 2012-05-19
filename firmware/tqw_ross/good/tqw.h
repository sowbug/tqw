/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#if !defined(__TQW_H__)
#define __TQW_H__

#if defined(TQW_SIMULATOR)
#include <stdlib.h>
#else
#include <inttypes.h>
#include <io.h>
#include <string.h>
//#include </usr/local/avr/include/interrupt.h>
#include <sig-avr.h>
#include <pgmspace.h>
#endif

#define TRUE (1==1)
#define FALSE (!TRUE)

#define MCU_MEGAHERTZ (8)

typedef void (*TQWRotationInterruptFunc)(void);
typedef void (*TQWColumnInterruptFunc)(void);

/* Call TQWInit in main(), before doing anything else.
 */
void TQWInit(void);

/* TQWSetRotationInterrupt() takes a function that gets called each time 
 * the spinner starts a rotation.
 */
void TQWSetRotationInterrupt(TQWRotationInterruptFunc f);

/* TQWGetAndResetRotationInterruptTime() returns the number of cycles that 
 * have elapsed since the last time GetAndResetRotationInterruptTime 
 * was called.
 */
uint16_t TQWGetAndResetRotationInterruptTime(void);

/* TQWSetColumnInterrupt() takes a function that gets called every X 
 * cycles, where X is set by TQWSetColumnInterruptTimer().
 */
void TQWSetColumnInterrupt(TQWColumnInterruptFunc f);
void TQWSetColumnInterruptTimer(const uint8_t t);

/* TQWSleepColumnInterruptTimer() should be called when the program 
 * is done rendering a rotation. It tells the microcontroller that 
 * it could go to sleep now if it wanted to (if the motor stops 
 * running for some reason but the circuit stays active, for 
 * example). */
void TQWSleepColumnInterruptTimer(void);

/* TQWLoadLatch puts the given byte into the latch. Each bit corresponds 
 * to a pixel in the current column. The LSB is the top pixel.
 */
void TQWLoadLatch(const uint8_t c);

/* TQWStrobe[Red,Green,Blue] copy whatever's in the latch to the 
 * specified column of lights.
 */
void TQWStrobeRed(void);
void TQWStrobeGreen(void);
void TQWStrobeBlue(void);

/* Turn off all lights.
 */
void TQWExtinguishLights(void);

/* Delay for a number of units. Adjust to taste.
 */
void TQWDelay(const uint8_t c);

#endif /* #if !defined(__TQW_H__) */
