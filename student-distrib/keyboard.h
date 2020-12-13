/* keyboard.h - defines for keyboard device
 * vim:ts=4 noexpandtab
 */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "types.h"
#include "lib.h"
#include "i8259.h"
#include "terminal.h"
#include "system_call.h"

/* interrupt request vector number for keyboard */
#define KEYBOARD_IRQ 1
/* port value for keyboard */
#define KEYBOARD_DATA 0x60
/* special key, P for pressed, R for released */
#define CAPSLOCK_P  0x3A
#define CPASLOCK_R  0xBA
#define LSHIFT_P    0x2A
#define LSHIFT_R    0xAA
#define RSHIFT_P    0x36
#define RSHIFT_R    0xB6
#define ALT_P       0x38
#define ALT_R       0xB8
#define CTRL_P      0x1D
#define CTRL_R      0x9D
#define BACKSPACE   0x0E
#define ENTER       0x1C
#define F1          0x3B
#define F2          0x3C
#define F3          0x3D
/* key buffer max size */
#define KEY_BUF_MAX 128

/* global variable */
extern volatile uint8_t key_buf[KEY_BUF_MAX];
extern volatile uint8_t key_buf_idx;
uint8_t keyboard_enabled;
int32_t ctrl_c_flag;

/* keyboard initialization */
void keyboard_init(void);
/* keyboard interrupt handler */
void keyboard_interrupt_handler(void);
/* clear key buffer */
void buf_clear(void);

#endif
