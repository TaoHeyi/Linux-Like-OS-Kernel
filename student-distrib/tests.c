#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
#include "terminal.h"
#include "keyboard.h"

#define PASS 1
#define FAIL 0
#define MAX_SIZE 10000

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__);
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 20; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}
	return result;
}

// add more tests here

/* interrupt_0_test
 * 
 * Raises a divide by zero interrupt
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT definition
 * Files: idt.h/c
 */
int interrupt_0_test(){
	TEST_HEADER;

	int x, y, z;
	x = 1;
	y = 0;
	z = x / y;
	return FAIL;
}

/* interrupt_14_test
 * 
 * Attempt to dereference a null pointer
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT definition
 * Files: idt.h/c
 */
int interrupt_14_test(){
	TEST_HEADER;

	int* x = NULL;
	int y = *x;
	y = y;
	return FAIL;
}

/* multiple_interrupt_test
 * 
 * Attempt to send two interrupts
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT definition
 * Files: idt.h/c
 */
int multiple_interrupt_test(){
	TEST_HEADER;

	int x, y, z;
	x = 1;
	y = 0;
	z = x / y;
	int* a = NULL;
	int b = *a;
	b = b;
	return FAIL;
}

/* system_call_test
 * 
 * Use asm volatile to make system call
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT definition
 * Files: idt.h/c
 */
int system_call_test(){
	TEST_HEADER;

	asm volatile("int $128");
	return FAIL;
}

/* rtc_test
 * 
 * Force generate RTC interrupt
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: RTC initialization
 * Files: rtc.h/c
 */
int rtc_test(){
	TEST_HEADER;

	rtc_set_freq(4);
	return PASS;
}

/* The following seven tests are used to test paging functionality */
/* All magic number of this part are explained in the function header */

/* paging_test1
 * 
 * Attempt to dereference at where paging should not appear (edge case to 0xB8000)
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Raise page fault
 * Coverage: paging definition
 * Files: paging.h/c
 */
int paging_test1()
{
	TEST_HEADER;

	int result = PASS;
	char val = *((char *) 0xB7FFF);
	val = val;
	//assertion_failure();
	//result = FAIL;
	return result;
}

/* paging_test2
 * 
 * Attempt to dereference at where video memory appears of paging
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Should pass the test
 * Coverage: paging definition
 * Files: paging.h/c
 */
int paging_test2()
{
	TEST_HEADER;

	int result = PASS;
	char val = *((char *) 0xB8000);
	val = val;
	//assertion_failure();
	//result = FAIL;
	return result;
}

/* paging_test3
 * 
 * Attempt to dereference at address 0x00000
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Should raise the page fault
 * Coverage: paging definition
 * Files: paging.h/c
 */
int paging_test3()
{
	TEST_HEADER;

	int result = PASS;
	char val = *((char *) 0x00000);
	val = val;
	//assertion_failure();
	//result = FAIL;
	return result;
}

/* paging_test4
 * 
 * Attempt to dereference at very last byte of video memory
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: should pass the test
 * Coverage: paging definition
 * Files: paging.h/c
 */
int paging_test4()
{
	TEST_HEADER;

	int result = PASS;
	char val = *((char *) 0xB8000 + 0x1000 -1);
	val = val;
	return result;
}

/* paging_test5
 * 
 * Attempt to dereference at next byte of the very last byte of video memory
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: should raise a page fault
 * Coverage: paging definition
 * Files: paging.h/c
 */
int paging_test5()
{
	TEST_HEADER;

	int result = PASS;
	char val = *((char *) 0xB8000 + 0x1000);
	val = val;
	return result;
}

/* paging_test6
 * 
 * Attempt to dereference at location of the kernel memory
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: should pass the test
 * Coverage: paging definition
 * Files: paging.h/c
 */
int paging_test6()
{
	TEST_HEADER;

	int result = PASS;
	char val = *((char *) 0x400000);
	val = val;
	return result;
}

/* paging_test7
 * 
 * Attempt to dereference at location of the kernel memory plus an offset
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: should pass the test
 * Coverage: paging definition
 * Files: paging.h/c
 */
int paging_test7()
{
	TEST_HEADER;

	int result = PASS;
	char val = *((char *) 0x400000 + 0x400);
	val = val;
	return result;
}

/* paging_test8
 * 
 * Attempt to dereference at a large memory location, which should not be paged
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: should raise page fault
 * Coverage: paging definition
 * Files: paging.h/c
 */
int paging_test8()
{
	TEST_HEADER;

	int result = PASS;
	char val = *((char *) 0x80000000);
	val = val;
	return result;
}

/* Checkpoint 2 tests */

/* terminal_test
 * 
 * Open terminal and repeated read/write
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: print the user input to screen
 * Coverage: Terminal and Keyboard interaction
 * Files: terminal.h/c, keyboard.h/c, lib.h/lib.c
 */
int terminal_test(){
	TEST_HEADER;
	uint8_t buf[128];
	term_open(NULL);
	while(1){
		term_read(NULL,buf,128);
		term_write(NULL,buf,128);
	}
	term_close(NULL);
	return PASS;
}

/* fs_helper_test
 * 
 * Test the FS helper functions
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: File system
 * Files: file_system.h/c
 */
int fs_helper_test(){
	TEST_HEADER;

	dentry_t test;
	if(read_dentry_by_name("frame0.txt", &test) == -1) return FAIL;
	else if(read_dentry_by_index(1, &test) == -1) return FAIL;
	else return PASS;
}

/* fs_small_file_test
 * 
 * Open a small file and print to terminal
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: open and print small txt file content
 * Coverage: File system
 * Files: file_system.h/c
 */
int fs_small_file_test(){
	TEST_HEADER;

	int fd;
	fd = file_open((uint8_t*)"frame0.txt");
	int8_t buf[MAX_SIZE];
	if(file_read((int32_t)"frame0.txt", buf, MAX_SIZE) != -1){
		printf(buf);
		file_close(fd);
		return PASS;
	}
	file_close(fd);
	return FAIL;
}


/* fs_large_file_test
 * 
 * Open a large file and print to terminal
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: open and print large txt file content
 * Coverage: File system
 * Files: file_system.h/c
 */
int fs_large_file_test(){
	TEST_HEADER;

	int fd;
	fd = file_open((uint8_t*)"verylargetextwithverylongname.tx");
	int8_t buf[MAX_SIZE];
	if(file_read((int32_t)"verylargetextwithverylongname.tx", buf, MAX_SIZE) != -1){
		printf(buf);
		file_close(fd);
		return PASS;
	}
	file_close(fd);
	return FAIL;
}

/* fs_exe_file_test
 * 
 * Open a executable file and print to terminal
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: open and print executable file content
 * Coverage: File system
 * Files: file_system.h/c
 */
int fs_exe_file_test(){
	TEST_HEADER;

	int fd, i;
	fd = file_open((uint8_t*)"hello");
	int8_t buf[MAX_SIZE];
	for(i = 0; i < MAX_SIZE; i++){
		buf[i] = '\0';
	}
	if(file_read((int32_t)"hello", buf, MAX_SIZE) != -1){
		for(i = 0; i < MAX_SIZE; i++){
			if(buf[i] != '\0') putc(buf[i]);
		}
		file_close(fd);
		printf("\n");
		return PASS;
	}
	file_close(fd);
	return FAIL;
}

/* fs_directory_test
 * 
 * Open a small file and print terminal
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: open and print small txt file content
 * Coverage: File system
 * Files: file_system.h/c
 */
int fs_directory_test(){
	TEST_HEADER;

	/* FS_BLOCK_SIZE is 4096 here, which is 4KB */
	uint8_t buf[FS_BLOCK_SIZE];
	while(dir_read(0, buf, 0)){
		printf((int8_t*)buf);
		printf("\n");
	}
	return PASS;
}

/* rtc_general_test
 * 
 * Test RTC write by writing specific number to the RTC and reading it
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: print to screen at 4Hz
 * Coverage: RTC driver
 * Files: rtc.h/c
 */
int rtc_general_test(){
	TEST_HEADER;

	int num;
	int buf[1];
	/* Test open, 2Hz */
	num = 0;
	rtc_open(NULL);
	printf("2Hz: ");
	while(1){
		if(!rtc_read(0, NULL, 0)){
			num++;
			printf("%d ", num);
		}
		if(num >= 20) break;
	}
	printf("\n");
	/* Test write, 4Hz */
	num = 0;
	buf[0] = 4; 
	rtc_write(0, buf, 4);
	printf("4Hz: ");
	while(1){
		if(!rtc_read(0, NULL, 0)){
			num++;
			printf("%d ", num);
		}
		if(num >= 20) break;
	}
	printf("\n");
	/* Test write, 9Hz */
	num = 0;
	buf[0] = 9; 
	rtc_write(0, buf, 4);
	printf("9Hz: ");
	while(1){
		if(!rtc_read(0, NULL, 0)){
			num++;
			printf("%d ", num);
		}
		if(num >= 20) break;
	}
	printf("\n");
	/* Test write, 1025Hz */
	num = 0;
	buf[0] = 1025; 
	rtc_write(0, buf, 4);
	printf("1025Hz: ");
	while(1){
		if(!rtc_read(0, NULL, 0)){
			num++;
			printf("%d ", num);
		}
		if(num >= 20) break;
	}
	printf("\n");
	/* Test write, 8Hz */
	num = 0;
	buf[0] = 8; 
	printf("8Hz: ");
	rtc_write(0, buf, 4);
	while(1){
		if(!rtc_read(0, NULL, 0)){
			num++;
			printf("%d ", num);
		}
		if(num >= 20) break;
	}
	printf("\n");
	rtc_close(0);
	return PASS;
}

/* rtc_sweep_test
 * 
 * Sweep frequency from 2Hz to 1024Hz
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: print to screen at different valid frequency
 * Coverage: RTC driver
 * Files: rtc.h/c
 */
int rtc_sweep_test(){
	TEST_HEADER;

	int num;
	int buf[1];
	buf[0] = 2;
	rtc_open(NULL);
	while(1){
		printf("Sweeping at %dHz: \n", buf[0]);
		num = 0;
		while(1){
			if(!rtc_read(0, NULL, 0)){
				printf("*");
				num++;
			}
			if(num >= 2 * buf[0]) break;
		}
		buf[0] = (buf[0] << 1);
		if(rtc_write(0, buf, 4) == -1) break;
		clear();
		set_screen_cursor(0,0);
	}
	printf("\n");
	return PASS;
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests()
{
	// TEST_OUTPUT("idt_test", idt_test());

	// launch your tests here

	/* IDT tests here */
	// TEST_OUTPUT("divide_by_zero_test", interrupt_0_test());
	// TEST_OUTPUT("page_fault_exception_test", interrupt_14_test());
	// TEST_OUTPUT("multiple_interrupt_test", multiple_interrupt_test());
	// TEST_OUTPUT("system_call_test", system_call_test());

	/* RTC tests here */
	// TEST_OUTPUT("rtc_test", rtc_test());
	
	/* Paging tests here */
	/* first test should have page fault */
	// TEST_OUTPUT("paging_test 1", paging_test1());
	/* second test should pass */
	// TEST_OUTPUT("paging_test 2", paging_test2());
	/* third test should have page fault */
	// TEST_OUTPUT("paging test 3", paging_test3());
	/* fourth test should pass */
	// TEST_OUTPUT("paging test 4", paging_test4());
	/* fifth test should have page fault */
	// TEST_OUTPUT("paging test 5", paging_test5());
	/* sixth test should pass */
	// TEST_OUTPUT("paging_test 6", paging_test6());
	/* seventh test should pass */
	// TEST_OUTPUT("paging_test 7", paging_test7());
	/* eighth test should raise page fault */
	// TEST_OUTPUT("paging_test 8", paging_test8());

	/* terminal test */
	// TEST_OUTPUT("terminal_test",terminal_test());

	/* File system test */
	/* Test file system read helper functions */
	// TEST_OUTPUT("fs_helper_test", fs_helper_test());
	/* Test reading from a small txt file */
	// TEST_OUTPUT("fs_small_file_test", fs_small_file_test());
	/* Test reading from a large txt file */
	// TEST_OUTPUT("fs_large_file_test", fs_large_file_test());
	/* Test reading from a executable file */
	// TEST_OUTPUT("fs_exe_file_test", fs_exe_file_test());
	/* Test reading the file directory */
	// TEST_OUTPUT("fs_directory_test", fs_directory_test());

	/* RTC test */
	/* Open RTC and read, should read freq of 2Hz
	 * Then write 4 to RTC, should read freq of 4Hz
	 * Then write 9 to RTC, should read freq of 4Hz
	 * because 9 isn't power of 2
	 * write 1025 to RTC, should read freq of 4Hz
	 * because 1025 exceeds limit of 1024Hz
	 * Lastly write 8 to RTC, should read freq of 8Hz
	 */
	// TEST_OUTPUT("rtc_general_test", rtc_general_test());
	/* Sweep frequency from 2Hz to 1024Hz */
	// TEST_OUTPUT("rtc_sweep_test", rtc_sweep_test());
}
