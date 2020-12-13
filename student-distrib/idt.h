/* idt.h - Function definition for idt
 * vim:ts=4 noexpandtab
 */

#ifndef _IDT_H
#define _IDT_H

#include "i8259.h"
#include "x86_desc.h"
#include "lib.h"
#include "idt_handler.h"
#include "system_call.h"

int32_t interrupt_halt_flag;

#define KBD_VEC 0x21
#define RTC_VEC 0x28
#define SYS_VEC 0x80
#define PIT_VEC 0x20
#define MSE_VEC 0x2C

void idt_init();
void undef_interrupt();

#endif
