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
		// kprint("");
		//kprint("hi");
		kprint_uint64(current_thread->pid);
		for (int i = 0; i < 1000000; i++) {
			asm volatile ("nop");
		}
		//kpass(200);
		// kprintln_uint64(current_thread->frame[2]);
		
	}
}

void gen1() {
	// asm volatile ("sti");
	kprint("gen1: hi from thread ");
	while (1) {
		kprint_uint64(current_thread->pid);
		//kpass(200);
		
	}
}

void init_scheduler() {
	thread_context* new_thread = (thread_context*)kmalloc_byte(sizeof(thread_context));
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
	size_t num_threads = 5;
	for (int i = 1; i < num_threads; i++) {

		if (i%2==0) current_thread->next_thread = create_thread(i, gen);
		if (i%2==1) current_thread->next_thread = create_thread(i, gen);
		//init_thread();

		current_thread = current_thread->next_thread;
	}
	current_thread->next_thread = new_thread;
	current_thread = current_thread->next_thread;//so we start on the 1st (1 indexed thread)
	
	//for (int i = 0; i < num_threads; i++) {
	//	lapic_periodic(100, 72, 0b0011, 0);
	//	current_thread->thread_entry();
	//	current_thread = current_thread->next_thread;
	//}
	//asm volatile("int $67");
	//current_thread = current_thread->next_thread;
	scheduler_loop();

}

void scheduler_loop() {
	while (1) {
		asm volatile ("sti");//this seems kinda wrong to do but it works

		volatile uint32_t* lapic_id = (uint32_t*)(ACPI_MADT->lapic_addr + 0x20);
		volatile uint32_t* lapic_eoi = (uint32_t*)(ACPI_MADT->lapic_addr + 0xb0);
		*lapic_eoi = 0;
		//lapic_periodic(300, 72, 0b0011, 0);

		//switch_thread((uint64_t)current_thread->next_thread->current_rsp);
		if (current_thread->total_run_time == 0) {
			kprintln("HIHIHIHI");
			//switch_thread((uint64_t)current_thread->return_rsp);
			start_thread(current_thread);
			//asm volatile ("int $67");
		}
		else {
			//current_thread = current_thread->next_thread;
			kprintln_uint64(current_thread->current_rsp);

			//switch_thread((uint64_t)current_thread->current_rsp);
			kprintln("BYEBYEBYEBYEBYE");

		}

		current_thread = current_thread->next_thread;
		kpass(500);
	}
}

void scheduler_return() {
	volatile uint32_t* lapic_id = (uint32_t*)(ACPI_MADT->lapic_addr + 0x20);
	volatile uint32_t* lapic_eoi = (uint32_t*)(ACPI_MADT->lapic_addr + 0xb0);
	//*lapic_eoi = 0;
	kprintln("hi");
	current_thread = current_thread->next_thread;
	scheduler_loop();

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

void start_thread(thread_context* thread) {
	//asm volatile ("sti");//this seems kinda wrong to do but it works

	volatile uint32_t* lapic_id = (uint32_t*)(ACPI_MADT->lapic_addr + 0x20);
	volatile uint32_t* lapic_eoi = (uint32_t*)(ACPI_MADT->lapic_addr + 0xb0);
	*lapic_eoi = 0;
	lapic_periodic(200, 72, 0b0011, 0);


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

	uint64_t return_rsp;
	asm volatile ("mov %%rsp, %0 " : "=r"(return_rsp) : );//save current rsp
	uint64_t misaligned_by = 16 - (return_rsp % 16);//calculate misalignment
	current_thread->return_rsp = (uint64_t*)return_rsp;//save current rsp. i'm not actually sure if i need to do this because i can just store it as a normal variable since this is also a variable but just global (which shouldn't make a difference)
	current_thread->misaligned_by = misaligned_by;//save misalignment


	current_thread->total_run_time += 1;
	current_thread->start_time = tsc_read_ns();
	current_thread->last_start_time;
	uint64_t thread_rsp = ((uint64_t)current_thread->stack_base) + THREAD_STACK_SIZE;//we land on the 15999th index (0 indexed)

	asm volatile ("mov %0, %%rsp" : : "r"(thread_rsp - (15 * sizeof(uint64_t))));//HERE we subtract by the number of elements we popped because we're still at the default stack pointer and not the modified one after pushing everything during thread initialization

	asm volatile (//i think this actually works because the rsp gets restored after the function call and it'll still be at 15*8 under the top of the stack
		"pop %r15\n"
		"pop %r14\n"
		"pop %r13\n"
		"pop %r12\n"
		"pop %r11\n"
		"pop %r10\n"
		"pop %r9\n"
		"pop %r8\n"
		"pop %rbp\n"
		"pop %rdi\n"
		"pop %rsi\n"
		"pop %rdx\n"
		"pop %rcx\n"
		"pop %rbx\n"
		"pop %rax\n"
		);

	current_thread->thread_entry();
	// asm volatile ("mov %%rsp, %0 " : "=r"((uint64_t)current_thread->current_rsp) :);
	

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

	asm volatile ("mov %0, %%rsp" : : "r"((uint64_t)current_thread->return_rsp - current_thread->misaligned_by));//load back in aligned rsp
	asm volatile ("add %0, %%rsp" : : "r"((uint64_t)current_thread->misaligned_by));//move rsp back to its original position

	asm volatile (
		"pop %r15\n"
		"pop %r14\n"
		"pop %r13\n"
		"pop %r12\n"
		"pop %r11\n"
		"pop %r10\n"
		"pop %r9\n"
		"pop %r8\n"
		"pop %rbp\n"
		"pop %rdi\n"
		"pop %rsi\n"
		"pop %rdx\n"
		"pop %rcx\n"
		"pop %rbx\n"
		"pop %rax\n"
		);
	// kprintln("bye");



	//kpass(500);//it seems to stall when i kpass() here. it's probably because the sleep handler's the same priority and this one hasn't cleared out yet, but even if i clear the eoi and do it it somehow doesn't work idk

	// kprintln("hi");
	//read tsc; if tsc > last start time + quantum, stop thread and return
	//*lapic_eoi = 0;
}

void init_thread() {
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

	uint64_t return_rsp;
	asm volatile ("mov %%rsp, %0 " : "=r"(return_rsp) : );//save current rsp
	uint64_t misaligned_by = 16 - (return_rsp % 16);//calculate misalignment
	current_thread->return_rsp = (uint64_t*)return_rsp;//save current rsp. i'm not actually sure if i need to do this because i can just store it as a normal variable since this is also a variable but just global (which shouldn't make a difference)
	current_thread->misaligned_by = misaligned_by;//save misalignment


	current_thread->total_run_time;
	current_thread->start_time = tsc_read_ns();
	current_thread->last_start_time;
	uint64_t thread_rsp = ((uint64_t)current_thread->stack_base) + THREAD_STACK_SIZE;//we land on the 15999th index (0 indexed)

	asm volatile ("mov %0, %%rsp" : : "r"(thread_rsp - (15 * sizeof(uint64_t))));//HERE we subtract by the number of elements we popped because we're still at the default stack pointer and not the modified one after pushing everything during thread initialization

	asm volatile (//i think this actually works because the rsp gets restored after the function call and it'll still be at 15*8 under the top of the stack
		"pop %r15\n"
		"pop %r14\n"
		"pop %r13\n"
		"pop %r12\n"
		"pop %r11\n"
		"pop %r10\n"
		"pop %r9\n"
		"pop %r8\n"
		"pop %rbp\n"
		"pop %rdi\n"
		"pop %rsi\n"
		"pop %rdx\n"
		"pop %rcx\n"
		"pop %rbx\n"
		"pop %rax\n"
		);

	//current_thread->thread_entry();
	// asm volatile ("mov %%rsp, %0 " : "=r"((uint64_t)current_thread->current_rsp) :);


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

	asm volatile ("mov %0, %%rsp" : : "r"((uint64_t)current_thread->return_rsp - current_thread->misaligned_by));//load back in aligned rsp
	asm volatile ("add %0, %%rsp" : : "r"((uint64_t)current_thread->misaligned_by));//move rsp back to its original position

	asm volatile (
		"pop %r15\n"
		"pop %r14\n"
		"pop %r13\n"
		"pop %r12\n"
		"pop %r11\n"
		"pop %r10\n"
		"pop %r9\n"
		"pop %r8\n"
		"pop %rbp\n"
		"pop %rdi\n"
		"pop %rsi\n"
		"pop %rdx\n"
		"pop %rcx\n"
		"pop %rbx\n"
		"pop %rax\n"
		);
}