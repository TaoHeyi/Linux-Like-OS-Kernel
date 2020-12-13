/* system_call.h - defines for system calls
 * vim:ts=4 noexpandtab
 */

#ifndef _SYSTEMCALL_H
#define _SYSTEMCALL_H

#include "types.h"
#include "x86_desc.h"
#include "lib.h"
#include "paging.h"
#include "file_system.h"
#include "terminal.h"
#include "keyboard.h"
#include "rtc.h"
#include "idt.h"

/* Macro definition section */
#define MAX_ARG_NUM 10
#define MAGIC_NUM 4
#define HEADER_NUM 40
#define MAX_FILE_NUM 8
#define FILE_NAME_SIZE 32
#define MAX_ARG_LENGTH 100
#define START_VITURAL_ADDR 0x08048000

#define RTC_TYPE 0
#define DIR_TYPE 1
#define FILE_TYPE 2

#define MAX_PCB_MASK_LEN 4

/* Struct definition section */

/* Struct file_optable_t
 * read: function pointer for read
 * write: function pointer for write
 * open: function pointer for open
 * close: function pointer for close */
typedef struct{
   int32_t (*read)(int32_t fd, void* buf, int32_t  nbytes);
   int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
   int32_t (*open)(const uint8_t* filename);
   int32_t (*close)(int32_t fd);
} file_optable_t;

/* Struct: file_desc_t
 * optable : a struct for file operations table
 * inode : inode number of this file in the file system
 * file_position : current position within the file we are reading
 * flags : used to figure out which fds are available for use when try to open a new file
 * file_name : name of current file
*/ 
typedef struct {
	file_optable_t optable;
	int32_t inode;
	int32_t file_position;
	int32_t flags;
} file_desc_t;

/* Struct: pcb_t
 * fds[MAX_FILE_NUM] : array of file descriptor 
 * filenames[MAX_FILE_NUM][FILE_NAME_SIZE] : an array which contains the name of open files
 * parent_ksp : the kernel stack ptr of the parent process
 * parent_kbp : the kernel base ptr of the parent process
 * ksp_before : the kernel stack ptr of the previous process in round robin
 * kbp_before : the kernel base ptr of the previous process in round robin
 * process_number : process number from 0 to 7
 * parent_process_number : parent process number from 0 to 7
 * argbuf[MAX_ARG_SIZE] : argument buffer in this process
 */ 
typedef struct {
	file_desc_t fds[MAX_FILE_NUM]; 
	uint32_t parent_ksp;
	uint32_t parent_kbp;
	uint32_t ksp;
	uint32_t kbp;
	int8_t process_number;
	int8_t parent_process_number;
	uint8_t argbuf[MAX_ARG_LENGTH];
	tss_t cur_tss;
	uint8_t term_id;
	uint32_t rtc_counter;
	uint32_t rtc_freq;
} pcb_t;

/* System Calls section */

/* system call: halt */
int32_t halt (uint8_t status);
/* system call: execute */
int32_t execute (const uint8_t* command);
/* system call: read */
int32_t read (int32_t fd, void* buf, int32_t  nbytes);
/* system call: write */
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
/* system call: open */
int32_t open (const uint8_t* filename);
/* system call: close */
int32_t close (int32_t fd);
/* system call: getargs */
int32_t getargs (uint8_t* buf, int32_t nbytes);
/* system call: vidmap */
int32_t vidmap (uint8_t** screen_start);
/* system call: set_handler */
int32_t set_handler (int32_t signum, void* handler_address);
/* system call: sigreturn */
int32_t sigreturn (void);


/************** Helper Functions Are In This Section **************/

/* Get the process Number that is free in PCB_mask */
int32_t PCB_process_number();
/* Get the total number of active tasks in PCB */
int32_t PCB_total_number();
/* Get current pcb pointer */
pcb_t* get_cur_pcb();
/* Get specific pcb pointer */
pcb_t* get_pcb_from_id(uint8_t id);
/* error operation to map into file operation table */
int32_t operation_error();

#endif
