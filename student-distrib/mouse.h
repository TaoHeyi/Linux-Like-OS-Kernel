/* mouse.h - defines for mouse device
 * vim:ts=4 noexpandtab
 */

#ifndef _MOUSE_H
#define _MOUSE_H

#include "types.h"
#include "lib.h"
#include "i8259.h"
#include "terminal.h"
#include "system_call.h"

/* interrupt request vector number for mouse (We are using the USB port) */
#define MOUSE_IRQ 12
/* port value for mouse */
#define MOUSE_DATA 0x60

/* Initialization function for mouse */
void mouse_init();
/* Handler for the mouse interrupt */
void mouse_interrupt_handler();

#endif
