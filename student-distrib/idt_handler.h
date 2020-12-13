/* idt_handler.h - Function definition for idt handler
 * vim:ts=4 noexpandtab
 */

#ifndef _IDT_HANDLER_H
#define _IDT_HANDLER_H

#include "x86_desc.h"

#ifndef ASM

extern void kbd_handler();
extern void rtc_handler();
extern void syc_handler();
extern void pit_handler();
extern void mse_handler();

extern void PF();
#endif

#endif
