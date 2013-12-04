/* Main file */
/* With boot and start functions */

#include <defs.h>
#include <stdarg.h>
#include <sys/gdt.h>
#include <print.h>
#include <idt.h>
#include <pic.h>
#include <timer.h>
#include <common.h>
#include <paging.h>
#include <process.h>
#include <sys/tarfs.h>
#include <shell.h>

void print_hello_world() {
	kprintf("\nHello World..!!");
}

void start(uint32_t* modulep, void* physbase, void* physfree)
{
	map_physical_address(modulep, physbase, physfree);
	// kernel starts here
	//print_hello_world();

	// Will reload the gdt
        reload_gdt();

        setup_tss();

	// Will initialize the IDT
        init_idt();

	// Will initialize the PIC and remap the interrupt number 0-15 to 32-47
        pic_remap(0x20, 0x28);

	// Will initialize the timer interrupt
        init_timer(100);

	// uint64_t message = stoi("023323232");	
	// kprintf("\n String to integer check: %d", message);
	
	// uint64_t decimal = octalToDecimal(232);
	// kprintf("\nOctal 232 to decimal is: %d", decimal);

	// int res = strcmp("HELLO","dsjhfkdhfkjhfjkshfd");
	// kprintf("\nComparing two strings: %d", res);

	//get_file_sections("/bin/hello/hello.o");
	
	// calling the first context switch
	// first_context_switch();
	// initContextSwitch();
	
	/*	
	char p[100];
	kscanf(p);
	int i = 0;
	while(p[i] != '\0') {
		kprintf("%c", p[i]);
		i++;
	}
	//kprintf("\n %s", p);
	*/
	showShell();
	while(1);
}

#define INITIAL_STACK_SIZE 4096
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase;

void boot(void)
{
	// note: function changes rsp, local stack variables can't be practically used
	//register char *temp1, *temp2;
	__asm__(
		"movq %%rsp, %0;"
		"movq %1, %%rsp;"
		:"=g"(loader_stack)
		:"r"(&stack[INITIAL_STACK_SIZE])
	);

// Will reload the gdt
	reload_gdt();

	setup_tss();

// Will initialize the IDT
        init_idt();

// Will initialize the PIC and remap the interrupt number 0-15 to 32-47
        pic_remap(0x20, 0x28);

// Will initialize the timer interrupt
        init_timer(100);

// code snippet to invoke an interrupt based upon arg number which is the interrupt number
//      int arg = 31;
//        __asm__("int %0\n" : : "N"((arg)) : "cc", "memory");

       	clear_screen();
       	start(
        	(uint32_t*)((char*)(uint64_t)loader_stack[3] + (uint64_t)&kernmem - (uint64_t)&physbase),
               	&physbase,
               	(void*)(uint64_t)loader_stack[4]
       	);

// code snippet to invoke the divide-by-zero interrupt
//      int a = 5;
//      int b = 5;
//      kprintf("%d", (a/(a-b)));

//      int a = 32;
//      kprintf("%p ", &a);
//      int *b = &a;
//      kprintf(" %d", *b);

        while(1);
}
