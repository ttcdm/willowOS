#include <scheduler.h>



thread_context* current_thread;

void gen() {
	// kprint("runtime ");
    // kprintln_uint64(current_thread->total_run_time);
	kprint("hi from thread ");
	kprintln_uint64(current_thread->pid);
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
	new_thread->next_thread = NULL;
	current_thread = new_thread;

	for (int i = 1; i < 4; i++) {
		//if (i == 0) {//i don't think that this if block is actually necessary since i think the rest of the stuff just does the same thing
		//	new_thread->next_thread = create_thread(i);
		//	current_thread = new_thread->next_thread
		//	continue;
		//}
		current_thread->next_thread = create_thread(i, gen);
		current_thread = current_thread->next_thread;
	}
	current_thread->next_thread = new_thread;
	
	// kprintln("hi");
	while (1) {
		// continue;
		asm volatile ("int $67");
		// kprintln("current thread: "); kprintln_uint64(current_thread->pid);

		// kprintln("hihihi");
		current_thread = current_thread->next_thread;
		kpass(300);
	}
}

thread_context* create_thread(uint64_t pid, void (*thread_entry)(void)) {
	uint64_t* thread_base = kmalloc_byte(sizeof(uint64_t) * 2000);//16kb
	//thread_context* thread = (thread_context*)thread_base;

	thread_context* new_thread = (thread_context*) kmalloc_byte(sizeof(thread_context));
	// kprintln("hihihi");
	// kprintln_uint64((uint64_t) thread_base);

	new_thread->start_time = tsc_read_ns();
	new_thread->last_start_time = 0;
	new_thread->total_run_time = 0;
	new_thread->quantum_ns = 10000000;//10ms
	new_thread->pid = pid;
	new_thread->thread_entry = thread_entry;
	new_thread->return_rsp = NULL;
	new_thread->misaligned_by = 0;
	new_thread->stack_base = thread_base;
	new_thread->next_thread = NULL;

    uint64_t thread_rsp = ((uint64_t) current_thread->stack_base) + THREAD_STACK_SIZE;//i'm not actually sure if kmalloc is supposed to return an address that's been casted to a pointer. either way, this reverts it so it should be okay for now i think
	uint64_t current_rsp;

	asm volatile ("mov %%rsp, %0 " : "=r"(current_rsp) :);
	// kprintln_uint64(current_rsp);
	// asm volatile ("mov %%rsp, %0" : : "r"(thread_rsp));
	asm volatile ("mov %0, %%rsp" : : "r"(thread_rsp));
	// asm volatile ("mov %%rsp, %0 " : "=r"(current_rsp) :);
	// kprintln_uint64(current_rsp);

	// push_all_regs();
	// pop_all_regs();
	// asm volatile ("push %rax");
	asm volatile (
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
	asm volatile ("mov %%rsp, %0" : "=r"(current_rsp));
	// asm volatile (
	// 	"pop %r15\n"
	// 	"pop %r14\n"
	// 	"pop %r13\n"
	// 	"pop %r12\n"
	// 	"pop %r11\n"
	// 	"pop %r10\n"
	// 	"pop %r9\n"
	// 	"pop %r8\n"
	// 	"pop %rbp\n"
	// 	"pop %rdi\n"
	// 	"pop %rsi\n"
	// 	"pop %rdx\n"
	// 	"pop %rcx\n"
	// 	"pop %rbx\n"
	// 	"pop %rax\n"
	// );
	

	return new_thread;
}