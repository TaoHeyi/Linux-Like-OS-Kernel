#include "file_system.h"
#include "lib.h"
#include "x86_desc.h"
#include "types.h"

uint32_t dentry_number = 0;
uint32_t inode_number = 0;
uint32_t block_number = 0;

/*
 * int32_t fs_init (uint32_t fs_start_addr, uint32_t fs_end_addr)
 * Inputs : fs_start_addr: start address of initialized file
 *          fs_end_addr: end address of initialized file
 * Return value : 0(PASS)/-1(FAIL)
 * Function: Initialize filesystem, Verifies that the filesystem 
 * between fs_start_addr and fs_end_addr is valid. */
int32_t fs_init (uint32_t fs_start_addr, uint32_t fs_end_addr)
{   
    /* Check for invalid address inputs */
    if(fs_start_addr == NULL || fs_end_addr == NULL) return -1;

    bootblock_t* check_num = (bootblock_t*) fs_start_addr;

    /* get the dentry number, inode_number, block_number here */
    dentry_number = * ((uint32_t *)fs_start_addr);
    inode_number = * ((uint32_t *)fs_start_addr + 1);
    block_number = * ((uint32_t *)fs_start_addr + 2);
    if(check_num -> num_dentries > MAX_FILE_DENTRIES) {
        return -1;
    }

    uint32_t bootblk_addr = fs_start_addr;

	memcpy(&bootblk, (void *)bootblk_addr, FS_STATS_SIZE);

	fs_dentry = (dentry_t *)(bootblk_addr + FS_STATS_SIZE );
	inodeblk = (inodeblock_t *)(bootblk_addr + FS_BLOCK_SIZE);
	first_datablk = bootblk_addr + (bootblk.num_inodes + 1)* FS_BLOCK_SIZE;

    /* Reset the number of files read and flag */
	num_file_read = 0;
    flag_file_read = 1;
    return 0;
}


/*
 * int32_t read_dentry_by_name (const int8_t* fname, dentry_t* dentry)
 * Inputs : fname: file name
 *          dentry: dentry to be written to 
 * Return value : 0(PASS)/-1(FAIL)
 * Function: Pass a dentry block to file with given name */
int32_t read_dentry_by_name (const int8_t* fname, dentry_t* dentry)
{

    int index, f1_length, f2_length;

    /* Check for invalid filename and dentry */
    if(fname == NULL || dentry == NULL) return -1;

    f1_length = strlen(fname);
    for(index = 0; index < MAX_FILE_DENTRIES; index++){ 

        /* Get length of the second file and truncate to 32 if exceeding maximum file length */
        f2_length = strlen(fs_dentry[index].filename);
        if(f2_length >= MAX_FILENAME_LENGTH) f2_length = MAX_FILENAME_LENGTH;

        /* Compare names of the two files */
        if(strncmp((fs_dentry[index].filename), (fname), f1_length) == 0 
        && strncmp((fs_dentry[index].filename), (fname), f2_length) == 0
        && fname[0] != '\0') 
        {
            strcpy(dentry->filename, fs_dentry[index].filename);
			dentry->filetype = fs_dentry[index].filetype;
			dentry->inode = fs_dentry[index].inode;
			return 0;
        }
    }

    /* fail to find the file */
    return -1;
}

/*
 * int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry)
 * Inputs : index: index of file inode
 *          dentry: dentry to be written to 
 * Return value : 0(PASS)/-1(FAIL)
 * Function: Pass a dentry block to file with given index */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry)
{   
    /* Check for invalid dentry */
    if(dentry == NULL) return -1;

    /* If the index is valid */
	if(index < MAX_FILE_DENTRIES)
    {   
        /* Copy over the content in dentry array to dentry passed in with given index */
        strcpy(dentry->filename, fs_dentry[index].filename);
		dentry->filetype = fs_dentry[index].filetype;
		dentry->inode = fs_dentry[index].inode;
        return 0;
	}
	return -1;
}

/*
 * int32_t read_data (uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length)
 * Input: inode : inode of file to be read from
 *         offset : starting byte 
 *         buf : the buffer data should be written to
 *         length : length of bytes to be read
 * Return value : length of data to be copied /-1(FAIL)
 * Function: Read data from inode */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length)
{
    /* Check for invalid buf input */
    if(buf == NULL) return -1;

    uint32_t cur_datablk = offset / FS_BLOCK_SIZE;
    uint32_t copied_bytes_done;
    uint32_t byte_in_blk = offset % FS_BLOCK_SIZE;
    uint8_t* cur_addr;

    cur_addr = (uint8_t *)(first_datablk 
                         + (inodeblk[inode].data_blocks[cur_datablk]) * FS_BLOCK_SIZE
                         + byte_in_blk);

    /* Copy data in unit of byte */
    for (copied_bytes_done = 0; copied_bytes_done < length; copied_bytes_done++)
    {
        if (byte_in_blk >= FS_BLOCK_SIZE)
        {
            cur_datablk++;
            byte_in_blk = 0;

            /* Check for overflow block index */
            if(inodeblk[inode].data_blocks[cur_datablk] >= bootblk.num_datablocks) return -1;

            cur_addr = (uint8_t *)(first_datablk 
                        + (inodeblk[inode].data_blocks[cur_datablk]) * FS_BLOCK_SIZE);
        }
        /* If current block is finished, return and move on to next block */
        if(copied_bytes_done + offset >= inodeblk[inode].size) return copied_bytes_done;

        //read byted
        buf[copied_bytes_done] = *cur_addr;	    
		byte_in_blk++;
		cur_addr++;
    }
    return copied_bytes_done;
}

/*
 * int32_t file_open (const uint8_t* file_name)
 * Inputs : file name
 * Return value : 0 (always PASS)
 * Function: open file */
int32_t file_open (const uint8_t* file_name)
{
    return 0;
}

/*
 * int32_t file_read (int32_t fd, void* buf, int32_t nbytes)
 * Inputs : fd: file descriptor
 *          buf : the buffer data to be written to
 *          nbytes : length of data to be written to
 * Return value : 0(PASS)/-1(FAIL)
 * Function: read file */
int32_t file_read (int32_t fd, void* buf, int32_t nbytes)
{
    pcb_t* curr_pcb = get_cur_pcb();
    int32_t fileposition_increment;
    uint32_t inode = curr_pcb->fds[fd].inode;
    uint32_t offset = curr_pcb->fds[fd].file_position;

    /* If fd and buf are valid */
    if ((fd != NULL) && ((int8_t*)buf != NULL) ) {
        fileposition_increment = read_data(inode, offset, (uint8_t*)buf, (uint32_t)nbytes);
        curr_pcb->fds[fd].file_position+=fileposition_increment;
        return fileposition_increment;
    }
    return -1;
}

/*
 * int32_t file_write (int32_t fd, const void* buf, int32_t nbytes)
 * Inputs : fd: file descriptor
 *          buf : the buffer data to be written to
 *          nbytes : length of data to be written to
 * Return value : -1 (always FAIL)
 * Function: write file */
int32_t file_write (int32_t fd, const void* buf, int32_t nbytes)
{
	return -1; 
}

/*
 * int32_t file_close (int32_t fd)
 * Inputs : fd :file descriptor
 * Return value : 0 (always PASS)
 * Function: close file */
int32_t file_close (int32_t fd)
{
	return 0;
}


/*
 * int32_t dir_open (const uint8_t* file_name)
 * Inputs : file name
 * Return value : 0 (always PASS)
 * Function: open a directory */
int32_t dir_open (const uint8_t* file_name)
{
	return 0;
}

/*
 * int32_t dir_read (int32_t fd, void* buf, int32_t nbytes)
 * Inputs : fd: file descriptor
 *          buf : the buffer data to be written to
 *          nbytes : length of data to be written to
 * Return value : 0(PASS)/-1(FAIL)
 * Function: open a directory */
int32_t dir_read (int32_t fd, void* buf, int32_t nbytes)
{
    uint32_t length = 0;

    /* Check for invalid buf */
    if(buf == NULL) return -1;

    /* If there are remaining files that haven't been read */
	if(num_file_read < bootblk.num_dentries) {
	    strncpy((int8_t*)buf, (int8_t*)fs_dentry[num_file_read].filename, MAX_FILENAME_LENGTH);
	    num_file_read++;
        ((int8_t*)buf)[MAX_FILENAME_LENGTH] = '\0';
        length = strlen((int8_t*)buf);
        return length;
	}

    /* Reset num_file_read to 0 */
    num_file_read = 0;
	return 0;
}

/*
 * int32_t dir_write (int32_t fd, const void* buf, int32_t nbytes)
 * Inputs : fd: file descriptor
 *          buf : the buffer data to be written to
 *          nbytes : length of data to be written to
 * Return value : -1 (always FAIL)
 * Function: write a directory */
int32_t dir_write (int32_t fd, const void* buf, int32_t nbytes)
{
	return -1;
}

/*
 * int32_t dir_close (int32_t fd)
 * Inputs : fd :file descriptor
 * Return value : 0 (always PASS)
 * Function: close a directory */
int32_t dir_close (int32_t fd)
{
	return 0;
}
