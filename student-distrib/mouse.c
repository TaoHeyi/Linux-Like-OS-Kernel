/* mouse.c - functions for mouse device
 * vim:ts=4 noexpandtab
 */

#include "mouse.h"

/* void mouse_init(void)
 * Input:  none
 * Return Value: none
 * Function: initialize mouse on PIC */
void mouse_init(){
    enable_irq(MOUSE_IRQ);
}

/* void mouse_interrupt_handler(void)
 * Input:  none
 * Return Value: none
 * Function: called when mouse interrupt occurs, move cursor and set screen cursor upon press */
void mouse_interrupt_handler(){
    cli();
    printf("Mouse interrupt has occurred!");
    send_eoi(MOUSE_IRQ);
    sti();
}
