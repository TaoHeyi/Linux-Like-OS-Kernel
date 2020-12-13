/* rtc.h - defines for real time clock
 * vim:ts=4 noexpandtab
 */

#ifndef _RTC_H
#define _RTC_H

#include "types.h"
#include "i8259.h"
#include "lib.h"

/* interrupt request vector number for rtc */
#define RTC_IRQ 8
/* port value for rtc */
#define RTC_INDEX   0x70
#define RTC_DATA    0x71
/* status registers of rtc, set 0x80 bit to 1 to disable NMI */
#define RTC_REG_A   0x8A
#define RTC_REG_B   0x8B
#define RTC_REG_C   0x8C

/* Initialize RTC */
void rtc_init(void);
/* RTC interrupt handler */
void rtc_interrupt_handler(void);
/* set own frequency */
void rtc_set_freq(int freq);
/* set the RTC interrupts to default frequency */
int32_t rtc_open(const uint8_t* filename);
/* close the RTC */
int32_t rtc_close(int32_t fd);
/* Read occurs once an interrupt is generated */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
/* Write specified frequency to the RTC */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

#endif
