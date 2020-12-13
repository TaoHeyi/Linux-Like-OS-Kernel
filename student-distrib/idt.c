/* idt.c - Interrupt Descriptor Table
 * vim:ts=4 noexpandtab
 */

#include "idt.h"


/* void def_interrupt();
 * Inputs: none
 * Return Value: none
 * Function: If any defined interrupt is raised, print interrupt message */
#define idt_table(def_interrupt, interrupt_msg) \
void def_interrupt() { \
    cli(); \
    /* Set interrupt flag to 1 for halt return 256 */ \
    interrupt_halt_flag = 1; \
    printf("%s\n", interrupt_msg); \
    /* Call halt(0) to squash interrupt generated end-of-program */ \
    halt(0); \
	while(1); \
}

idt_table (DE, "Divide Error Exception");
idt_table (DB, "Debug Exception");
idt_table (NMI, "NMI Interrupt");
idt_table (BP, "Breakpoint Exception");
idt_table (OF, "Overflow Exception");
idt_table (BR, "BOUND Range Exceeded Exception");
idt_table (UD, "Invalid Opcode Exception");
idt_table (NM, "Device Not Available Exception");
idt_table (DF, "Double Fault Exception");
idt_table (CSO, "Coprocessor Segment Overrun");
idt_table (TS, "Invalid TSS Exception");
idt_table (NP, "Segment Not Present");
idt_table (SS, "Stack Fault Exception");
idt_table (GP, "General Protection Exception");
//idt_table (PF, "Page-Fault Exception");
idt_table (MF, "x87 FPU Floating-Point Error");
idt_table (AC, "Alignment Check Exception");
idt_table (MC, "Machine-Check Exception");
idt_table (XF, "SIMD Floating-Point Exception");

/* void idt_init();
 * Inputs: none
 * Return Value: none
 * Function: Initialize the IDT */
void idt_init() {

    int index;

    for(index = 0; index < NUM_VEC; index++)
    {
        idt[index].seg_selector = KERNEL_CS; /* Set SS to kernel */
        idt[index].reserved4 = 0x0; /* Always 0 */
        idt[index].reserved3 = 0x0; /* 0 for interrupts 0 - 31 */
        idt[index].reserved2 = 0x1; /* Always 1 */
        idt[index].reserved1 = 0x1; /* Always 1 */
        idt[index].size = 0x1; /* Size of gate is 32-bits */
        idt[index].reserved0 = 0x0; /* Always 0 */
        idt[index].dpl = 0x0; /* Priviledge level is 0 by default */
        idt[index].present = 0x1; /* All entries in IDT should be present after initialization */

        /* For indexes greater than 31, set reserved 3 to 0 since these are user defined interrupts */
        if(index > 31)
        {
        	SET_IDT_ENTRY(idt[index], undef_interrupt);
        }
    }
    
    /* Also set dpl for system call to 0x3 (user level) */
    idt[SYS_VEC].dpl = 0x3;

    SET_IDT_ENTRY(idt[0] , DE);  /* Interrupt 0:  Divide Error Exception */
    SET_IDT_ENTRY(idt[1] , DB);  /* Interrupt 1:  Debug Exception */
    SET_IDT_ENTRY(idt[2] , NMI); /* Interrupt 2:  NMI Interrupt */
    SET_IDT_ENTRY(idt[3] , BP);  /* Interrupt 3:  Breakpoint Exception */
    SET_IDT_ENTRY(idt[4] , OF);  /* Interrupt 4:  Overflow Exception */
    SET_IDT_ENTRY(idt[5] , BR);  /* Interrupt 5:  BOUND Range Exceeded Exception */
    SET_IDT_ENTRY(idt[6] , UD);  /* Interrupt 6:  Invalid Opcode Exception */
    SET_IDT_ENTRY(idt[7] , NM);  /* Interrupt 7:  Device Not Available Exception */
    SET_IDT_ENTRY(idt[8] , DF);  /* Interrupt 8:  Double Fault Exception */
    SET_IDT_ENTRY(idt[9] , CSO); /* Interrupt 9:  Coprocessor Segment Overrun */
    SET_IDT_ENTRY(idt[10], TS);  /* Interrupt 10: Invalid TSS Exception */
    SET_IDT_ENTRY(idt[11], NP);  /* Interrupt 11: Segment Not Present */
    SET_IDT_ENTRY(idt[12], SS);  /* Interrupt 12: Stack Fault Exception */
    SET_IDT_ENTRY(idt[13], GP);  /* Interrupt 13: General Protection Exception */
    SET_IDT_ENTRY(idt[14], PF);  /* Interrupt 14: Page-Fault Exception */
                                 /* Interrupt 15: Reserved by Intel */
    SET_IDT_ENTRY(idt[16], MF);  /* Interrupt 16: x87 FPU Floating-Point Error */
    SET_IDT_ENTRY(idt[17], AC);  /* Interrupt 17: Alignment Check Exception */
    SET_IDT_ENTRY(idt[18], MC);  /* Interrupt 18: Machine-Check Exception */
    SET_IDT_ENTRY(idt[19], XF);  /* Interrupt 19: SIMD Floating-Point Exception */

    SET_IDT_ENTRY(idt[KBD_VEC], kbd_handler);
    SET_IDT_ENTRY(idt[RTC_VEC], rtc_handler);
    SET_IDT_ENTRY(idt[SYS_VEC], syc_handler);
    SET_IDT_ENTRY(idt[PIT_VEC], pit_handler);
    SET_IDT_ENTRY(idt[MSE_VEC], mse_handler);
}

/* void undef_interrupt();
 * Inputs: none
 * Return Value: none
 * Function: If an interrupt index is undefined, print undefined message*/
void undef_interrupt() {
	printf("Undefined Interrupt");
}

void pf_handler(int32_t cr,int32_t error){
    cli();
    /* Set interrupt flag to 1 for halt return 256 */
    interrupt_halt_flag = 1;
    printf("page-fault\n");
    printf("%x\n",cr);
    printf("%x\n",error);
    /* Call halt(0) to squash interrupt generated end-of-program */
    halt(0);
    while(1);
    //sti();
}
