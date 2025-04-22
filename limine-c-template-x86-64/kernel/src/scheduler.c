#include <scheduler.h>



//thread_context* current_thread;

void gen0() {
	kprintf("gen0: hi from thread %d\n", get_current_thread()->pid);
	// kprintf("gen0: hi from thread %lld", current_thread->pid);
	// kpass(1000);
	// return;
	while (1){
		int a = 0;
		kprint("hi");
		//kprint_uint64(current_thread->pid);
	}
}

void gen1() {
	// asm volatile ("sti");
	kprintf("gen1: hi from thread %d\n", get_current_thread()->pid);
	// return;
	for (int j = 0; j < 500; j++) {
		kprint("bye");
	}
}

thread_context* ready_queue; // typically linked list for round robin scheduler
thread_context* ready_queue_head;
thread_context* ready_queue_end;
thread_context* ready_queue_second_last;


extern void move_two(int**);
void test_move_two() {
    int x = 0;
    int* y = &x;
    move_two(&y);
    kprint("y is: ");
	kprintf("%px", y);
    kprint("\n");
}

void init_scheduler() {

	size_t num_threads = 100;

	thread_context* current_thread;

	thread_context* new_thread = create_thread(0, gen0);
	new_thread->next_thread = NULL;
	current_thread = new_thread;
	for (int i = 1; i < num_threads; i++) {//remember to use 1 because we start from the 2nd thread
		if (i % 2 == 0) current_thread->next_thread = create_thread(i, gen0);
		else current_thread->next_thread = create_thread(i, gen1);
		if (num_threads - i > 1) current_thread = current_thread->next_thread;
	}

	ready_queue_second_last = current_thread;
	current_thread = current_thread->next_thread;

	ready_queue_end = current_thread;

	current_thread = new_thread;
	ready_queue_head = new_thread;

	while (1) reschedule();


}

thread_context* pop_front(thread_context* thread) {
	thread_context* head = ready_queue_head;
	ready_queue_head = ready_queue_head->next_thread;
	return head;//i think this works?? hopefully it just copies the memory over instead of having it get changed because ready_queue_head got changed the next line
}

void push_back(thread_context* ready_queue, thread_context* thread) {
	ready_queue_end->next_thread = thread;
	ready_queue_second_last = ready_queue_end;
	ready_queue_end = ready_queue_end->next_thread;
}

thread_context* get_current_thread() {
	return ready_queue_head;
}

void disable_preemption()
{	
	kprintf("hi");
	volatile uint32_t* lapic_timer = (uint32_t*)(ACPI_MADT->lapic_addr + 0x320);
*lapic_timer |= (1 << 16); // Set the mask bit
	asm volatile ("cli");
	// volatile uint32_t* lapic_id = (uint32_t*)(ACPI_MADT->lapic_addr + 0x20);
	// volatile uint32_t* lapic_eoi = (uint32_t*)(ACPI_MADT->lapic_addr + 0xb0);
	// *lapic_eoi = 0;

}
void enable_preemption()
{
	volatile uint32_t* lapic_timer = (uint32_t*)(ACPI_MADT->lapic_addr + 0x320);
	*lapic_timer &= ~(1 << 16); // Clear the mask bit
	asm volatile ("sti");
}

//thanks to mishakov for the code outline
uint8_t first = 0;
void reschedule() {
	// lapic_oneshot(0, 72, 0b0011, 1);
	disable_preemption();
	thread_context* current_thread;
	
	if (first == 0) {//i could probably simplify this..
		first = 1;
		current_thread = pop_front(ready_queue);
		assert(current_thread);
		uint64_t* a = kmalloc_byte(256);//placeholder
		if (!current_thread)
			goto end;
		kprintf("%d\n", current_thread->pid);

		push_back(ready_queue, current_thread);//&ready_queue
		change_tss(&tss, current_thread->stack_base);
		// enable_preemption();
		lapic_oneshot(THREAD_QUANTUM, 72, 0b0011, 0);//HERE REMEMBER TO USE 0b0011 INSTEAD OF 16
		switch_thread(&a, current_thread->current_rsp);
		kfree(a);
		// enable_preemption();
		return;
	}
	else {
		current_thread = get_current_thread();
	}

	thread_context* next_thread = pop_front(ready_queue);//&ready_queue
	assert(next_thread);
	if (!next_thread)
		goto end;
	// enable_preemption();
	change_tss(&tss, next_thread->stack_base);
	kprintf("%d\n%d\n", ready_queue_second_last->pid, next_thread->pid);
	// enable_preemption();

	lapic_oneshot(THREAD_QUANTUM, 72, 0b0011, 0);//HERE REMEMBER TO USE 0b0011 INSTEAD OF 16
	switch_thread(&ready_queue_second_last->current_rsp, next_thread->current_rsp);

	end:
	// enable_preemption();
}

void scheduler_return() {//basically pthread_exit
	disable_preemption();
	// lapic_oneshot(0, 64, 0b0011, 1);
	volatile uint32_t* lapic_id = (uint32_t*)(ACPI_MADT->lapic_addr + 0x20);
	volatile uint32_t* lapic_eoi = (uint32_t*)(ACPI_MADT->lapic_addr + 0xb0);
	*lapic_eoi = 0;
	thread_context* temp = ready_queue_second_last;
	//add lock thing here
	// disable_preemption();
	kfree(ready_queue_second_last->stack_base);
	kfree(temp);
	// enable_preemption();
	ready_queue_second_last = ready_queue_end;
	kprintf("\nexited thread\n");
	reschedule();
	kprintf("error\n");

}

thread_context* create_thread(uint64_t pid, void (*thread_entry)(void)) {
	//add lock thing here
	disable_preemption();
	// kmalloc_byte(4096);
	uint64_t* thread_base = kmalloc_byte(sizeof(uint64_t) * 2000);//16kb
	// kmalloc_byte(4096);
	thread_context* new_thread = (thread_context*) kmalloc_byte(sizeof(thread_context));
	// kmalloc_byte(4096);
	// enable_preemption();

	new_thread->start_time = tsc_read_ns();
	new_thread->last_start_time = 0;
	new_thread->total_run_time = 0;
	new_thread->quantum_ns = 10000000;//10ms
	new_thread->pid = pid;
	new_thread->thread_entry = thread_entry;
	new_thread->return_rsp = NULL;
	new_thread->misaligned_by = 0;
	new_thread->current_rsp = NULL;
	new_thread->current_misaligned_by = 0;
	// new_thread->rip = NULL;
	new_thread->stack_base = thread_base;
	new_thread->next_thread = NULL;

    uint64_t* thread_rsp = new_thread->stack_base + THREAD_STACK_SIZE;//i'm not actually sure if kmalloc is supposed to return an address that's been casted to a pointer. either way, this reverts it so it should be okay for now i think
	new_thread->current_rsp = thread_rsp;
	// map_page((uint64_t*) (pml4_address_virt_glob), (uint64_t)thread_entry, (uint64_t)thread_entry, 0b11);
	start_thread(&new_thread->current_rsp, thread_entry);
	return new_thread;
}

void start_thread(uint64_t **sp, void *entry) {//thread_entry runs and then scheduler_return runs. the function never actually exits or something idk
	*sp -= 1;//apparently it moves it by 8 bytes for each index
	**sp = (uint64_t) scheduler_return;//basically pthread_exit i think
	*sp -= 1;
	**sp = (uint64_t) disable_preemption;
	*sp -= 1;
	**sp = (uint64_t) entry;
	*sp -= 1;
	**sp = (uint64_t) enable_preemption;
	*sp -= 6;
}