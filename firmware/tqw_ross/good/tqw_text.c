/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <tqw.h>
#include <font6x8.h>

/* A screen is made up of one or more frames. */
#define SCREEN_WIDTH (128)
#define SCREEN_WIDTH_EXP (7)
#define FRAME_WIDTH (128)
#define FRAMES_IN_SCREEN (1)

volatile uint8_t g_rbuf[FRAME_WIDTH];
volatile uint8_t g_gbuf[FRAME_WIDTH];
volatile uint8_t g_bbuf[FRAME_WIDTH];

volatile uint8_t g_column;
volatile uint8_t g_frames;
volatile uint8_t g_buffer_offset;
volatile uint8_t g_buffer_remaining;
volatile uint16_t g_program_counter = 0;

uint8_t hexDigits[] = {
	'0', '1', '2', '3',
	'4', '5', '6', '7',
	'8', '9', 'a', 'b',
	'c', 'd', 'e', 'f'
};

static void adjustAndResetColumnTimer(void)
{
    uint16_t elapsed;
    elapsed = TQWGetAndResetRotationInterruptTime();
    
    /* Adjust the column interval only if this looks like a reasonable
     * cycle (roughly, mcu's CK/Hz / CK/64 / 30fps). Otherwise there
     * might be some ruined frames because of rotation sensor bouncing.
     */
    if (elapsed > (MCU_MEGAHERTZ * 1500) ) {
        uint8_t elapsed8;
        elapsed = elapsed >> SCREEN_WIDTH_EXP;
        elapsed8 = (uint8_t)elapsed;
        TQWSetColumnInterruptTimer(255 - elapsed8);
    }
}

static void handleRotationInterrupt(void)
{
    g_column = g_buffer_offset;
    g_buffer_remaining = FRAME_WIDTH;
    g_frames = 0;
    adjustAndResetColumnTimer();
}

static void setColumnLights(void)
{
    TQWLoadLatch(g_rbuf[g_column]);
    TQWStrobeRed();
    TQWLoadLatch(g_gbuf[g_column]);
    TQWStrobeGreen();
    TQWLoadLatch(g_bbuf[g_column]);
    TQWStrobeBlue();
}

/* Move to next column in frame, overflowing to next frame if necessary.
 * If we've finished the last frame, put the timer to sleep.
 */
static void advanceColumnMarker(void)
{
    if (++g_column >= FRAME_WIDTH) {
        g_column = 0;
    }
    if (0 == --g_buffer_remaining) {
        ++g_frames;
        g_buffer_remaining = FRAME_WIDTH;
    }
}

static void handleColumnInterrupt(void)
{
    /* The check for frame overflow is here instead of advanceColumnMarker
     * because we want to extinguish the lights _after_ the final column has
     * had a full phase. If we shut down the timer and the lights right after
     * setting the final column, then we'd never see the final column.
     */
    if (g_frames < FRAMES_IN_SCREEN) {
        setColumnLights();
        advanceColumnMarker();
    } else {
        TQWExtinguishLights();
        TQWSleepColumnInterruptTimer();
    }
}

void emit_character(char c, uint8_t col_num, uint8_t color_mask)
{
    uint8_t bitmap[FONT_WIDTH];
    uint8_t i;

    if (c >= 33) {
        PGM_P sfp = smallFont + (c - 33) * FONT_WIDTH;
        memcpy_P(bitmap, sfp, FONT_WIDTH);
    } else {
        bitmap[0] = 
            bitmap[1] = 
            bitmap[2] = 
            bitmap[3] = 
            bitmap[4] = 
            bitmap[5] = 0;
    }

    for (i = 0; i < FONT_WIDTH; i++) {
        uint8_t column = bitmap[i];
        if ((1 & color_mask)) {
            g_rbuf[col_num + i] = column;
        } else {
            g_rbuf[col_num + i] = 0;
        }
        if ((2 & color_mask)) {
            g_gbuf[col_num + i] = column;
        } else {
            g_gbuf[col_num + i] = 0;
        }
        if ((4 & color_mask)) {
            g_bbuf[col_num + i] = column;
        } else {
            g_bbuf[col_num + i] = 0;
        }
    }
}

static void emit_string(uint8_t *s, uint8_t offset)
{
    uint8_t color_mask = 1;
    uint8_t i = 0;
    while ('\0' != *s) {
        emit_character(*s, i + offset, color_mask);
        i += FONT_WIDTH;
        color_mask++;
        if (color_mask > 7) {
            color_mask = 1;
        }
    	s++;
    }
}

static void dumpHex(uint8_t *s, uint8_t n)
{
    uint8_t color_mask = 1;
    uint8_t i = 0;
    while (n-- > 0) {
        uint8_t temp_char;
        uint8_t binary_digit = *s;
        memcpy(&temp_char, &hexDigits[binary_digit >> 4], 1);
        emit_character(temp_char, i, color_mask);
        i += FONT_WIDTH;
        memcpy(&temp_char, &hexDigits[binary_digit & 0x0f], 1);
        emit_character(temp_char, i, color_mask);
        i += FONT_WIDTH;
        color_mask++;
        if (color_mask > 7) {
            color_mask = 1;
        }
    	s++;
    }
}

static void shiftVirtualBufferStart(uint8_t forward, uint8_t rate)
{
    if (forward) {
		g_buffer_offset++;
		if (g_buffer_offset > FRAME_WIDTH) {
		    g_buffer_offset = 0;
		}
    } else {
		if (0 == g_buffer_offset) {
		    g_buffer_offset = FRAME_WIDTH - 1;
		} else {
            g_buffer_offset--;
        }
    }
}

static void fillFrameBuffer(void)
{
    /*           123456789012345678901 */
    emit_string("The Quoting, Whirled ", 0);
}

int main(void)
{
    TQWInit();
    TQWSetRotationInterrupt(handleRotationInterrupt);
    TQWSetColumnInterrupt(handleColumnInterrupt);

    fillFrameBuffer();
    
    do {
        shiftVirtualBufferStart(TRUE, 128);
        TQWDelay(32);
    } while (TRUE);
}
