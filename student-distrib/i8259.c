/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask=0xFF; /* IRQs 0-7  */
uint8_t slave_mask=0xFF;  /* IRQs 8-15 */

/* void i8259_init(void)
 * Input:  none
 * Return Value: none
 * Function: Initialize the I8259 PIC
 */
void i8259_init(void)
{
    outb(ICW1,MASTER_8259_CMD);     //select master pic
    outb(ICW2_MASTER,MASTER_8259_DATA);
    outb(ICW3_MASTER,MASTER_8259_DATA);
    outb(ICW4,MASTER_8259_DATA);
    outb(ICW1,SLAVE_8259_CMD);      //select slave pic
    outb(ICW2_SLAVE,SLAVE_8259_DATA);
    outb(ICW3_SLAVE,SLAVE_8259_DATA);
    outb(ICW4,SLAVE_8259_DATA);

    outb(master_mask,MASTER_8259_DATA);     //restore master IRQ mask
    outb(slave_mask,SLAVE_8259_DATA);       //restore slave IRQ mask

    enable_irq(2);      //slave is at IRQ2
}
/* void enable_irq(uint32_t irq_num)
 * Input:  irq line number
 * Return Value: none
 * Function: Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num)
{
    uint8_t cur_enabled=0x01;       //initialize the bit we are comparing
    if(irq_num>15||irq_num<0)   return;
    if(irq_num<8){
        cur_enabled = cur_enabled << irq_num;   //find the bit we want to operate
        master_mask &= ~cur_enabled;            //set that bit to 0(enabled) and "and" to master_mask
        outb(master_mask,MASTER_8259_DATA);
    }
    else{
        cur_enabled = cur_enabled << (irq_num-8);       //8 is total num of IRQ port on master
        slave_mask &= ~cur_enabled;                     //set that bit to 0(enabled) and "and" to slave_mask
        outb(slave_mask,SLAVE_8259_DATA);
    }
}

/* void disable_irq(uint32_t irq_num)
 * Input:  irq line number
 * Return Value: none
 * Function: Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num)
{
    uint8_t cur_disabled=0x01;       //initialize the bit we are comparing
    if(irq_num>15||irq_num<0)   return;
    if(irq_num<8){
        cur_disabled = cur_disabled << irq_num;   //find the bit we want to operate
        master_mask |= ~cur_disabled;            //set that bit to 1(disabled) and "or" to master_mask
        outb(master_mask,MASTER_8259_DATA);
    }
    else{
        cur_disabled = cur_disabled << (irq_num-8);       //8 is total num of IRQ port on master
        slave_mask |= ~cur_disabled;                     //set that bit to 1(disabled) and "or" to slave_mask
        outb(slave_mask,SLAVE_8259_DATA);
    }
}

/* void send_eoi(uint32_t irq_num)
 * Input:  irq line number
 * Return Value: none
 * Function: Send end-of-interrupt signal for the specified IR */
void send_eoi(uint32_t irq_num)
{
    if(irq_num>15||irq_num<0) return;
    if(irq_num<8){
        outb(EOI | irq_num , MASTER_8259_CMD);          //eoi "or" with irq_num, and send to master command port
    }
    else{
        outb(EOI | (irq_num-8) , SLAVE_8259_CMD);       //eoi "or" with irq_num, and send to slave command port
        send_eoi(2);        //slave port is IRQ 2, and re send eoi for master
    }
}
