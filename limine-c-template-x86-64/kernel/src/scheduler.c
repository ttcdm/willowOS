#include <scheduler.h>

//HERE ONLY USE "THREAD SAFE" FUNCTIONS. DO NOT USE FUNCTIONS THAT STI DURING SECTIONS THAT ARE CLI'D

//thread_context* current_thread;

void gen0() {
	kprintf("gen0: hi from thread %d\n", get_current_thread()->pid);
	// kprintf("gen0: hi from thread %lld", current_thread->pid);
	// kpass(1000);
	// return;
	while (1){
		int a = 0;
		// kprint("hi");
		// for (int i = 0; i < 30; i++) kprintf("hi");
		kprintf("hi");
		// yield_thread();
		// kprint("hi");
		// kprintf("hi");
		// kprintf("gen0: hi from thread %d\n", get_current_thread()->pid);
		// kprint("hi");
		// kprintf("gen0: hi from thread %d\n", get_current_thread()->pid);
		//kprint_uint64(current_thread->pid);
	}
}

void gen1() {
	// asm volatile ("sti");
	kprintf("gen1: hi from thread %d\n", get_current_thread()->pid);
	// volatile thread_context* a = block_thread(0);
	// return;
	for (int j = 0; j < 1000; j++) {
		int a = 0;
		// kprint("bye");
		kprintf("bye");
		// yield_thread();
	}

	// if (a != NULL) unblock_thread(a);

}

volatile thread_context* ready_queue; // typically linked list for round robin scheduler
volatile thread_context* ready_queue_head;
volatile thread_context* ready_queue_end;
volatile thread_context* ready_queue_second_last;
volatile thread_context** current_actual;

void init_scheduler() {

	size_t num_threads = 200;

	// volatile thread_context* current_thread;

	// volatile thread_context* new_thread = create_thread(0, gen1);
	// new_thread->next_thread = NULL;
	// current_thread = new_thread;
	// current_thread->next_thread = create_thread(100, gen0);
	// current_thread = current_thread->next_thread;
	// for (int i = 1; i < num_threads; i++) {//i should probably recycle pid's. remember to use 1 because we start from the 2nd thread
	// 	if (i % 3 == 0) current_thread->next_thread = create_thread(i, gen0);
	// 	else current_thread->next_thread = create_thread(i, gen1);
	// 	if (num_threads - i > 1) current_thread = current_thread->next_thread;
	// }

	// ready_queue_second_last = current_thread;
	// // current_actual = &current_thread;
	// current_thread = current_thread->next_thread;

	// ready_queue_end = current_thread;

	// current_thread = new_thread;
	// ready_queue_head = new_thread;


	// push_thread(create_thread(0, gen1));
	// push_thread(create_thread(1, gen1));

	for (int i = 0; i < num_threads; i++) {
		if (i % 3 == 0) push_thread(create_thread(i, gen0));
		else push_thread(create_thread(i, gen1));
	}


	while (1) reschedule();

	

}

thread_context* pop_front(thread_context* thread) {
	volatile thread_context* head = ready_queue_head;
	ready_queue_head = ready_queue_head->next_thread;
	return head;//i think this works?? hopefully it just copies the memory over instead of having it get changed because ready_queue_head got changed the next line
}

void push_back(thread_context* ready_queue, thread_context* thread) {
	ready_queue_end->next_thread = thread;
	ready_queue_second_last = ready_queue_end;
	current_actual = &ready_queue_end;
	ready_queue_end = ready_queue_end->next_thread;
}

thread_context* get_current_thread() {
	return ready_queue_head;
	// return *current_actual;
}

void disable_preemption()
{	
	// kprintf("\0");
	// for (int i = 0; i < 100000; i++) {
	// 	asm volatile ("nop");
	// }
// 	volatile uint32_t* lapic_timer = (uint32_t*)(ACPI_MADT->lapic_addr + 0x320);
// *lapic_timer |= (1 << 16); // Set the mask bit
	asm volatile ("cli");
	// volatile uint32_t* lapic_id = (uint32_t*)(ACPI_MADT->lapic_addr + 0x20);
	// volatile uint32_t* lapic_eoi = (uint32_t*)(ACPI_MADT->lapic_addr + 0xb0);
	// *lapic_eoi = 0;

}
void enable_preemption()
{
	// volatile uint32_t* lapic_timer = (uint32_t*)(ACPI_MADT->lapic_addr + 0x320);
	// *lapic_timer &= ~(1 << 16); // Clear the mask bit
	asm volatile ("sti");
}

//thanks to mishakov for the code outline
uint8_t first = 0;
void reschedule() {
	// lapic_oneshot(0, 72, 0b0011, 1);
	// disable_preemption();
	asm volatile ("cli");
	volatile thread_context* current_thread;
	
	if (first == 0) {//i could probably simplify this..
		first = 1;
		current_thread = pop_front(ready_queue);
		// assert(current_thread);
		uint64_t* a;// = kmalloc_byte(256);//placeholder
		// assert(current_thread);
		// uint64_t* a;// = kmalloc_byte(256);//placeholder
		if (!current_thread) {
			kprintf("no more threads");
			while (1) {asm volatile ("cli; hlt");}
			// goto end;
		}
		// kprintf("%d\n", current_thread->pid);
		// kprintf("%d\n", current_thread->pid);

		push_back(ready_queue, current_thread);//&ready_queue
		change_tss(&tss, current_thread->stack_base);
		// enable_preemption();
		lapic_oneshot(THREAD_QUANTUM, 72, 0b0011, 0);//HERE REMEMBER TO USE 0b0011 INSTEAD OF 16
		switch_thread(&a, current_thread->current_rsp);
		// kfree_interruptable(thread_context* a));
		// kfree_interruptable(a);
		// enable_preemption();
		return;
	}
	else {
		current_thread = get_current_thread();
		// *current_actual = (*current_actual)->next_thread;
	}

	volatile thread_context* next_thread = pop_front(ready_queue);//&ready_queue
	// assert(next_thread);
	if (!next_thread) {
		kprintf_interruptable("error no more threads");
		while (1) {asm volatile ("cli; hlt");}
	// assert(next_thread);
	}
	if (next_thread->pid == ready_queue_second_last->pid) {
		// kprintf("1 thread left");
		return;//don't switch just return
	}

	if (next_thread->status[3] == 1) {
        // push_back(ready_queue, current_thread);//&ready_queue
		if (next_thread->next_thread == NULL) {
			kprintf_interruptable("hihihi");
			while (1);
			// return;
		}
		else {
			next_thread = next_thread->next_thread;
		}
    }

	// enable_preemption();
	change_tss(&tss, next_thread->stack_base);
	// kprintf("%d\n%d\n", ready_queue_second_last->pid, next_thread->pid);
	// kprintf("%d\n%d\n", ready_queue_second_last->pid, next_thread->pid);

	// while (next_thread->frame[0] == 0) {
	// 	next_thread = pop_front(ready_queue);//not = next_thread->next_thread because we need to change ready_queue_head as well
	// }


	kprintf_interruptable("\nswitching from thread %d to thread %d at reschedule\n", ready_queue_second_last->pid, next_thread->pid);

	// volatile uint32_t* lapic_irr = (uint32_t*)(ACPI_MADT->lapic_addr + 0x220);//HERE technically this isn't needed because you can't queue irq 72 again if it's already been queued
	// if (*(lapic_irr) >> 8 & 1) {
	// 	volatile uint32_t* lapic_id = (uint32_t*)(ACPI_MADT->lapic_addr + 0x20);
	// 	volatile uint32_t* lapic_eoi = (uint32_t*)(ACPI_MADT->lapic_addr + 0xb0);
	// 	*lapic_eoi = 0;
	// 	switch_thread(&ready_queue_second_last->current_rsp, next_thread->current_rsp);
	// }
	// else {
		lapic_oneshot(THREAD_QUANTUM, 72, 0b0011, 0);//HERE REMEMBER TO USE 0b0011 INSTEAD OF 16
		switch_thread(&ready_queue_second_last->current_rsp, next_thread->current_rsp);
	// }
	end:
	// enable_preemption();
}

void scheduler_return() {//basically pthread_exit
	// disable_preemption();
	asm volatile ("cli");

	// kprintf("\nexited thread\n");
	// kprintf("\nexited thread\n");
	// lapic_oneshot(0, 64, 0b0011, 1);
	volatile uint32_t* lapic_id = (uint32_t*)(ACPI_MADT->lapic_addr + 0x20);
	volatile uint32_t* lapic_eoi = (uint32_t*)(ACPI_MADT->lapic_addr + 0xb0);
	*lapic_eoi = 0;
	volatile thread_context* temp = ready_queue_second_last;//HERE not sure if i'm supposed to do double pointer or just copy it
	//add lock thing here
	// disable_preemption();
	kfree_interruptable(temp->stack_base);
	kfree_interruptable((uint64_t*) temp);
	// enable_preemption();
	ready_queue_second_last = ready_queue_end;
	volatile thread_context* next_thread = pop_front(ready_queue);
	if (!next_thread) {
		kprintf("no more threads");
		while (1) {asm volatile ("cli; hlt");}
		// goto end;
	}
	if (next_thread->pid == ready_queue_second_last->pid) {
		kprintf("no more threads to schedule");
		while (1);
	}
	change_tss(&tss, next_thread->stack_base);

	// volatile thread_context* a = (thread_context*) kmalloc_byte(64);
	uint64_t* a;
	kprintf_interruptable("\nthread exited!\nswitching from thread %d to thread %d at return\n", ready_queue_second_last->pid, next_thread->pid);

	// volatile uint32_t* lapic_irr = (uint32_t*)(ACPI_MADT->lapic_addr + 0x220);//HERE technically this isn't needed because you can't queue irq 72 again if it's already been queued
	// if (*(lapic_irr) >> 8 & 1) {
	// 	volatile uint32_t* lapic_id = (uint32_t*)(ACPI_MADT->lapic_addr + 0x20);
	// 	volatile uint32_t* lapic_eoi = (uint32_t*)(ACPI_MADT->lapic_addr + 0xb0);
	// 	*lapic_eoi = 0;
	// 	switch_thread(&a, next_thread->current_rsp);
	// }
	// else {
		lapic_oneshot(THREAD_QUANTUM, 72, 0b0011, 0);//HERE REMEMBER TO USE 0b0011 INSTEAD OF 16
		switch_thread(&a, next_thread->current_rsp);
	// }
	// reschedule();
	// while(1);
	// kprintf("error\n");

}

thread_context* create_thread(uint64_t pid, void (*thread_entry)(void)) {
	//add lock thing here
	disable_preemption();
	// kmalloc_byte(4096);
	volatile uint64_t* thread_base = kmalloc_byte(sizeof(uint64_t) * 2000);//16kb
	// kmalloc_byte(4096);
	volatile thread_context* new_thread = (thread_context*) kmalloc_byte(sizeof(thread_context));
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

    volatile uint64_t* thread_rsp = new_thread->stack_base + THREAD_STACK_SIZE;//i'm not actually sure if kmalloc is supposed to return an address that's been casted to a pointer. either way, this reverts it so it should be okay for now i think
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


bool first_thread = 1;
void push_thread(thread_context* thread) {
	if (first_thread) {
		ready_queue_head = thread;
		ready_queue_second_last = ready_queue_head;
		ready_queue_end = ready_queue_head;
		push_back(ready_queue, ready_queue_head);
		first_thread = 0;
	}
	else {
		push_back(ready_queue, thread);
	}
}

thread_context* block_thread(uint64_t pid) {
	asm volatile ("cli");
	volatile thread_context* current_thread = ready_queue_head;
	while (current_thread) {
		asm volatile ("cli");
		if (current_thread->pid == pid) {
			if (current_thread->status[3] == 1) {
				kprintf_interruptable("thread %d is already blocked", pid);
				asm volatile ("sti");
				return NULL;
			}
			current_thread->status[3] == 1;
			kprintf_interruptable("blocked thread %d", pid);
			// while (1);
			if (pid == get_current_thread()->pid) {
				asm volatile ("int $72");//not sure if it should do this before returning if it's trying to block itself, but i'm not sure if it matters either
			}
			asm volatile ("sti");
			return current_thread;
		}
		current_thread = current_thread->next_thread;
	}
	kprintf_interruptable("thread %d not found", pid);
	asm volatile ("sti");
	return NULL;//might run into issues with it being a null pointer
}

void unblock_thread(thread_context* thread) {
	asm volatile ("cli");
	if (thread->status[3] == 1) {
		thread->status[3] = 0;
		push_back(ready_queue, thread);//not sure if it's supposed to run immediately or just put it back onto the queue
		kprintf_interruptable("unblocked thread %d", thread->pid);
	}
	else {
		kprintf_interruptable("thread %d is not blocked", thread->pid);
	}
	asm volatile ("sti");
}

void yield_thread() {
	asm volatile ("cli");
	volatile thread_context* current_thread = get_current_thread();
	kprintf_interruptable("%d", current_thread->pid);
	push_back(ready_queue, current_thread);//maybe add a check for if it's blocked
	reschedule();
	// asm volatile ("sti");//technically no need for sti because switch_thread() inside reschedule already sti's and it eventually gets back here i think
}

thread_context* get_thread_by_pid(uint64_t pid) {
	volatile thread_context* current_thread = ready_queue_head;
	while (current_thread) {
		if (current_thread->pid == pid) {
			return current_thread;
		}
		current_thread = current_thread->next_thread;
	}
	kprintf_interruptable("thread %d not found", pid);
	return NULL;
}