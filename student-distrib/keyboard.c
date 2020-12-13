/* keyboard.c - functions for keyboard device
 * vim:ts=4 noexpandtab
 */

#include "keyboard.h"


/* 4 states total, bit 0 determines if shift is pressed, bit 1 determines if capslock is pressed */
static uint8_t scancode_state;
/* ctrl and alt pressed or not state, 1 for pressed, 0 for released; enter state for terminal_read to check */
static uint8_t ctrl_state=0;
static uint8_t alt_state=0;

/* scancode map to character, 4 states total, and 59 keys in total(from 0x00 to 0x3A). 
 * special key marked as \0 (null key) including the 0x00, which is undefined 
 */
static uint8_t scancode_map[4][59]={
    // capslock=0 and shift=0
	{'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0',
	 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\0', '\0', 'a', 's',
	 'd', 'f', 'g', 'h', 'j', 'k', 'l' , ';', '\'', '`', '\0', '\\', 'z', 'x', 'c', 'v', 
	 'b', 'n', 'm',',', '.', '/', '\0', '*', '\0', ' ', '\0'},
	// capslock=0 and shift=1
	{'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0', '\0',
	 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\0', '\0', 'A', 'S',
	 'D', 'F', 'G', 'H', 'J', 'K', 'L' , ':', '"', '~', '\0', '|', 'Z', 'X', 'C', 'V', 
	 'B', 'N', 'M', '<', '>', '?', '\0', '*', '\0', ' ', '\0'},
	// capslock=1 and shift=0
	{'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0',
	 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\0', '\0', 'A', 'S',
	 'D', 'F', 'G', 'H', 'J', 'K', 'L' , ';', '\'', '`', '\0', '\\', 'Z', 'X', 'C', 'V', 
	 'B', 'N', 'M', ',', '.', '/', '\0', '*', '\0', ' ', '\0'},
	// capslock=1 and shift=1
	{'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0', '\0',
	 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\0', '\0', 'a', 's',
	 'd', 'f', 'g', 'h', 'j', 'k', 'l' , ':', '"', '~', '\0', '\\', 'z', 'x', 'c', 'v', 
	 'b', 'n', 'm', '<', '>', '?', '\0', '*', '\0', ' ', '\0'}
};

/* key buffer and buffer index used to display in terminal */
volatile uint8_t key_buf[KEY_BUF_MAX];
volatile uint8_t key_buf_idx=0;

/* void keyboard_init(void)
 * Input:  none
 * Return Value: none
 * Function: initialize keyboard on PIC */
void keyboard_init(){
    enable_irq(KEYBOARD_IRQ);
}

/* void keyboard_interrupt_handler(void)
 * Input:  none
 * Return Value: none
 * Function: called when keyboard interrupt occurs, echo key pressed (if possible) to screen */
void keyboard_interrupt_handler(){
    cli();
    uint8_t scancode_idx;
    uint8_t key;
    keyboard_enabled = 1;
    ctrl_c_flag = 0;
    //if scancode available, load it
    scancode_idx = inb(KEYBOARD_DATA);
    //check for different key cases
    switch(scancode_idx){
        //handle special characters
        case CAPSLOCK_P:
            scancode_state ^= 0x02;         //xor bit 1
            break;
        case LSHIFT_P:
            scancode_state |= 0x01;         //change bit 0 to 1
            break;
        case LSHIFT_R:
            scancode_state &= 0xFE;         //change bit 0 to 0
            break;
        case RSHIFT_P:
            scancode_state |= 0x01;         //change bit 0 to 1
            break;
        case RSHIFT_R:
            scancode_state &= 0xFE;         //change bit 0 to 0
            break;
        case CTRL_P:
            ctrl_state = 1;
            break;
        case CTRL_R:
            ctrl_state = 0;
            break;
        case ALT_P:
            alt_state = 1;
            break;
        case ALT_R:
            alt_state = 0;
            break;
        case ENTER:
            if(key_buf_idx<KEY_BUF_MAX){
                key_buf[key_buf_idx]='\n';
                key_buf_idx++;
                term[cur_term_id].enter_state = 1; 
                enter();
            }
            break;
        case BACKSPACE:
            if(key_buf_idx>0){
                backspace();                    //move cursor
                key_buf_idx--;
                key_buf[key_buf_idx]='\0';
            }
            break;
        case F1:
            if(alt_state){
                term_switch(cur_term_id, 0);
            }
            break;
        case F2:
            if(alt_state){
                term_switch(cur_term_id, 1);
            }
            break;
        case F3:
            if(alt_state){
                term_switch(cur_term_id, 2);
            }
            break;
        //handle normal characters
        default:
            if(scancode_idx>=59)    break;      //check for out of bound
            key = scancode_map[scancode_state][scancode_idx];
            if(key=='\0')   break;              //check for special key
            if(alt_state==1) break;             //alt is not handled
            if(ctrl_state==1){                  //handle crtl
                if(key=='l'||key=='L'){
                    clear();                    //clear video memory
                    set_screen_cursor(0,0);     //move cursor to 0,0
                    break;
                }
                else if(key=='c'||key=='C'){    // ctrl+c exit out of current program
                    ctrl_c_flag = 1;            // Set ctrl_c_flag to 1 for halting
                    buf_clear();
                    break;
                }
                else break;  
            }
            else{    
                if(key_buf_idx<(KEY_BUF_MAX-1)){ // check for out of bound
                    key_buf[key_buf_idx]=key;
                    key_buf_idx++;
                    putc(key);                   //echo it to screen, function from "lib.h"
                }
                break;
            }
            break;
    }
    keyboard_enabled = 0;
    send_eoi(KEYBOARD_IRQ);                     //send eoi
    if(ctrl_c_flag) halt(1);                    //ctrl_c still has problem
}

/* void buf_clear(void)
 * Input:  none
 * Return Value: none
 * Function: clear the key buffer */
void buf_clear(){
    uint8_t i;
    for(i=0;i<KEY_BUF_MAX;i++){
        key_buf[i]='\0';                         //put NULL character at each location of key buffer
    }
    key_buf_idx=0;
}

