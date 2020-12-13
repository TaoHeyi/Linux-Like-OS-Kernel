# idt_handler.S - Handles system call, keyboard, and RTC
# vim:ts=4 noexpandtab
 
#define ASM     1

#include "idt_handler.h"

.global syc_handler

# macro for the interrupt wrapper
#define INT_WRAP(kernel_func,user_func) \
.globl kernel_func ;\
kernel_func: ;\
    pushal ;\
    pushfl ;\
    cld ;\
    call user_func ;\
    popfl ;\
    popal ;\
    iret

# wrapper for rtc, keyboard and system calls
INT_WRAP(kbd_handler,keyboard_interrupt_handler);
INT_WRAP(rtc_handler,rtc_interrupt_handler);
INT_WRAP(pit_handler,pit_interrupt_handler);
INT_WRAP(mse_handler,mouse_interrupt_handler);

.global PF
PF:
    movl %cr2,%ebx
    pushl %ebx
    call pf_handler

# System call handler
syc_handler:

    # Restore interrupt before system call
    # cli
    pushl %es
  	pushl %ds
  	pushl %ebx
  	pushl %ecx
  	pushl %edx
  	pushl %esi
  	pushl %edi
  	pushl %ebp
  	pushfl

    # Push arguments following C convention
    pushl %edx
    pushl %ecx
    pushl %ebx

    # First check for valid arg number called
    # EAX should have a number between 1 and 10
    cmpl $1, %eax
    jl invalid_callnum
    cmpl $10, %eax
    jg invalid_callnum

    # Call the corresponding system call
    # sti
    call *syc_jumptable(,%eax,4)
    # cli
    jmp end_system_call

invalid_callnum:
    # Return -1 in EAX if invalid
    movl $-1, %eax

end_system_call:
    # Clean up stack
    addl $12,%esp
    popfl
  	popl %ebp
  	popl %edi
  	popl %esi
  	popl %edx
 	popl %ecx
  	popl %ebx
  	popl %ds
  	popl %es
	# sti
    iret

# Jump table used for system calls
syc_jumptable:
    .long 0x0
    .long halt
    .long execute
    .long read
    .long write
    .long open
    .long close
    .long getargs
    .long vidmap
    .long set_handler
    .long sigreturn

