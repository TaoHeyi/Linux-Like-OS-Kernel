/* terminal.h - defines for terminals
 * vim:ts=4 noexpandtab
 */

#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"
#include "lib.h"
#include "keyboard.h"
#include "file_system.h"
#include "paging.h"
#include "system_call.h"
#include "scheduling.h"

#define KEY_BUF_MAX 128
#define TERM_MAX 3

typedef struct{
    uint8_t term_id;
    int8_t cur_pcb_id;
    uint32_t cursor_x;
    uint32_t cursor_y;
    int32_t rtc_virtual_freq;
    int32_t rtc_virtual_counter;
    int32_t rtc_interrupt_received;
    volatile uint8_t key_buf[KEY_BUF_MAX];
    volatile uint8_t key_buf_idx;
    volatile uint8_t enter_state;
    uint8_t running;
    uint8_t* video_mem;
}term_t;

/* global variable */
extern term_t term[TERM_MAX];
extern volatile uint8_t cur_term_id;

/* terminal operation */
void term_init();
int32_t term_launch(uint8_t id);
int32_t term_save(uint8_t id);
int32_t term_restore(uint8_t id);
int32_t term_switch(uint8_t old_id, uint8_t new_id);
int32_t term_bootup();
/* system call */
int32_t term_open(const uint8_t* file_name);
int32_t term_read(int32_t fd, void* buf, int32_t length);
int32_t term_write(int32_t fd, const void* buf, int32_t length);
int32_t term_close(int32_t fd);


#endif
