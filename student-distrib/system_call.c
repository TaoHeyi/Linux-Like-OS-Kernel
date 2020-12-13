/* system_call.c - functions for system calls
 * vim:ts=4 noexpandtab
 */

#include "system_call.h"

/* Process ID array */
int32_t PCB_mask[TERM_MAX][MAX_PCB_MASK_LEN] = {
                                               {0, 0, 0, 0},
                                               {0, 0, 0, 0},
                                               {0, 0, 0, 0}
                                               };

/* Static fop for specific files */
file_optable_t stdin_fop = {term_read,operation_error,term_open,term_close};
file_optable_t stdout_fop = {operation_error,term_write,term_open,term_close};
file_optable_t rtc_fop = {rtc_read,rtc_write,rtc_open,rtc_close};
file_optable_t dir_fop = {dir_read,dir_write,dir_open,dir_close};
file_optable_t file_fop = {file_read,file_write,file_open,file_close};
file_optable_t error_fop = {operation_error,operation_error,operation_error,operation_error};

/* int32_t halt (uint8_t status)
 * Input: 
 * Return Value: 
 * Function: halt the most recent process of currently running terminal */
int32_t halt (uint8_t status){   
    
    uint32_t i;
    int32_t cur_status;
    pcb_t* old_pcb;

    /* Clear miscellaneous keyboard input during execution of program */
    buf_clear();

    /* Obtain current PCB and update cur_pcb_id of that terminal */
    pcb_t* cur_pcb = get_pcb_from_id(term[now_term_id].cur_pcb_id+now_term_id*MAX_PCB_MASK_LEN);
    term[now_term_id].cur_pcb_id = cur_pcb->parent_process_number % MAX_PCB_MASK_LEN;

    /* Unmask the bit in the PCB_mask */
    PCB_mask[(uint8_t)cur_pcb->process_number / MAX_PCB_MASK_LEN][(uint8_t)cur_pcb->process_number % MAX_PCB_MASK_LEN] = 0;

    /* Disable the flags of the fds */
    for(i=0; i<MAX_FILE_NUM; i++)
    {   
        /* If the flag is 1, then we need to set it to 0 and then close it */
        if(cur_pcb->fds[i].flags == 1)
        {   
            cur_pcb->fds[i].flags = 0;
            close(i);
        }
        cur_pcb->fds[i].optable = error_fop;
    }

    /* If user attemp to close the last shell, restart it */
    if(cur_pcb->parent_process_number == -1){
        printf("Halting the last shell is not allowed!\n");
        execute((uint8_t*)"shell");
    }

    /* Get the parent pcb: 0x8000000 (128MB) - (process number + 1) * 0x2000 (8KB) */
    pcb_t * parent_pcb = (pcb_t*) (0x800000 - (cur_pcb->parent_process_number + 1) * 0x2000);

    /* Map parent's virtual address 0x8000000 (128MB) to physical address 0x800000 (8MB),
     * plus process_number number times 0x400000 (4MB) */
    pcb_mapping(0x8000000, 0x800000 + parent_pcb->process_number * 0x400000);

    /* record the cur_pcb to old_pcb for later use and update cur_pcb */
    old_pcb = cur_pcb;
    cur_pcb = parent_pcb;

    /* update tss information */
    tss.esp0 = 0x800000 - parent_pcb->process_number * 0x2000 - 0x4; /* 8MB - process number * 8KB - 4 */

    cur_status = status;

    /* If program halted by exception, or 0 with 256 to get halt(256) effect */
    if(interrupt_halt_flag) cur_status |= 0x100;
    interrupt_halt_flag = 0;

    asm volatile(
        "movl %0, %%eax;" /* Get status for return */
        "movl %1, %%ebp;" /* restore parent kbp */
        "movl %2, %%esp;" /* restore parent ksp */
        "leave;"
        "ret;"
        /* there is no output here */
        :   
        :"r"(cur_status), "r"(old_pcb->parent_kbp), "r"(old_pcb->parent_ksp)
        :"eax" /* clobber EAX */
    );
    return 0;
}

/* int32_t execute (const uint8_t* command)
 * Input: pointer to command being issued by user
 * Return Value: -1 -- Fail to execute
 *              256 -- Program squashed due to exception
 *            0-255 -- Program successfully executed
 * Function: execute a file given pointer to its location */
int32_t execute (const uint8_t* command){

    uint32_t i, offset;
    int32_t filename_flag;
    int32_t PCB_number;
    uint32_t entrypoint;
    uint8_t filename[MAX_FILENAME_LENGTH]; /* File name to be executed */
    uint8_t fileargs[MAX_ARG_LENGTH]; /* the file arguments after parsing */
    uint8_t headerbuf[HEADER_NUM]; /* Buffer for checking for magic number */
    uint8_t magicnumber[MAGIC_NUM] = {0x7f, 0x45, 0x4c, 0x46}; /* Expected magic number string */
    dentry_t loader; /* dentry for loading the program into VM */
    dentry_t magic_dentry;

    /* Check for invalid input */
    if(command == NULL) return -1;

    /* Initialize the filename and fileargs */
    /* May due to the reason we did not implement halt while test exectute program,
     * the first time we type in is exactly what we want,
     * the second time will cause filebuf full of trash. */
    for(i = 0; i < MAX_FILENAME_LENGTH; i++){
        filename[i] = '\0';
    }
    for(i = 0; i < MAX_ARG_LENGTH; i++){
        fileargs[i] = '\0';
    }

    /* Filename flag is -1 until we start recording filename */
    filename_flag = -1;
    /* Offset is 0 initially */
    offset = 0;

    /* Get filename and args */
    for(i = 0; command[i] != '\0'; i++){

        /* Check for long executable name */
        if(i > MAX_FILENAME_LENGTH && filename_flag == 0) return -1;

        /* Case we are getting leading spaces */
        if(command[i] == ' ' && filename_flag == -1){
            /* Skipping the spaces */
            continue;
        }
        /* Case we are recording the file name */
        else if(command[i] != ' ' && (filename_flag == -1 || filename_flag == 0)){
            /* Flag is set to 0 because we are currently recording filename */
            filename_flag = 0;
            filename[offset] = command[i];
            offset++;
        }
        /* Case we are getting the middle extra spaces */
        else if(command[i] == ' ' && (filename_flag == 0 || filename_flag == 1)){
            /* Mark end of recording filename */
            filename_flag = 1;
            offset = 0;
            continue;
        }
        /* Case we are starting to record the arguments */
        else if(filename_flag == 1 || filename_flag == 2){
            filename_flag = 2;
            fileargs[offset] = command[i];
            offset++;
        }
    }

    /* Check valid name */
    if(read_dentry_by_name((int8_t*)filename, &magic_dentry) == -1) return -1;

    /* Check valid magic number for indicating it is an executable */
    read_data(magic_dentry.inode,0,headerbuf,HEADER_NUM);
    if(strncmp((const int8_t*)headerbuf, (const int8_t*)magicnumber, MAGIC_NUM) != 0) return -1;

    /*----------------------------------------------- Paging -----------------------------------------------*/

    /* Get the process free number and total task number in the PCB */
    PCB_number = PCB_process_number();

    /* If there are none free spaces, then return 0 */
    if(PCB_number == -1)
    {
        printf("There are no available space in the PCB\n");
        return 0;
    }

    /* Map the virtual address 0x8000000 (128MB) to physical address 0x800000 (8MB)
     * with some offsets depending on the pcb number we have */
    pcb_mapping(0x8000000, 0x800000 + PCB_number * 0x400000);

    /*----------------------------------------------- Loader -----------------------------------------------*/

    /* Load file to be executed into VM
     * First get file information into the loader
     * Then read file information into starting address of VM */
    if(read_dentry_by_name((int8_t*)filename, &loader) == -1) return -1;
    if(read_data(loader.inode, 0, (uint8_t*)START_VITURAL_ADDR, inodeblk[loader.inode].size) == -1) return -1;

    /*--------------------------------------------- Create PCB ---------------------------------------------*/
    
    /* Initialize the pointer to the memory of the PCB stack, 0x800000 for 8MB and 0x2000 for 8KB */
    pcb_t* pcb = (pcb_t*) (0x800000 - (PCB_number + 1) * 0x2000);

    /* Save current ESP and EBP into the struct as previous KSP/KBP */
    asm volatile(
        "movl %%ebp, %%eax;"
        "movl %%esp, %%ebx;"
        /* there is no input here */
        :"=a"(pcb->parent_kbp), "=b"(pcb->parent_ksp)
    );

    /* assign process number and cur_term_id to pcb, and also assign pcb id to terminal */
    pcb->process_number = PCB_number;
    pcb->term_id = cur_term_id;
    term[cur_term_id].cur_pcb_id = PCB_number % 4;

    /* Set parent process number to -1 if it is the first process in a terminal */
    if(PCB_number % 4 == 0) pcb->parent_process_number = -1;
    /* Otherwise parent is the previous process in the terminal */
    else pcb->parent_process_number = pcb->process_number - 1;


    /* initialize file descriptor for each file */
    for(i = 0;i < MAX_FILE_NUM; i++){ //i is used here, check if i need to be reserved from the above content
        pcb->fds[i].optable = error_fop;
        pcb->fds[i].inode = NULL;
        pcb->fds[i].file_position = 0;
        pcb->fds[i].flags = 0;
    }
    /* initialize file descriptor for stdin and stdout */
    pcb->fds[0].optable = stdin_fop;
    pcb->fds[0].flags = 1;  
    pcb->fds[1].optable = stdout_fop;
    pcb->fds[1].flags = 1;

    /* Initialize argument buffer to empty string */
    for(i = 0; i < MAX_ARG_LENGTH; i++){
        pcb->argbuf[i] = '\0';
    }

    /* Store user arguments into user_buf, ending with null */
    for(i = 0; i < MAX_ARG_LENGTH; i++){
        pcb->argbuf[i] = fileargs[i];
    }
    pcb->argbuf[i] = '\0';

    /*------------------------------------------- Context Switch -------------------------------------------*/

    /* Modify TSS */
    tss.esp0 = 0x800000 - PCB_number * 0x2000 - 0x4; /* 8MB - process number * 8KB - 4 */
    tss.ss0 = KERNEL_DS;

    /* Flush the TLB */
    flush();

    /* Let parent shell know that current program is operating normally */
    interrupt_halt_flag = 0;
    
    /* Shift bytes and cast into 4-byte unsigned int */
    entrypoint = (headerbuf[27] << 24) | (headerbuf[26] << 16) | (headerbuf[25] << 8) | headerbuf[24];

    asm volatile(
        "pushl $0x002B;" /* Push USER_DS */
        "pushl $0x83FFFFC;" /* Push ESP, 132MB - 4B */
        "pushfl;" /* Push EFLAGS */
        "popl %%eax;" /* Get EFLAGS to change bit */
        "orl $0x200, %%eax;" /* Alter interrupt bit */
        "pushl %%eax;" /* Push EFLAGS again */
        "pushl $0x0023;" /* Push USER_CS */
        "pushl %0;"  /* Push entrypoint(EIP) */
        "iret;" /* Use IRET to fake the jump to next program */
        :       /* there is no output here */
        :"r"(entrypoint)
        :"eax" /* clobber EAX */
    );

    return 0;
}

/* int32_t read (int32_t fd, void* buf, int32_t  nbytes)
 * Input: file descriptor, buffer, number of bytes that will be operated
 * Return Value: number of bytes read if success, -1 if fail
 * Function: perform read on a specific file */
int32_t read (int32_t fd, void* buf, int32_t  nbytes){

    /* Obtain the current PCB */
    pcb_t* cur_pcb = get_pcb_from_id(term[now_term_id].cur_pcb_id + now_term_id*MAX_PCB_MASK_LEN);

    /* Check for invalid fd */
    if(fd >= MAX_FILE_NUM || fd < 0) return -1;

    /* Check for invalid buf input */
    if(buf == NULL) return -1;

    /* Check for whether file is currently open */
    if(cur_pcb->fds[fd].flags == 0) return -1;

    /* Call corresponding read function using optable */
    return cur_pcb->fds[fd].optable.read(fd,buf,nbytes);
}

/* int32_t write (int32_t fd, const void* buf, int32_t nbytes)
 * Input: file descriptor, buffer, number of bytes that will be operated
 * Return Value: number of bytes written if success, -1 if fail
 * Function: perform write on a specific file */
int32_t write (int32_t fd, const void* buf, int32_t nbytes){

    /* Obtain the current PCB */
    pcb_t* cur_pcb = get_pcb_from_id(term[now_term_id].cur_pcb_id + now_term_id*MAX_PCB_MASK_LEN);

    /* Check for invalid fd */
    if(fd >= MAX_FILE_NUM || fd < 0) return -1;

    /* Check for invalid buf input */
    if(buf == NULL) return -1;

    /* Check for whether file is currently open */
    if(cur_pcb->fds[fd].flags == 0) return -1;

    /* Call corresponding read function using optable */
    return cur_pcb->fds[fd].optable.write(fd,buf,nbytes);
}

/* int32_t open (const uint8_t* filename)
 * Input: file name
 * Return Value: file descriptor if success, -1 if fail
 * Function: put the new file in the next available place in the file descriptor arrary */
int32_t open (const uint8_t* filename){

    int32_t fd;

    /* Obtain the current PCB */
    pcb_t* cur_pcb = get_pcb_from_id(term[now_term_id].cur_pcb_id + now_term_id*MAX_PCB_MASK_LEN);

    /* get current file's dentry information */
    dentry_t local_dentry;
    if (read_dentry_by_name((int8_t*)filename, &local_dentry) == -1) return -1;

    /* get next available fd and change the flag to 1 */
    for (fd = 0; fd < MAX_FILE_NUM; fd++) {
        if (cur_pcb->fds[fd].flags == 0) {
            cur_pcb->fds[fd].flags = 1;
            cur_pcb->fds[fd].file_position = 0;
            break;
        }
    }

    /* no available spot in file descriptor array, return -1 */
    if (fd >= MAX_FILE_NUM) return -1;

    /* assign function pointer and inode value based on file type */
    switch(local_dentry.filetype){
        case RTC_TYPE:
            if (rtc_open(filename) != 0) return -1;
            cur_pcb->fds[fd].inode = NULL;
            cur_pcb->fds[fd].optable = rtc_fop;
            break;
        case DIR_TYPE:
            if (dir_open(filename) != 0) return -1;
            cur_pcb->fds[fd].inode = NULL;
            cur_pcb->fds[fd].optable = dir_fop;
            break;
        case FILE_TYPE:
            if (file_open(filename) != 0) return -1;
            cur_pcb->fds[fd].inode = local_dentry.inode;
            cur_pcb->fds[fd].optable = file_fop;
            break;
        default:    
            break;
    }
    /* return file descriptor */
    return fd;
}

/* int32_t close (int32_t fd)
 * Input: file descriptor
 * Return Value: 0 if success, -1 if fail
 * Function: close the corresponding file */
int32_t close (int32_t fd){

    /* Obtain the current PCB */
    pcb_t* cur_pcb = get_pcb_from_id(term[now_term_id].cur_pcb_id + now_term_id*MAX_PCB_MASK_LEN);

    /* Check for invalid fd, note that stdin and stdout cannot be closed */
    if(fd >= MAX_FILE_NUM || fd < 2) return -1;

    /* set corresponding file flag to 0 if applicable, else return -1 */
    if (cur_pcb->fds[fd].flags == 0) return -1;
    cur_pcb->fds[fd].flags = 0; 

    /* Call corresponding close function using optable */
    return cur_pcb->fds[fd].optable.close(fd);
}

/* int32_t getargs (uint8_t* buf, int32_t nbytes)
 * Input: address of user space buffer and size of buffer
 * Return Value: 0 if success, -1 if fail
 * Function: Return the user typed arguments to buf */
int32_t getargs (uint8_t* buf, int32_t nbytes){

    int32_t i, arg_length;  

    /* Obtain the current PCB */
    pcb_t* cur_pcb = get_pcb_from_id(term[now_term_id].cur_pcb_id + now_term_id*MAX_PCB_MASK_LEN);

    /* Get current argument length from PCB */
    arg_length = strlen((int8_t*)cur_pcb->argbuf);

    /* Return -1 if no argument or not enough space */
    if(arg_length <= 0 || arg_length >= nbytes) return -1;

    for(i = 0; i < arg_length; i++){
        buf[i] = cur_pcb->argbuf[i];
    }

    /* Add null ending for the arg */
    buf[i] = '\0';

    return 0;
}

/* int32_t vidmap (uint8_t** screen_start)
 * Input: screen_start, which is point we need to modify to virtual address for our virtual memory
 * Return Value: 0 if successful, -1 if failure
 * Function: syscall that map the text-mode video memory into user space at a pre-set virtual address */
int32_t vidmap (uint8_t** screen_start){

    /* Check whether the pointer passed in is a NULL pointer or not */
    if(screen_start == NULL) {return -1;}

    /* Make sure the pointer falls in user-level range 0x8000000(128MB) to 0x8400000(132MB) */
    if((uint32_t)screen_start < 0x8000000 || (uint32_t)screen_start>=0x8400000) {return -1;}

    /* Call the syscall_video_mapping function to map from physical address for video memory to virtual address */
    /* 0xB8000 is the physical address we want to map to the video memory */
    syscall_video_mapping(0xB8000);

    /* Map the screen_start to the virtual address */
    /* 0x8400000 is the address 132MB, which we used to map to our video memory */
    *screen_start = (uint8_t*)0x8400000;

    return 0;
}

/* int32_t getargs (int32_t signum, void* handler_address)
 * Input: signal number being called and the handler function address
 * Return Value: -1(FAIL) until we implement signal
 * Function: handles the signal received */
int32_t set_handler (int32_t signum, void* handler_address){
    return -1;
}

/* int32_t getargs (void)
 * Input: NULL
 * Return Value: -1(FAIL) until we implement signal
 * Function: return signal being handled */
int32_t sigreturn (void){
    return -1;
}

/************** Helper Functions Are In This Section **************/

/* int32_t PCB_process_number()
 * Input: None
 * Return Value: Return index in that PCB that is free. 
 * If there are no free spaces in current terminal, return -1
 * Function:  Find next available PCB in current terminal */
int32_t PCB_process_number()
{
    int32_t i;

    for(i = 0; i < MAX_PCB_MASK_LEN; i++)
    {
        if(PCB_mask[cur_term_id][i] == 0)
        {
            PCB_mask[cur_term_id][i] = 1;
            /* If more than 6 processes allocated, return -1 */
            if(PCB_total_number() > 6){
                PCB_mask[cur_term_id][i] = 0;
                return -1;
            }
            return i + cur_term_id * MAX_PCB_MASK_LEN;
        }
    }
    return -1;
}

/* int32_t PCB_total_number()
 * Input: None
 * Return Value: Return total number of active tasks. 
 * If there are no free spaces in total PCB array, return -1
 * Function:  Find whether we have available space in PCB */
int32_t PCB_total_number()
{
    int32_t i, j, sum = 0;

    for(i = 0; i < TERM_MAX; i++){
        for(j = 0; j < MAX_PCB_MASK_LEN; j++){
            if(PCB_mask[i][j] != 0) sum++;
        }
    }
    return sum;
}

/* pcb_t* get_cur_pcb()
 * Input: None
 * Return Value: current pcb pointer
 * Function:  Find the current pcb pointer based on stack pointer */
pcb_t* get_cur_pcb()
{
    pcb_t* curr;
    asm volatile(
        "andl %%esp, %%eax;"
        :"=a"(curr)         /* get current pcb pointer based on esp */
        :"a"(0xFFFFE000)    /* mask it with !8kb */
        :"cc"
    );
    return curr;
}

/* pcb_t* get_cur_pcb_from_id(uint8_t id)
 * Input: None
 * Return Value: pcb pointer of specified if
 * Function:  Find the pcb pointer based on pcb id*/
pcb_t* get_pcb_from_id(uint8_t id)
{
    /* formula: 8MB-(id+1)*8KB */
    return (pcb_t*)(0x800000-(id+1)*0x2000);
}

/* int32_t operation_error()
 * Input: None
 * Return Value: -1
 * Function:  function pointer error operation to map into file operation table */
int32_t operation_error() 
{
    return -1;
}
