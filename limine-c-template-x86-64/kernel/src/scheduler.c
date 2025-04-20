#include <scheduler.h>



//thread_context* current_thread;

void gen() {
	kprint("gen0: hi from thread\n");
	// kprintf("gen0: hi from thread %lld", current_thread->pid);
	// kpass(1000);
	// return;
	while (1){
		int a = 0;
		kprint("hi");
		//kprint_uint64(current_thread->pid);
		for (int i = 0; i < 1000000; i++) {
			asm volatile ("nop");
		}
	}
}

void gen1() {
	// asm volatile ("sti");
	kprint("gen1: hi from thread\n");
	// return;
	for (int i = 0; i<100; i++) {
		for (int i = 0; i < 1000000; i++) {
			asm volatile ("nop");
		}
		kprint("m");
	}
}
// void start_thread_other(unsigned long** new_thread->current_rsp, void* entry) {
// 	*new_thread->current_rsp -= 2; // 2 because of stack alignment
// 	**new_thread->current_rsp = (unsigned long)entry;
// 	*new_thread->current_rsp -= 6;
// }

thread_context* ready_queue_head;
thread_context* ready_queue_end;

void init_scheduler() {

	thread_context* current_thread;

	thread_context* new_thread = create_thread(0, gen);
	new_thread->next_thread = NULL;
	// kprintln("hi");
	current_thread = new_thread;
	size_t num_threads = 4;
	current_thread->next_thread = create_thread(1, gen);
	current_thread = current_thread->next_thread;
	current_thread->next_thread = create_thread(2, gen1);
	current_thread = current_thread->next_thread;
	current_thread->next_thread = create_thread(3, gen1);
	current_thread = current_thread->next_thread;
	// current_thread->next_thread = new_thread;

	ready_queue_end = current_thread;

	// current_thread = current_thread->next_thread;

	//current_thread is basically the head??
	
	char* s = "helloworld";
	uint64_t i = 2;
	// kprintf("%s %lld\n", s, i);
	// kprintf("%lld %s %s %lld\n", i, s, s, i);
	// while(1){}
	// while (1) {
	// 	kprint(__FILE__);
	// 	kprint(": ");
	// 	kprintln_uint64(__LINE__);
	// 	current_thread = current_thread->next_thread;
	// 	kprintln_uint64(current_thread->pid);
	// }



	// uint64_t* a = kmalloc_byte(20000*sizeof(uint64_t));
	// for (int i = 0; i < 20000; i++) {
	// 	a[i] = i;
	// 	kprintf("%lld ", a[i]);
	// }
	

	current_thread = new_thread;
	ready_queue_head = new_thread;

	// while (1) {
	// 	kprintf("%lld\n", current_thread->pid);
	// 	current_thread = current_thread->next_thread;
	// }

	// pop_front(ready_queue_head);
	while (1) reschedule();


}


thread_context* ready_queue; // typically linked list for round robin scheduler

// void change_tss();

thread_context* pop_front(thread_context* thread) {
	thread_context* head = ready_queue_head;
	ready_queue_head = ready_queue_head->next_thread;
	return head;//i think this works?? hopefully it just copies the memory over instead of having it get changed because ready_queue_head got changed the next line
}

void push_back(thread_context* ready_queue, thread_context* thread) {
	ready_queue_end->next_thread = thread;
	ready_queue_end = ready_queue_end->next_thread;
}

thread_context* get_current_thread() {
	return ready_queue_head;
}

void disable_preemption()
{
	asm volatile ("cli");
}
void enable_preemption()
{
	asm volatile ("sti");
}

//thanks to mishakov for the code outline
thread_context* current_thread_actual;
uint8_t first = 0;
void reschedule() {
	disable_preemption();
	thread_context* current_thread;
	
	if (first == 0) {//i could probably simplify this..
		first = 1;
		current_thread = pop_front(ready_queue);
		kprintf("%lld\n", current_thread->pid);
		uint64_t* a;//placeholder
		// thread_context* next_thread = pop_front(ready_queue);//&ready_queue
		if (!current_thread)
			goto end;
		push_back(ready_queue, current_thread);//&ready_queue
		// change_tss(&tss, current_thread->current_rsp);
		enable_preemption();
		lapic_oneshot(200, 72, 16, 0);
		switch_thread(&a, current_thread->current_rsp);
		disable_preemption();
		return;
	}
	else {
		// current_thread = pop_front(ready_queue);
		current_thread = get_current_thread();
	}

	thread_context* next_thread = pop_front(ready_queue);//&ready_queue
	if (!next_thread)
		goto end;
	// current_thread_actual = get_current_thread();
	kprintf("%lld\n", current_thread->pid);
	uint64_t* a = kmalloc_byte(8);
	push_back(ready_queue, current_thread);//&ready_queue
	// change_tss(&tss, current_thread->current_rsp);
	enable_preemption();
	lapic_oneshot(200, 72, 16, 0);
	switch_thread(&current_thread, next_thread->current_rsp);
	disable_preemption();

	end:
	enable_preemption();
}

void scheduler_return() {
	volatile uint32_t* lapic_id = (uint32_t*)(ACPI_MADT->lapic_addr + 0x20);
	volatile uint32_t* lapic_eoi = (uint32_t*)(ACPI_MADT->lapic_addr + 0xb0);
	//*lapic_eoi = 0;
	kprintln("exited thread!");
	// scheduler_loop();
	// current_thread_actual = current_thread_actual->next_thread;
	reschedule();

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
	new_thread->current_rsp = NULL;
	new_thread->current_misaligned_by = 0;
	// new_thread->rip = NULL;
	new_thread->stack_base = thread_base;
	new_thread->next_thread = NULL;

    uint64_t* thread_rsp = new_thread->stack_base + THREAD_STACK_SIZE;//i'm not actually sure if kmalloc is supposed to return an address that's been casted to a pointer. either way, this reverts it so it should be okay for now i think
	new_thread->current_rsp = thread_rsp;
	// map_page((uint64_t*) (pml4_address_virt_glob), (uint64_t)thread_entry, (uint64_t)thread_entry, 0b11);
	start_thread(&new_thread->current_rsp, thread_entry);
	// new_thread->current_rsp;
	// kprintln("hihi");
	return new_thread;
}

void start_thread(uint64_t **sp, void *entry) {//thread_entry runs and then scheduler_return runs. the function never actually exits or something idk
	*sp -= 1;//apparently it moves it by 8 bytes for each index
	**sp = (uint64_t) scheduler_return;//basically pthread_exit i think
	*sp -= 1;
	**sp = (uint64_t) entry;
	*sp -= 6;
}