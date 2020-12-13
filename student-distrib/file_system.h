#ifndef _FILE_SYSTEM_H
#define _FILE_SYSTEM_H

#include "lib.h"
#include "types.h"
#include "paging.h"
#include "system_call.h"

#define MAX_FILE_DENTRIES    63
#define MAX_FILENAME_LENGTH  32
#define FS_BLOCK_SIZE        4096 //4kB
#define FS_STATS_SIZE        64
#define RESERVED_NUM1        24
#define RESERVED_NUM2        52
#define DATA_BLK             1023

/*
 * File system directory entry
 */
typedef struct {
	int8_t   filename[MAX_FILENAME_LENGTH];
	uint32_t filetype;
	uint32_t inode;
	uint8_t  reserved[RESERVED_NUM1];
} dentry_t;

/* 
 * Boot block format.
 */
typedef struct {
	uint32_t num_dentries;
	uint32_t num_inodes;
	uint32_t num_datablocks;
	uint8_t  reserved[RESERVED_NUM2];
} bootblock_t;

/* 
 * Inode block format.
 */
typedef struct{
	uint32_t size;
	uint32_t data_blocks[DATA_BLK];
} inodeblock_t;

/* Dentry read by filesystem */
dentry_t* fs_dentry;
/* inode block */
inodeblock_t* inodeblk;
/* Starting Address of the first block*/
uint32_t first_datablk;
/* Number of files readed by fs */
uint32_t num_file_read;
/* Starting address of Boot Block */
bootblock_t bootblk;
/* Flag */
uint32_t flag_file_read;

int32_t fs_init (uint32_t fs_start_addr, uint32_t fs_end_addr);

int32_t read_dentry_by_name (const int8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length);

int32_t file_open (const uint8_t* file_name);
int32_t file_read (int32_t fd, void* buf, int32_t nbytes);
int32_t file_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t file_close (int32_t fd);

int32_t dir_open (const uint8_t* file_name);
int32_t dir_read (int32_t fd, void* buf, int32_t nbytes);
int32_t dir_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t dir_close (int32_t fd);

#endif /* _FILE_SYSTEM_H */
