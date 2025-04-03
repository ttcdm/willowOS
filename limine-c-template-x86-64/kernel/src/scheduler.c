#include <scheduler.h>



thread_context* current_thread;

void init_scheduler() {
	//so we have a thread.

	thread_context* new_thread = kmalloc_byte(sizeof(thread_context));
	uint64_t* thread_base = kmalloc_byte(sizeof(uint64_t) * 2000);//16kb
	new_thread->start_time = 0;
	new_thread->last_start_time = 0;
	new_thread->total_run_time = 0;
	new_thread->quantum_ns = 10000000;//10ms
	new_thread->pid = 0;
	new_thread->stack_base = thread_base;
	new_thread->next_thread = NULL;
	current_thread = new_thread;

	for (int i = 1; i < 3; i++) {
		//if (i == 0) {//i don't think that this if block is actually necessary since i think the rest of the stuff just does the same thing
		//	new_thread->next_thread = create_thread(i);
		//	current_thread = new_thread->next_thread
		//	continue;
		//}
		current_thread->next_thread = create_thread(i);
		current_thread = current_thread->next_thread;
	}
	current_thread->next_thread = new_thread;
	

	while (1) {
		asm volatile ("int $67");
		//kpass(500);
		kprintln("current thread: "); kprintln_uint64(current_thread->pid);
		current_thread = current_thread->next_thread;
	}
}

thread_context* create_thread(uint64_t pid) {
	uint64_t* thread_base = kmalloc_byte(sizeof(uint64_t) * 2000);//16kb
	//thread_context* thread = (thread_context*)thread_base;

	thread_context* new_thread = kmalloc_byte(sizeof(thread_context));
	new_thread->start_time = 0;
	new_thread->last_start_time = 0;
	new_thread->total_run_time = 0;
	new_thread->quantum_ns = 10000000;//10ms
	new_thread->pid = pid;
	new_thread->stack_base = thread_base;
	new_thread->next_thread = NULL;
	return new_thread;
}