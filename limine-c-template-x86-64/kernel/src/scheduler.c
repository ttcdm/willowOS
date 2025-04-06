#include <scheduler.h>



thread_context* current_thread;

void gen() {
	// kprint("runtime ");
    // kprintln_uint64(current_thread->total_run_time);
	// asm volatile ("sti");//enable interrupts and allows control to be passed back to scheduler. this seems kinda wrong to do but it works
	kprint("gen0: hi from thread ");
	// kpass(1000);
	while (1){
		int a = current_thread->pid;
		kprint("");
		//kprint("hi");
		//kprint_uint64(current_thread->pid);
		// kprintln_uint64(current_thread->frame[2]);
		
	}
}

void gen1() {
	// asm volatile ("sti");
	kprint("gen1: hi from thread ");
	while (1) {
		kprintln_uint64(current_thread->pid);
		
	}
}

void init_scheduler() {
	//so we have a thread.

	thread_context* new_thread = (thread_context*) kmalloc_byte(sizeof(thread_context));
	uint64_t* thread_base = kmalloc_byte(sizeof(uint64_t) * 2000);//16kb
	new_thread->start_time = tsc_read_ns();
	new_thread->last_start_time = 0;
	new_thread->total_run_time = 0;
	new_thread->quantum_ns = 10000000;//10ms
	new_thread->pid = 0;
	new_thread->thread_entry = gen;
	new_thread->stack_base = thread_base;
	new_thread->return_rsp = NULL;
	new_thread->misaligned_by = 0;
	new_thread->current_rsp = 0;
	new_thread->current_misaligned_by = 0;
	// new_thread->rip = NULL;
	new_thread->next_thread = NULL;
	current_thread = new_thread;

	for (int i = 1; i < 1; i++) {

		if (i%2==0 || i%2==1) current_thread->next_thread = create_thread(i, gen);
		// if (i%2==1) current_thread->next_thread = create_thread(i, gen1);

		current_thread = current_thread->next_thread;
	}
	current_thread->next_thread = new_thread;
	current_thread = current_thread->next_thread;//so we start on the 1st (1 indexed thread)
	while (1) {
		asm volatile ("int $67");
		current_thread = current_thread->next_thread;
		kpass(300);
	}
}

thread_context* create_thread(uint64_t pid, void (*thread_entry)(void)) {
	uint64_t* thread_base = kmalloc_byte(sizeof(uint64_t) * 2000);//16kb
	thread_context* new_thread = (thread_context*) kmalloc_byte(sizeof(thread_context));

	new_thread->start_time = tsc_read_ns();
	new_thread->last_start_time = 0;
	new_thread->total_run_time = 0;
	new_thread->quantum_ns = 10000000;//10ms
	new_thread->pid = pid;
	new_thread->thread_entry = thread_entry;
	new_thread->return_rsp = NULL;
	new_thread->misaligned_by = 0;
	new_thread->current_rsp = 0;
	new_thread->current_misaligned_by = 0;
	// new_thread->rip = NULL;
	new_thread->stack_base = thread_base;
	new_thread->next_thread = NULL;

    uint64_t thread_rsp = ((uint64_t) current_thread->stack_base) + THREAD_STACK_SIZE;//i'm not actually sure if kmalloc is supposed to return an address that's been casted to a pointer. either way, this reverts it so it should be okay for now i think
	uint64_t current_rsp;

	asm volatile ("mov %%rsp, %0 " : "=r"(current_rsp) :);//save current rsp
	asm volatile ("mov %0, %%rsp" : : "r"(thread_rsp));//load in new rsp

	asm volatile (//load cpu state
		"push %rax\n"
		"push %rbx\n"
		"push %rcx\n"
		"push %rdx\n"
		"push %rsi\n"
		"push %rdi\n"
		"push %rbp\n"
		"push %r8\n"
		"push %r9\n"
		"push %r10\n"
		"push %r11\n"
		"push %r12\n"
		"push %r13\n"
		"push %r14\n"
		"push %r15\n"
		);
	asm volatile ("mov %%rsp, %0" : "=r"(current_rsp));//restore rsp
	

	return new_thread;
}