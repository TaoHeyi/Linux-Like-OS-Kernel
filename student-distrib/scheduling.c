/* scheduling.c - Interaction and initialization of scheduling
 * vim:ts=4 noexpandtab
 */

#include "scheduling.h"

/* void PIT_init(void)
 * Input:  none
 * Return Value: none
 * Function: Initialize PIT, and its status register. 
 * enable irq0 on PIC, turns on periodic interrupt and set default frequency to 100HZ. 
 * Reference: https://wiki.osdev.org/Programmable_Interval_Timer
 *            http://www.osdever.net/bkerndev/Docs/pit.htm */
void pit_init(void)
{   
    /* Write value 0x36 to Mode/Command register to generate square waveform 
     * and set access mode as lobyte/hibyte*/
    outb(PIT_Mode_Three, PIT_Mode_Reg);

    /* Write the lower eight bits of the divider to PIT Channel Zero */
    outb((PIT_freq&Lower_Eight_Mask), PIT_Channel_Zero);

    /* Write the remaining higher bits of the divider to PIT Channel Zero */
    outb((PIT_freq>>Hight_Eight_bits), PIT_Channel_Zero);

    /* Enable irq0 */
    enable_irq(PIT_IRQ);

    /* Initialize the first terminal to run process on */
    now_term_id = 0;
}

/* void PIT_handler(void)
 * Input:  none
 * Return Value: none
 * Function: Call the PIT_handler whenever receiving the PIT interrupts */
void pit_interrupt_handler(void)
{   
    int32_t process_number = -1;
    int32_t pcb_number;
    pcb_t* now_pcb;
    pcb_t* next_pcb;
    term_t next_term;

    /* Send end of interrupts for irq0, which is for PIT_irq */
    send_eoi(PIT_IRQ);
    cli();

    /* Find the next terminal we need to process, and the next PCB and process number */
    next_term_id = (now_term_id + 1) % TERM_MAX;
    process_number = term[next_term_id].cur_pcb_id;
    pcb_number = process_number + next_term_id * MAX_PCB_MASK_LEN;

    now_pcb = get_pcb_from_id(term[now_term_id].cur_pcb_id+now_term_id*MAX_PCB_MASK_LEN);
    
    prev_term_id = now_term_id;
    now_term_id = next_term_id;
    
    /* Update EBP and ESP */
    asm volatile(
        "movl %%ebp, %%eax;"
        "movl %%esp, %%ebx;"
        /* there is no input here */
        :"=a"(now_pcb->kbp), "=b"(now_pcb->ksp)
    );

    /* If the next terminal has no process, boot up */
    if(term[next_term_id].cur_pcb_id == -1) 
    {   
        term_launch(next_term_id); 
        return;
    }

    /* Process switch */

    /* Find the pcb for the current terminal and the pcb for the next terminal */
    next_pcb = get_pcb_from_id(term[next_term_id].cur_pcb_id+next_term_id*MAX_PCB_MASK_LEN);    

    /* Map parent's virtual address 0x8000000 (128MB) to physical address 0x800000 (8MB),
     * plus some number times 0x400000 (4MB), used to map the next program */
    pcb_mapping(0x8000000, 0x800000 + pcb_number * 0x400000);

    /* Check whether next term_id is not equal to the current term_id, if not we map 
     * the virtual address to the pre-saved address of that terminal */
    next_term = term[next_pcb->term_id];
    
    if(cur_term_id!=now_term_id){
        scheduling_video_mapping((uint32_t)next_term.video_mem);
        set_vidmem((char*)next_term.video_mem);
    }    
    /* Otherwise map to B8000 (Video memory of currently viewing) */
    else{
        scheduling_video_mapping(0xB8000);
        set_vidmem((char*)0xB8000);
    }

    /* Modify TSS */
    tss.ss0 = KERNEL_DS;
    /* 8MB - PCB_numbr * 8KB - 4 */
    tss.esp0 = 0x800000 - pcb_number * 0x2000 - 0x4;

    asm volatile(
        "movl %0, %%ebp;"
        "movl %1, %%esp;"
        :/* there is no output here */
        :"r"(next_pcb->kbp),"r"(next_pcb->ksp)
    );
    return;
}
