/* scheduling.h - Defines used for scheduling
 * vim:ts=4 noexpandtab
 */

#ifndef _SCHEDULING_H
#define _SCHEDULING_H

#include "paging.h"
#include "terminal.h"
#include "lib.h"
#include "types.h"
#include "i8259.h"
#include "x86_desc.h"
#include "system_call.h"

/******* Define Terms *******/ 
#define PIT_freq            11931   /* Set the PIT frequency to 100HZ(Get frequency by using PIT_freq = 1193180/HZ_WE_WANT)*/
#define PIT_IRQ             0
#define PIT_Channel_Zero    0x40
#define PIT_Mode_Reg        0x43
#define PIT_Mode_Three      0x36
#define Hight_Eight_bits    8
#define Lower_Eight_Mask    0xFF

/******* Global Variable *******/

/* The process has been lastly processed */
int32_t now_term_id;
int32_t next_term_id;
int32_t prev_term_id;

/* Initialize Programmable Interrupt Time (PIT) */
void pit_init(void);

/* PIT handlers here */
void pit_interrupt_handler(void);

#endif
