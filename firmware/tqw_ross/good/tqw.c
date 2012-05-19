#include "tqw.h"

#if (RAMEND != 0x45F)
Shit shit shit!
#endif

volatile uint8_t g_timer0StartValue = 0x80;
volatile TQWRotationInterruptFunc g_rotationInterruptHandler = NULL;
volatile TQWColumnInterruptFunc g_columnInterruptHandler = NULL;

#define sei()  __asm__ __volatile__ ("sei" ::)
#define cli()  __asm__ __volatile__ ("cli" ::)

inline void enable_external_int (unsigned char ints)
{
#ifdef EIMSK
    outp (ints, EIMSK);
#else
#ifdef GIMSK
    outp (ints, GIMSK);
#endif
#endif
}

inline void timer_enable_int (unsigned char ints)
{
#ifdef TIMSK
    outp (ints, TIMSK);
#endif
}

static void stopTimer0(void)
{
    *((uint8_t *) TCCR0) = 0;
}

static void startTimer0(void)
{
    *((uint8_t *) TCCR0) = BV(CS00) + BV(CS01); /* CK/64 */
}

static void resetTimer0(void)
{
    *((uint8_t *) TCNT0) = g_timer0StartValue;
}

SIGNAL(SIG_OVERFLOW0)
{
    resetTimer0();
    if (NULL != g_columnInterruptHandler) {
        g_columnInterruptHandler();
    }
}

/* The rotation sensor tripped. Wake up the column timer in case it 
 * was asleep, and call the application's interrupt function if it 
 * exists.
 */
SIGNAL(SIG_INTERRUPT0)
{
    startTimer0();
    if (NULL != g_rotationInterruptHandler) {
        g_rotationInterruptHandler();
    }
}

/* This counter never stops.
 */
static void startCounter1(void)
{
    *((uint8_t *) TCCR1B) = BV(CS10) + BV(CS11); /* CK/64 */
}

static void enableTimer0Overflow(void)
{
    timer_enable_int(BV(TOIE0));
}

static void enableExternalInterrupts(void)
{
    /* Falling edge of INT0 triggers interrupt. */
    *((uint8_t *) MCUCR) = BV(ISC01);
    enable_external_int(BV(INT0));
}

void TQWDelay(uint8_t c)
{
    uint8_t i, j, k;
    for (i = 0; i < c; i++) {
        for (j = 0; j < 32; j++) {
            for (k = 0; k < 32; k++) {
                volatile uint8_t x = 0;
                x++;
            }
        }
    }
}

void TQWSetRotationInterrupt(TQWRotationInterruptFunc f)
{
    g_rotationInterruptHandler = f;
}

uint16_t TQWGetAndResetRotationInterruptTime(void)
{
    uint16_t elapsed = *((uint16_t *) TCNT1L);
    *((uint16_t *) TCNT1L) = 0;    
    return elapsed;
}

void TQWSetColumnInterrupt(TQWColumnInterruptFunc f)
{
    g_columnInterruptHandler = f;
}

void TQWSetColumnInterruptTimer(const uint8_t t)
{
    g_timer0StartValue = t;
}

void TQWSleepColumnInterruptTimer(void)
{
    stopTimer0();
}

#define COLUMN_PORT_LOW PORTC
#define COLUMN_PORT_HIGH PORTD

void TQWLoadLatch(const uint8_t c)
{
    *((uint8_t *) COLUMN_PORT_LOW) = c;
    if (c & BV(6)) {
        sbi(COLUMN_PORT_HIGH, 6);
    } else {
        cbi(COLUMN_PORT_HIGH, 6);
    }
    if (c & BV(7)) {
        sbi(COLUMN_PORT_HIGH, 7);
    } else {
        cbi(COLUMN_PORT_HIGH, 7);
    }
}

#define RED_LATCH_BIT (0)
#define GREEN_LATCH_BIT (4)
#define BLUE_LATCH_BIT (5)

void TQWStrobeRed(void)
{
    sbi(PORTB, RED_LATCH_BIT); cbi(PORTB, RED_LATCH_BIT);
}

void TQWStrobeGreen(void)
{
    sbi(PORTD, GREEN_LATCH_BIT); cbi(PORTD, GREEN_LATCH_BIT);
}

void TQWStrobeBlue(void)
{
    sbi(PORTD, BLUE_LATCH_BIT); cbi(PORTD, BLUE_LATCH_BIT);
}

void TQWExtinguishLights(void)
{
    TQWLoadLatch(0);
    TQWStrobeRed();
    TQWStrobeGreen();
    TQWStrobeBlue();
}

static void lightPixels(uint8_t color, uint8_t pixels)
{
    if (color & 1) {
        TQWLoadLatch(pixels);
	} else {
	    TQWLoadLatch(0);
	}
    TQWStrobeRed();
    if (color & 2) {
        TQWLoadLatch(pixels);
	} else {
	    TQWLoadLatch(0);
	}
    TQWStrobeGreen();
    if (color & 4) {
        TQWLoadLatch(pixels);
	} else {
	    TQWLoadLatch(0);
	}
    TQWStrobeBlue();
}

#define STARTUP_SEQUENCE_DELAY (8)
static void doStartupSequence(void)
{
    uint8_t c;
    for (c = 1; c < 8; c++) {
        uint8_t n = 2;
        for (n = 0; n < 2; n++) {
	        int8_t q;
	        for (q = 0; q < 8; q++) {
	            lightPixels(c, (uint8_t)BV(q));
	            TQWDelay(STARTUP_SEQUENCE_DELAY);
	        }
	        for (; q >= 0; q--) {
	            lightPixels(c, (uint8_t)BV(q));
	            TQWDelay(STARTUP_SEQUENCE_DELAY);
	        }
	    }
    }
    for (c = 0; c < 8; c++) {
        lightPixels(c, (uint8_t)0xff);
        TQWDelay(255);
    }
    for (c = 0; c < 8; c++) {
		TQWExtinguishLights();
		TQWDelay(255);
		lightPixels(7, (uint8_t)0xff);
		TQWDelay(255);
    }
    TQWExtinguishLights();
}

/* Enable eight output pins and three latch pins.
 */
static void enablePins(void)
{
    *((uint8_t *) DDRB) = BV(PB0);
    *((uint8_t *) DDRC) =
        BV(PC0) + BV(PC1) + BV(PC2) + BV(PC3) + BV(PC4) + BV(PC5);
    *((uint8_t *) DDRD) = BV(PD4) + BV(PD5) + BV(PD6) + BV(PD7);
}

void TQWInit(void)
{
    enablePins();
    doStartupSequence();
    enableTimer0Overflow();
    enableExternalInterrupts();
    startCounter1();
    sei();
}

