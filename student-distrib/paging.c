/* paging.c - Interaction and initialization of memory paging
 * vim:ts=4 noexpandtab
 */

#include "paging.h"

/* void paging_init(void);
 * Inputs: void
 * Return Value: none
 * Function: Initialize the page directory, page table, and enable paging */
void paging_init (void)
{   
    /* use memset to initialize page directory, number 2 enables read/write */
    memset(page_dir, 0x00000002, dir_size*sizeof(uint32_t));
    /* memset(page_tab, 0, tab_size*sizeof(uint32_t)); */

    unsigned int i;
    /* fill all 1024 entries in the table, mapping 4GB */
    for(i=0; i<1024; i++)
    {   
        /* only need to specify the the address starting at the thirteenth bit 
           or 3 to make the register in supervisor level, read/write, present */
        page_tab[i] = (i*0x1000) | 2;
    }

    /* put the newly created page table into our blank page directory, each page has size 4kB */
    page_dir[0] = ((unsigned int)page_tab) | 3;
    /* put the single 4MB page into the second entry of the page_dir */
    page_dir[1] = 0x400000 | 0x83;
    /* in lib.c, it states that video memory occupies 4kB starting at address 0xB8000 */
    page_tab[0xB8] = page_tab[0xB8] | 3;

    asm volatile(
                "pushl %%eax;"
                "movl %0, %%eax;"
                "movl %%eax, %%cr3;"        /* load the page directory address into CR3 register */
                "movl %%cr4, %%eax;"
                "orl  $0x00000010, %%eax;"  /* set the fourth bit to 1 to allow mixed page size */
                "movl %%eax, %%cr4;"
                "movl %%cr0, %%eax;"
                "orl  $0x80000001, %%eax;"  /* enable paging and protection mode in cr0 register */
                "movl %%eax, %%cr0;"
                :                           /* there is no output here */
                :"r"(page_dir)              /* input is page_dir here */
                :"%eax"                     /* clobbered register */
    );
}

/* void pcb_mapping (uint32_t virtual_address, uint32_t physical_address)
 * Inputs: virtual address, physical address
 * Return Value: none
 * Function: Map from virtual address to physical address in page directory for our PCB */
void pcb_mapping (uint32_t virtual_address, uint32_t physical_address)
{   
    /* Calculate the offset we should set as the index to the page directory, 0x400000 is 4MB */
    uint32_t offset = virtual_address/0x400000;

    /* Set the directory to point to the physcial address we want */
    /* Set user bit, resent bit, read/write bit, and size bit to 1 */
    page_dir[offset] = physical_address | 0x87;

    /* Flush the tlb */
    flush();
}

/* void syscall_video_mapping (uint32_t physical_address)
 * Inputs: physical address
 * Return Value: none
 * Function: Map from virtual address to physical address for our video memory */
void syscall_video_mapping (uint32_t physical_address)
{   
    /* 33 (132MB/4MB) is the offset we should set as the index to the page directory */
    uint32_t offset = 33;

    /* Make the page_dir points the page table points to our video memory */
    /* Or with 0x07 activates user level bit, read/write bit, and enable bit */
    page_dir[offset] = ((unsigned int)page_video_tab) | 0x7;

    /* Make the 0th entry of the page_video_table points to physical address of video memory */
    /* Or with 0x07 activates user level bit, read/write bit, and enable bit */
    page_video_tab[0] = physical_address|0x7;

    /* Flush the tlb */
    flush();
}

/* void terminal_video_mapping (uint32_t physical_address, uint32_t page_idx)
 * Inputs: physical address, page index
 * Return Value: none
 * Function: Map from virtual address to physical address for our terminal video memory */
void terminal_video_mapping (uint32_t physical_address, uint32_t page_idx)
{   
    /* Make the page_dir points the page table points to our video memory for each terminal */
    /* Or with 0x07 activates user level bit, read/write bit, and enable bit */
    page_dir[0] = ((unsigned int)page_tab) | 0x3;

    /* Make the specific entry of the page_table points to physical address of video memory of each terminal starting from 0xB9000 */
    /* Or with 0x03 deactivates user level bit, activates read/write bit, and enable bit */
    page_tab[0xB9+page_idx] = physical_address|0x3;

    /* Flush the tlb */
    flush();
}

/* void scheduling_video_mapping (uint32_t physical_address)
 * Inputs: physical address fordifferent terminal
 * Return Value: none
 * Function: Map from virtual address to physical address for our video memory */
void scheduling_video_mapping (uint32_t physical_address)
{   
    /* 33 (132MB/4MB) is the offset we should set as the index to the page directory */
    uint32_t offset = 33;

    /* Make the page_dir points the page table points to our video memory */
    /* Or with 0x07 activates user level bit, read/write bit, and enable bit */
    page_dir[offset] = ((unsigned int)page_video_tab) | 0x7;

    /* Make the 0th entry of the page_video_table points to physical address of video memory */
    /* Or with 0x07 activates user level bit, read/write bit, and enable bit */
    page_video_tab[0] = physical_address|0x7;

    /* Flush the tlb */
    flush();
}

/* void flush(void);
 * Inputs: void
 * Return Value: none
 * Function: flush the TLB register, but not changing its value */
void flush(void)
{
    asm volatile(
        "movl %%cr3, %%eax;"
        "movl %%eax, %%cr3;"    /* reload cr3 register*/
        :                       /* there is no output */
        :                       /* there is no input  */
        :"eax"                  /* eax register is clobbered regiser */
    );
}
