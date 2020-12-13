/* paging.h - Defines used for paging
 * vim:ts=4 noexpandtab
 */

#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"
#include "lib.h"

#define dir_size            1024
#define tab_size            1024
#define page_align_bytes    4096

/* Page Directory and Page table when we initialized the paging */
uint32_t page_dir[dir_size] __attribute__((aligned (page_align_bytes)));
uint32_t page_tab[tab_size] __attribute__((aligned (page_align_bytes)));
uint32_t page_video_tab[tab_size] __attribute__((aligned (page_align_bytes)));
uint32_t page_schedule_tab[tab_size] __attribute__((aligned (page_align_bytes)));

/* New function to initialize paging */
void paging_init (void);

/* Helper function to flush the TLB */
void flush (void);

/* Map from virtual address to physical address in page directory for our PCB */
void pcb_mapping (uint32_t virtual_address, uint32_t physical_address);

/* New function used to map virtual address for video memory to physical address */
void syscall_video_mapping (uint32_t physical_address);

/* New function used to map virtual address for terminal video memory to physical address */
void terminal_video_mapping (uint32_t physical_address, uint32_t page_idx);

/* New function used to map  virtual addresss to scheduled process video memory */
void scheduling_video_mapping (uint32_t physical_address);

#endif
