/* terminal.c - functions for terminal
 * vim:ts=4 noexpandtab
 */

#include "terminal.h"

/* static var */
term_t term[TERM_MAX];
volatile uint8_t cur_term_id;
file_optable_t stdin_fop_ = {term_read,operation_error,term_open,term_close};
file_optable_t stdout_fop_ = {operation_error,term_write,term_open,term_close};
file_optable_t error_fop_ = {operation_error,operation_error,operation_error,operation_error};

/* void term_init(void)
 * Input:  none
 * Return Value: none
 * Function: Initialize the terminal */
void term_init()
{
    int i,j;

    /* initialize each terminal's information */
    for(i=0;i<TERM_MAX;i++){

        term[i].term_id=i;
        term[i].cur_pcb_id= -1;
        term[i].cursor_x=0;
        term[i].cursor_y=0;

        for(j=0;j<(int)KEY_BUF_MAX;j++){

            term[i].key_buf[j]='\0';   //assign to null char

        }

        term[i].key_buf_idx=0;
        term[i].enter_state=0;
        term[i].running=0;
        term[i].rtc_virtual_freq = 2;
        term[i].rtc_virtual_counter = 0;
        term[i].rtc_interrupt_received = 0;

        /* allocate space in physical memory starting at 0xB9000 for each terminal's video memory and put them in video page table at virtual address 0xB9000 */
        terminal_video_mapping(0xb9000+0x1000*i,i);
        term[i].video_mem=(uint8_t*)(0xb9000+0x1000*i);

        for(j=0;j<NUM_ROWS*NUM_COLS;j++){
            *(uint8_t *)(term[i].video_mem + (j << 1)) = ' ';
            if(i==0)    *(uint8_t *)(term[i].video_mem + (j << 1) + 1) = ATTRIB_T1;
            if(i==1)    *(uint8_t *)(term[i].video_mem + (j << 1) + 1) = ATTRIB_T2;
            if(i==2)    *(uint8_t *)(term[i].video_mem + (j << 1) + 1) = ATTRIB_T3;
        }
    }

    /* set cur_term_id to 3 indicating no terminal is running currently */
    cur_term_id = 3;
}

/* int32_t term_launch(uint8_t id)
 * Input:  terminal id
 * Return Value: 0 if success, -1 if fail
 * Function: launch the specific terminal */
int32_t term_launch(uint8_t id)
{
    cli();

    /* bound check */
    if(id >= TERM_MAX)    return -1;

    /* redundant check */
    if(id == cur_term_id) return 0;

    /* if terminal is already running */
    if(term[id].running){
        term_switch(cur_term_id,id);
        return 0;
    }

    /* if terminal is not running */
    if(cur_term_id < TERM_MAX) term_save(cur_term_id);
    term_restore(id);
    term[id].running = 1;

    /* start up shell */
    execute((uint8_t*)"shell");
    return 0;
}

/* int32_t term_save(uint8_t id)
 * Input:  terminal id
 * Return Value: none
 * Function: save the screen and process information to the current terminal */
int32_t term_save(uint8_t id)
{
    /* save buffer and screen info to the terminal */
    int j;

    for(j = 0; j < (int)KEY_BUF_MAX; j++){
        term[id].key_buf[j]=key_buf[j];
    }

    term[id].key_buf_idx=key_buf_idx;
    term[id].cursor_x = get_cursor_x();
    term[id].cursor_y = get_cursor_y();

    /* copy screen video memory to terminal video memory */
    memcpy(term[id].video_mem, (uint8_t*)VIDEO, 2*NUM_COLS*NUM_ROWS);
    return 0;
}

/* int32_t term_restore(uint8_t id)
 * Input:  terminal id
 * Return Value: none
 * Function: restore the terminal information to the screen and process */
int32_t term_restore(uint8_t id)
{
    /* load terminal info to buffer and screen */
    int j;
    for(j=0;j<(int)KEY_BUF_MAX;j++){
        key_buf[j]=term[id].key_buf[j];
    }
    key_buf_idx=term[id].key_buf_idx;
    set_screen_cursor(term[id].cursor_x,term[id].cursor_y);

    /* copy terminal video memory to screen video memory */
    memcpy((uint8_t*)VIDEO, term[id].video_mem, 2*NUM_COLS*NUM_ROWS);

    /* assign cur term id */
    cur_term_id = id;

    return 0;
}

/* int32_t term_switch(uint8_t old_id, uint8_t new_id)
 * Input:  terminal id
 * Return Value: none
 * Function: switch terminal from the old one to the new one */
int32_t term_switch(uint8_t old_id, uint8_t new_id)
{
    term_save(old_id);
    term_restore(new_id);
    return 0;
}

/* int32_t term_open(int8_t* file_name)
 * Input:  none
 * Return Value: none
 * Function: Initialized to map the corresponding function pointer in entry */
int32_t term_open(const uint8_t* file_name)
{
    return 0;
}

/* int32_t term_read(int32_t fd, uint8_t* buf, uint32_t length)
 * Input:  file descriptor, buffer, and length
 * Return Value: number of bytes read
 * Function: read key buffer into the read buffer */
int32_t term_read(int32_t fd, void* buf, int32_t length)
{
    sti();
    int32_t i;
    int8_t* temp_buf;
    if(buf==NULL)   return -1;          //check for NULL pointer
    while(term[now_term_id].enter_state==0);     //wait if enter is not pressed'
    term[now_term_id].enter_state=0;             //reset enter state
    temp_buf = (int8_t*)buf;
    for(i=0;(i<KEY_BUF_MAX)&&(i<length);i++){
        if(key_buf[i]=='\0') break;
        temp_buf[i] = key_buf[i];       //move key buffer to read buffer
    }
    buf_clear();
    return (int32_t)i;
}

/* int32_t term_write(int32_t fd, uint8_t* buf, uint32_t length)
 * Input:  file descriptor, buffer, and length
 * Return Value: number of bytes written
 * Function: print the write buffer to screen */
int32_t term_write(int32_t fd, const void* buf, int32_t length)
{
    int8_t temp_buf[length+1];          //additional slot for NULL char
    int32_t i;
    if(buf==NULL)   return -1;           //check NULL
    for(i=0;i<length;i++){
        if(((int8_t*)buf)[i]=='\0') break;
        temp_buf[i]=((int8_t*)buf)[i];  //copy length specificed bytes from buf to temp buf
    }
    temp_buf[i]='\0';                   //printf is '\0' terminated

    /* Two different types of output to the screen depending on 
     * whether current terminal is terminal being executed */
    if(now_term_id == cur_term_id) {printf(temp_buf);}
    else {multi_printf(temp_buf);}
    return (int32_t)i;
}

/* int32_t term_close(int8_t* file_name)
 * Input:  none
 * Return Value: none
 * Function: Initialized to map the corresponding function pointer in entry */
int32_t term_close(int32_t fd)
{
    return 0;
}
