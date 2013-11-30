#include <process.h>
#include <defs.h>
#include <stdio.h>
#include <paging.h>
#include <sys/gdt.h>

#define yield() __asm__ __volatile__("int $0x80")

static uint32_t ids = 0;
extern pml4e *pml4e_table;
runQueue* currProcess;
bool firstSwitch = true;

void function1();
void function2();

// To fetch the kernel virtual address
void* get_kva(page *pp)
{
    	uint64_t ppn_addr= page2ppn(pp) << PGSHIFT;
    	//uint32_t __m_ppn = ppn_addr >> PTXSHIFT ;

    	uint64_t kva_addr = ppn_addr + KERNBASE; 
    	return (void *)kva_addr;
}

// Will create a circular linked list for process in the run queue
void createProcess(void* function) {
	// create the process structure
	task process;
	page* pp = page_alloc(0);
	uint64_t* pml4e_p = (uint64_t* )get_kva(pp);			// initiaize the memory str for process
	pml4e_p[511] = pml4e_table[511];			// copy the kernel address space in process VM
	
	process.pid = ids++;
	process.cr3 = (uint64_t)PADDR((uint64_t)pml4e_p);		// init cr3
	process.pml4e_p = pml4e_p;
	
	process.stack[59] = (uint64_t)function;
        process.rsp = (uint64_t)&process.stack[45];

        process.stack[63] = 0x23 ;                              //  Data Segment    
        process.stack[62] = (uint64_t)(kmalloc(4096) + (uint64_t)4096);      //  RIP
        process.stack[61] = 0x246;                           //  EFlags
        process.stack[60] = 0x1b ;                              // Code Segment
		
	if(ids == 1) {
		runQ->process = process;
		runQ->next = runQ;
	}	
	else {
		runQueue* newProcess = NULL;
		newProcess->process = process;
		newProcess->next = runQ;
		
		runQueue* temp = runQ;
		while(temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = newProcess;
	}
}

// Will switch to first process in the run Queue
void initContextSwitch() {
	// create two user space process
	createProcess(&function1);
	createProcess(&function2);

	firstSwitch = false;		// unset the first context switch flag
	
	currProcess = runQ;		// init the current Process with head of runQ
	task pr = currProcess->process;

	__asm__ __volatile__("movq %0, %%cr3":: "a"(pr.cr3));		// load the cr3 reg with the cr3 of process
	printf("I am in process virtual address space \n");
	
	__asm__ __volatile__ (						
            "movq %0, %%rsp;" //load next's stack in rsp
            :
            :"r"(pr.rsp)
    	);								
		
	// pop all the general purpose registers
	__asm__ __volatile__("popq %r15");
	__asm__ __volatile__("popq %r14");
	__asm__ __volatile__("popq %r13");
	__asm__ __volatile__("popq %r12");
	__asm__ __volatile__("popq %r11");
        __asm__ __volatile__("popq %r10");
        __asm__ __volatile__("popq %r9");
        __asm__ __volatile__("popq %r8");
        __asm__ __volatile__("popq %rdi");
        __asm__ __volatile__("popq %rsi");
        __asm__ __volatile__("popq %rdx");
        __asm__ __volatile__("popq %rcx");
        __asm__ __volatile__("popq %rbx");
        __asm__ __volatile__("popq %rax");
	__asm__ __volatile__("sti");	

	tss.rsp0 = (uint64_t)&pr.stack[63];  

    	__asm__ __volatile__("mov $0x2b,%ax");
    	__asm__ __volatile__("ltr %ax");

    	__asm__ __volatile__("iretq");
}

// Will switch the process the processes in round robin manner using the circular
// run Queue
void switchProcess() {
	if(firstSwitch) {
		initContextSwitch();		
	}
	else {
		// code commented because the push operation is done from the timer interrupt handler
        	/*			
		__asm__ __volatile__ (
        		"pushq %rax;"\
        		"pushq %rbx;"\
        		"pushq %rcx;"\
	        	"pushq %rdx;"\
        		"pushq %rsi;"\
        		"pushq %rdi;"\
        		"pushq %r8;"\
            		"pushq %r9;"
            		"pushq %r10;"\
            		"pushq %r11;"
            		"pushq %r12;"
            		"pushq %r13;"
            		"pushq %r14;"
            		"pushq %r15;"
        	);
		*/
		__asm__ __volatile__(					// save the context of curr Process
            		"movq %%rsp, %0;"
            		:"=m"(currProcess->process.rsp)
            		:
            		:"memory"
        	);
    			
		currProcess = currProcess->next;			// move the curr Process to the next process in runQ
		
		task next = currProcess->process;
		asm volatile("movq %0, %%cr3":: "a"(next.cr3));	
    	
		__asm__ __volatile__ (
            		"movq %0, %%rsp;"
            		:
            		:"m"(next.rsp)
            		:"memory"
        	);
		
		tss.rsp0 = (uint64_t)next.stack[63];
        
		__asm__ __volatile__("popq %r15");
		__asm__ __volatile__("popq %r14");
        	__asm__ __volatile__("popq %r13");
        	__asm__ __volatile__("popq %r12");
        	__asm__ __volatile__("popq %r11");
        	__asm__ __volatile__("popq %r10");
        	__asm__ __volatile__("popq %r9");
        	__asm__ __volatile__("popq %r8");
        	__asm__ __volatile__("popq %rdi");
        	__asm__ __volatile__("popq %rsi");
        	__asm__ __volatile__("popq %rdx");
        	__asm__ __volatile__("popq %rcx");
        	__asm__ __volatile__("popq %rbx");
        	__asm__ __volatile__("popq %rax");
        	__asm__ __volatile__("sti");  
        	__asm__ __volatile__("iretq");		
	}
}

void initThreads() {
	page* pp1 = NULL;
        page* pp2 = NULL;

        // initialize both the task structures
        // set the address of function1 to the start of the stack of thread1 and same for thread2
        // set the rsp to point to the stack	

	pp1 = page_alloc(0);
        pp2 = page_alloc(0);

        uint64_t *pml4a = (uint64_t *)get_kva(pp1);
        uint64_t *pml4b = (uint64_t *)get_kva(pp2);


        pml4a[511] = pml4e_table[511]; //point to pdpe of kernel
        pml4b[511] = pml4e_table[511]; //point to pdpe of kernel

        thread1.cr3 = (uint64_t)PADDR((uint64_t)pml4a);
        thread2.cr3 = (uint64_t)PADDR((uint64_t)pml4b);

        thread1.stack[59] = (uint64_t)&function1;
        thread1.rsp = (uint64_t)&thread1.stack[45];

        thread1.stack[63] = 0x23 ;                              //  Data Segment    
        thread1.stack[62] = (uint64_t)(kmalloc(4096) + (uint64_t)4096);      //  RIP
        //thread1.stack[61] = 0x20202 ;                           //  RFlags
        thread1.stack[61] = 0x246;                           //  EFlags
        thread1.stack[60] = 0x1b ;                              // Code Segment

        thread2.stack[59] = (uint64_t)&function2;
        thread2.rsp = (uint64_t)&thread2.stack[45];

        thread2.stack[63] = 0x23 ;                              //  Data Segment    
        thread2.stack[62] = (uint64_t)(kmalloc(4096) + (uint64_t)4096);      //  RIP
        //thread1.stack[61] = 0x20202 ;                           //  RFlags
        thread2.stack[61] = 0x246;                           //  EFlags
        thread2.stack[60] = 0x1b ;                              // Code Segment

        // inilialize the ready queue with both the task structures
        readyQ[0] = thread1;
        readyQ[1] = thread2;

	// initialize flags	
	firstSwitch = true;
	flag = true;
}

void first_context_switch() 
{    
	initThreads();
    	
	firstSwitch = false;
	
	// load the value of rsp of thread1 into the kernel rsp
    	// this will cause context switch

   	// __asm__("cli");
    	asm volatile("movq %0, %%cr3":: "a"(thread1.cr3));
   	// __asm__("sti");
        

    	printf("I am in process virtual address space \n");

    	__asm__ __volatile__ (
            "movq %0, %%rsp;" //load next's stack in rsp
            :
            :"r"(thread1.rsp)
    	);

	__asm__ __volatile__("popq %r15");
	__asm__ __volatile__("popq %r14");
	__asm__ __volatile__("popq %r13");
	__asm__ __volatile__("popq %r12");
	__asm__ __volatile__("popq %r11");
        __asm__ __volatile__("popq %r10");
        __asm__ __volatile__("popq %r9");
        __asm__ __volatile__("popq %r8");
        __asm__ __volatile__("popq %rdi");
        __asm__ __volatile__("popq %rsi");
        __asm__ __volatile__("popq %rdx");
        __asm__ __volatile__("popq %rcx");
        __asm__ __volatile__("popq %rbx");
        __asm__ __volatile__("popq %rax");
	__asm__ __volatile__("sti");	

	tss.rsp0 = (uint64_t)&thread1.stack[63];  

    	__asm__ __volatile__("mov $0x2b,%ax");
    	__asm__ __volatile__("ltr %ax");

    	__asm__ __volatile__("iretq");
}


void switch_to(task* prev, task* next) {
    	// will save the content prev task on the stack
    	// will update the value of the current rsp to the point to the rsp of the next task
    	// this will cause the context switch from prev task to next task
    	
	// code commented because the push operation is done from the timer interrupt handler
        /*			
	__asm__ __volatile__ (
        	"pushq %rax;"\
        	"pushq %rbx;"\
        	"pushq %rcx;"\
        	"pushq %rdx;"\
        	"pushq %rsi;"\
        	"pushq %rdi;"\
        	"pushq %r8;"\
            	"pushq %r9;"
            	"pushq %r10;"\
            	"pushq %r11;"
            	"pushq %r12;"
            	"pushq %r13;"
            	"pushq %r14;"
            	"pushq %r15;"
        );
	*/
    	__asm__ __volatile__(
            	"movq %%rsp, %0;"
            	:"=m"(prev->rsp)
            	:
            	:"memory"
        );
    	
	asm volatile("movq %0, %%cr3":: "a"(next->cr3));
    	
	__asm__ __volatile__ (
            	"movq %0, %%rsp;"
            	:
            	:"m"(next->rsp)
            	:"memory"
        );

	tss.rsp0 = (uint64_t)&next->stack[63];
        
	__asm__ __volatile__("popq %r15");
	__asm__ __volatile__("popq %r14");
        __asm__ __volatile__("popq %r13");
        __asm__ __volatile__("popq %r12");
        __asm__ __volatile__("popq %r11");
        __asm__ __volatile__("popq %r10");
        __asm__ __volatile__("popq %r9");
        __asm__ __volatile__("popq %r8");
        __asm__ __volatile__("popq %rdi");
        __asm__ __volatile__("popq %rsi");
        __asm__ __volatile__("popq %rdx");
        __asm__ __volatile__("popq %rcx");
        __asm__ __volatile__("popq %rbx");
        __asm__ __volatile__("popq %rax");
        __asm__ __volatile__("sti");  
        __asm__("iretq");
}

void schedule() {
    	// will halt the currently executing thread
    	// will pop the next task from the ready queue
    	// call switch_to function with prev and next task
        //printf("In schedule%s %s", firstSwitch, flag);	
	if(firstSwitch) {
		firstSwitch = false;
		first_context_switch();
	}
	else {
		if(flag) {
                	flag = false;
                	switch_to(&readyQ[0], &readyQ[1]);
        	}
        	else {
                	flag = true;
                	switch_to(&readyQ[1], &readyQ[0]);
        	}
	}
}

void function1() {
	// to call the synthetic interrupts
		//int arg = 14;
	// to call the hardware originated divide by zero interrupt
		//__asm__("int %0\n" : : "N"((arg)) : "cc", "memory");
		//uint64_t *a = 0x0ul; int b = *a;
		//printf("%d",b);

	printf("\nHello");    		
	while(1) {
		static int i = 0;
        	printf("\nHello inside while: %d", i++);
        	//schedule();
        	//yield();
   	}
}

void function2() {
	printf("\nWorld..!!");
    	while(1) {
		static int i = 0;
        	printf("World..!! inside while: %d", i++);
        	//schedule();
        	//yield();
   	}
}
