#include <scheduler.h>
#include <loader.h>

//HERE ONLY USE "THREAD SAFE" FUNCTIONS. DO NOT USE FUNCTIONS THAT STI DURING SECTIONS THAT ARE CLI'D

//thread_context* current_thread;

void gen0() {
	kprintf("gen0: hi from thread %d\n", get_current_thread()->pid);
	// kprintf("gen0: hi from thread %lld", current_thread->pid);
	// kpass(1000);
	// return;
	kprintf("hi\n");
	sleep_thread(0, 3000);
	kprintf("bye");
	scheduler_return();
	while (1){
		int a = 0;
		a++; a--;
		// kprint("hi");
		// for (int i = 0; i < 30; i++) kprintf("hi");
		// kprintf("hi");
		kprintf("%d", get_current_thread()->pid);
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
	asm volatile ("cli");
	// asm volatile ("sti");
	kprintf("gen1: hi from thread %d\n", get_current_thread()->pid);
	volatile thread_context* a = block_thread(1);
	a->start_time--; a->start_time++;//to make gcc happy about unused variable
	// while (1);
	// return;
	// scheduler_return();
	for (int j = 0; j < 500; j++) {
		int a = 0;
		a++; a--;
		// kprint("bye");
		kprintf("bye");
		// yield_thread();
	}

	// if (a != NULL) unblock_thread(a);

}

int aaa = 0;
void gen2() {
	// kprintf_interruptable("hi");
	aaa += 2;
	// aaa = 2;
	int c;
	c++;
	if (c == 1) c = 0;
	if (aaa == 2) hot_create_and_push_thread(5, gen2);
	//no need to call reschedule nor hot_reschedule after because of race condition and you just set the lapic timer instead
	// if (aaa == 2) hot_create_and_push_user_thread(running_thread->pid+1000, test_a);

	// while (1) {test_b();}

	while (1) {}//kprintf("gen2: hi from thread %d\n", get_current_thread()->pid); }
}

void idle_thread() {
	while (1) {
		asm volatile ("pause");
	}
}

volatile thread_context* ready_queue; // typically linked list for round robin scheduler
volatile thread_context* ready_queue_head;
volatile thread_context* ready_queue_end;
volatile thread_context* ready_queue_second_last;
// volatile thread_context** current_actual;
volatile thread_context* running_thread;

void init_scheduler() {
	push_thread(create_thread(1500, gen2));
	// push_thread(create_thread(3, gen1));

	lapic_periodic(5, 80, 0b0011, 0);
	while (1) reschedule();
}

volatile thread_context* pop_front(volatile thread_context* thread) {
	// thread->start_time++; thread->start_time--;//to make gcc happy about unused parameter

	volatile thread_context* head = ready_queue_head;
	if (ready_queue_head->next_thread == NULL) {
		kprintf_interruptable("no more threads\n");
		return NULL;
	}
	ready_queue_head = ready_queue_head->next_thread;
	return head;//i think this works?? hopefully it just copies the memory over instead of having it get changed because ready_queue_head got changed the next line
}

void push_back(volatile thread_context* ready_queue, volatile thread_context* thread) {
	ready_queue_end->next_thread = thread;
	ready_queue_second_last = ready_queue_end;
	// current_actual = &ready_queue_end;
	ready_queue_end = ready_queue_end->next_thread;
}

uint64_t create_new_userspace_page_table() {//returns the virtual address
	//REMEMBER TO FREE THE FRAME
	uint64_t* cr3 = (uint64_t*) (alloc_frame() + hhdm_offset);

	for (int i = 0; i < 512; i++) {
		cr3[i] = 0;
	}

	for (int i = 256; i < 512; i++) {
		cr3[i] = ((uint64_t*)(pml4_address_virt_glob))[i];
	}
	
	// //HERE MAKE SURE CR3 IS 4KIB ALIGNED
	// asm volatile ("mov %0, %%cr3" :: "r"((uint64_t) new_thread->cr3 - hhdm_offset));// we don't switch here

	return ((uint64_t) cr3);
}

void hot_create_and_push_thread(uint64_t pid, void (*thread_entry)(void)) {
	asm volatile ("cli");
	volatile thread_context* t = create_thread(pid, thread_entry);
	uint64_t new_cr3 = create_new_userspace_page_table();
	t->cr3 = (uint64_t*) new_cr3;
	push_thread(t);
	asm volatile ("sti");
}

void hot_exec_elf(uint64_t pid, void* file) {//HERE remember to add some sort of free_elf functions as well to free all the physical frames
	// asm volatile ("cli");
	bool irq;
	irq_disable_save(&irq);

	//HERE
	//one workaround is we pass in the elf entry into create thread, and we run load elf but with different args i guess after
	//we can also just not create a new page table inside create_thread() and instead do all of that here with a function call and assign cr3 inside create_thread to it
	
	uint64_t current_cr3 = (uint64_t) get_cr3();
	uint64_t new_cr3 = create_new_userspace_page_table();
	// new_cr3 -= hhdm_offset;
	asm volatile ("mov %0, %%cr3" :: "r"(new_cr3 - hhdm_offset));
	volatile thread_context* t = create_thread(pid, userspace_run_elf);
    t->elf_entry = load_elf(file, new_cr3);
	t->cr3 = (uint64_t*) (new_cr3);
    push_thread(t);
	asm volatile ("mov %0, %%cr3" :: "r"(current_cr3));
    // reschedule();//not sure if i should add reschedule here
	
	
	irq_restore(&irq);
	// asm volatile ("sti");
}

void hot_create_and_push_user_thread(uint64_t pid, void (*thread_entry)(void)) {
	bool irq;
	irq_disable_save(&irq);

	volatile thread_context* t = create_thread(pid, userspace_run_elf);
	uint64_t new_cr3 = create_new_userspace_page_table();
	t->elf_entry = thread_entry;
	t->cr3 = (uint64_t*) new_cr3;
	push_thread(t);
	irq_restore(&irq);
}

void hot_reschedule() {//HERE must use this to call reschedule instead of just reschedule() itself inside a thread because if you create a thread inside a thread and you don't call reschedule() right after it, it gets preempted and some stuff doesn't get pushed back and the logic breaks, so you must either do both in "atomically", i.e., without getting preempted or just have this after which takes care of it i think
	asm volatile ("cli");
	volatile thread_context* current_thread = get_current_thread();
	if (current_thread->status[3] == 0) {
		push_back(ready_queue, current_thread);
    }
	reschedule();
	asm volatile ("sti");
}

volatile thread_context* get_current_thread() {
	// return ready_queue_head;
	// return *current_actual;
	return running_thread;
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
	asm volatile ("mov %0, %%cr3" :: "r"(((uint64_t) pml4_address_virt_glob) - hhdm_offset));

	//we clear the eoi here because i don't wanna wrap it with a cr3 switch
    volatile uint32_t* lapic_eoi = (uint32_t*) ((uintptr_t)(ACPI_MADT->lapic_addr + 0xb0));
	*lapic_eoi = 0;

	volatile thread_context* current_thread;
	
	if (first == 0) {//i could probably simplify this..
		first = 1;
		current_thread = pop_front(ready_queue);
		// current_thread = get_current_thread();
		// assert(current_thread);
		uint64_t* a;// = kmalloc_byte(256);//placeholder
		// assert(current_thread);
		// uint64_t* a;// = kmalloc_byte(256);//placeholder
		if (!current_thread) {
			kprintf_interruptable("no more threads");
			while (1) {asm volatile ("cli; hlt");}
			// goto end;
		}

		push_back(ready_queue, current_thread);//&ready_queue
		// change_tss(&tss, current_thread->stack_base);
		// enable_preemption();
		running_thread = current_thread;
		running_thread->last_run_time = tsc_read_ns();

		// change_tss(&tss, current_thread->current_rsp);
		// change_tss(tss, (uint64_t*) ((uint64_t) current_thread->stack_base)-THREAD_STACK_SIZE);
		change_tss(tss, current_thread->stack_base);
		// change_tss(tss, current_thread->current_rsp);
		// change_tss(tss, (uint64_t*) (((uint64_t) current_thread->stack_base)-THREAD_STACK_SIZE));

		if (current_thread->status[4] == 1) {
			swap_to_user_gs();
		}


		lapic_oneshot(THREAD_QUANTUM, 72, 0b0011, 0);//HERE REMEMBER TO USE 0b0011 INSTEAD OF 16
		asm volatile ("mov %0, %%cr3" :: "r"(((uint64_t) current_thread->cr3) - hhdm_offset));
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
	if (next_thread->pid == running_thread->pid) {//HERE FIX ME
		// kprintf("1 thread left");
		lapic_oneshot(THREAD_QUANTUM, 72, 0b0011, 0);//idk maybe disregard the stuff after this sentence; i think reschedule() does return so we can call it after creating a thread so we can leave this off i think. i'm not sure if i should leave this on or not. if i leave it off i leave it up to the newly created thread or the user to call reschedule, but then it also means that it can't return back to it because reschedule never returns or something idk, so leaving it on would force the isr to reschedule instead of the function so it doesn't break anything i guess?? but i'm also not 100% sure that reschedule returns or not, as yield_thread() calls it and idk if it returns??? i don't think i actually need this. because if i do end up adding another thread when there's only 1 left, next_thread will be different. HERE REMEMBER TO USE 0b0011 INSTEAD OF 16
		// asm volatile ("sti");//need to reenable it because we don't have switch thread which reenables it

		// kprintf_interruptable("byebye");
		asm volatile ("mov %0, %%cr3" :: "r"(((uint64_t) running_thread->cr3) - hhdm_offset));//ALWAYS REMEMBER TO SWAP THE CORRECT CR3 BACK IN
		return;//don't switch just return
	}

	if (next_thread->status[3] == 1) {
		while (next_thread->status[3] == 1) {//prevents the next thread from being blocked.
			next_thread = pop_front(ready_queue);
			assert(next_thread);
			if (next_thread->pid == 0xDEADBEEFCAFEBABE) {
				kprintf_interruptable("idle thread...");
				lapic_oneshot(THREAD_QUANTUM, 72, 0b0011, 0);
				return;
			}
			kprintf_interruptable("\current thread blocked, switching from thread %d\n", next_thread->pid);
		}
    }

	if (next_thread->status[3] == 1) {
		kprintf_interruptable("\nAAAAAA %d AAAAA\n", next_thread->pid);
		while (1);
	}
	
	if (next_thread->status[3] == 0) {

		ready_queue_second_last = current_thread;
		if (running_thread->pid == next_thread->pid) {
			return;
		}
		kprintf_interruptable("\nswitching from thread %d to thread %d at reschedule\n", ready_queue_second_last->pid, next_thread->pid);
		running_thread = next_thread;
		ready_queue_second_last->last_run_time = tsc_read_ns();
	
		// change_tss(tss, (uint64_t*) (((uint64_t) current_thread->stack_base)-THREAD_STACK_SIZE));
		// change_tss(tss, (uint64_t*) (((uint64_t) next_thread->stack_base)-THREAD_STACK_SIZE));
		change_tss(tss, next_thread->stack_base);
		// change_tss(tss, next_thread->current_rsp);

		if (current_thread->status[4] == 1) {
			swap_to_user_gs();
		}

		lapic_oneshot(THREAD_QUANTUM, 72, 0b0011, 0);//HERE REMEMBER TO USE 0b0011 INSTEAD OF 16
		asm volatile ("mov %0, %%cr3" :: "r"(((uint64_t) next_thread->cr3) - hhdm_offset));
		switch_thread(&ready_queue_second_last->current_rsp, next_thread->current_rsp);
	}

}

void scheduler_return() {//basically pthread_exit
	//HERE remember to figure out if you need a way to return to kernelspace via a syscall something for scheduler_return() to run
	// disable_preemption();
	asm volatile ("cli");
	asm volatile ("mov %0, %%cr3" :: "r"(((uint64_t) pml4_address_virt_glob) - hhdm_offset));


	volatile thread_context* current_thread = get_current_thread();

	// volatile thread_context* temp = ready_queue_second_last;//HERE not sure if i'm supposed to do double pointer or just copy it
	volatile thread_context* temp = current_thread;
	uint64_t temp_pid = temp->pid;
	// ready_queue_second_last->status[3] = 1;
	// temp->status[3] = 1;
	//add lock thing here
	// disable_preemption();

	//HERE disregard the stuff after this sentence; i think because before i was using the top? (or bottom?) and now it's the other way around. remember to free temp->stackbase+THREAD_STACK_SIZE since stack base is at the very top and we allocated from the bottom

	// ready_queue_second_last = current_thread;
	volatile thread_context* next_thread = pop_front(ready_queue);
	if (!next_thread) {
		kprintf_interruptable("no more threads");
		while (1) {asm volatile ("cli; hlt");}
	}

	// ready_queue_second_last->status[3] = 1;//same thing as below


	// uint64_t temp_pid = ready_queue_second_last->pid;
	uint64_t* a;

	// asm volatile ("mov %0, %%cr3" :: "r"(((uint64_t) pml4_address_virt_glob) - hhdm_offset));
	volatile uint32_t* lapic_eoi = (uint32_t*) ((uintptr_t)(ACPI_MADT->lapic_addr + 0xb0));
	*lapic_eoi = 0;

	while (next_thread->status[3] == 1) {//prevents the next thread from being blocked.
		next_thread = pop_front(ready_queue);
		if (temp_pid == next_thread->pid) {

			kfree_interruptable((uint64_t*) (((uint64_t) temp->stack_base)-THREAD_STACK_SIZE));
			free_frame(((uint64_t)temp->cr3) - hhdm_offset);
			kfree_interruptable((uint64_t*) temp);

			kprintf_interruptable("\nno more threads to schedule. switching to idle\n");
			// while (1);
			// ready_queue_second_last->last_run_time = tsc_read_ns();

			hot_create_and_push_thread(0xDEADBEEFCAFEBABE, idle_thread);

			next_thread = pop_front(ready_queue);//not 100% sure if pop_front() will return the created thread from hot_create...()

			if (next_thread->pid != 0xDEADBEEFCAFEBABE) {
				kprintf_interruptable("\nthread %d not found\n", next_thread->pid);
				while (1) asm volatile ("cli; hlt");
			}

			change_tss(tss, next_thread->stack_base);
			lapic_oneshot(THREAD_QUANTUM, 72, 0b0011, 0);//still set a timer because a thread still may somehow get pushed idk?? not sure if it really matters
			asm volatile ("mov %0, %%cr3" :: "r"(((uint64_t) next_thread->cr3) - hhdm_offset));
			switch_thread(&a, next_thread->current_rsp);
		}
		else {
			push_thread(next_thread);//i hope this doesn't break anything since we're just popping the threads off and we should put the threads back on but idk if that affects ready_queue_second_last in a way i'm not supposed to
		}
		assert(next_thread);
	}
	if (next_thread->status[3] == 0) {
	// if (temp_pid == next_thread->pid) {
		// ready_queue_second_last = current_thread;
		// ready_queue_second_last->status[3] = 1;//HERE the logic is weird so we just block the finished thread and let the scheduler handle it. it's not the best fix imo but it works for now. i just hope that it actually gets removed from the queue itself
		// ready_queue_second_last->last_run_time = tsc_read_ns();
		
		
		kfree_interruptable((uint64_t*) (((uint64_t) temp->stack_base)-THREAD_STACK_SIZE));
		free_frame(((uint64_t)temp->cr3) - hhdm_offset);
		temp->next_thread = NULL;
		kfree_interruptable((uint64_t*) temp);
		
		kprintf_interruptable("\nthread exited!\nswitching from thread %d to thread %d at return\n", temp_pid, next_thread->pid);
		running_thread = next_thread;

		change_tss(tss, next_thread->stack_base);

		lapic_oneshot(THREAD_QUANTUM, 72, 0b0011, 0);//still set a timer because a thread may eventually get created and hot_reschedule may not get called
		asm volatile ("mov %0, %%cr3" :: "r"(((uint64_t) next_thread->cr3) - hhdm_offset));
		switch_thread(&a, next_thread->current_rsp);
	}
}

volatile thread_context* create_thread(uint64_t pid, void (*thread_entry)(void)) {
	asm volatile ("cli");
	//add lock thing here
	disable_preemption();

	volatile uint64_t* thread_base = (uint64_t*) (((uint64_t) kmalloc_byte_interruptable(THREAD_STACK_SIZE)) + THREAD_STACK_SIZE);//16kb
	volatile thread_context* new_thread = (thread_context*) kmalloc_byte_interruptable(sizeof(thread_context));//HERE REMEMBER TO ALWAYS DISABLE INTERRUPTS WHEN NECESSARY OR USE THE STATE SAVING FUNCTIONS


	new_thread->start_time;// = tsc_read_ns();
	new_thread->last_start_time = 0;
	new_thread->total_run_time = 0;
	new_thread->last_run_time;// = tsc_read_ns();
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

	new_thread->elf_entry = NULL;
	new_thread->status[4] = 0;


	// while (1);
	//HERE REMEMBER TO CAST TO PREVENT DOING POINTER ARITHMETIC INSTEAD OF JUST NORMAL ARITHEMETIC
    volatile uint64_t* thread_rsp = (uint64_t*) ((uint64_t) new_thread->stack_base);//i'm not actually sure if kmalloc is supposed to return an address that's been casted to a pointer. either way, this reverts it so it should be okay for now i think
	new_thread->current_rsp = thread_rsp;
	// map_page((uint64_t*) (pml4_address_virt_glob), (uint64_t)thread_entry, (uint64_t)thread_entry, 0b11);
	start_thread(&new_thread->current_rsp, thread_entry);
	// new_thread->stack_base -= 4;
	// new_thread->stack_base -= 20;
	kprintf_interruptable("created thread %d\n", pid);
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
	*sp -= 15;
}


bool first_thread = 1;
void push_thread(volatile thread_context* thread) {
	if (first_thread) {
		// thread_context* temp = create_thread(0xDEADBEEFCAFEBABE, gen0);
		running_thread = thread;
		ready_queue_head = thread;
		ready_queue_second_last = ready_queue_end;
		ready_queue_end = ready_queue_head;
		push_back(ready_queue, thread);
		first_thread = 0;
	}
	else {
		push_back(ready_queue, thread);
	}
}

volatile thread_context* block_thread(uint64_t pid) {
	bool irq;
	irq_disable_save(&irq);
	// asm volatile ("cli");
	volatile thread_context* current_thread = ready_queue_head;
	while (current_thread) {
		irq_disable_save(&irq);
		// asm volatile ("cli");
		if (current_thread->pid == pid) {
			if (current_thread->status[3] == 1) {
				kprintf_interruptable("\nthread %d is already blocked\n", pid);
				// asm volatile ("sti");
				irq_restore(&irq);
				return NULL;
			}
			current_thread->status[3] = 1;
			kprintf_interruptable("\nblocked thread %d\n", pid);
			// while (1);
			if (pid == get_current_thread()->pid) {
				asm volatile ("int $72");//not sure if it should do this before returning if it's trying to block itself, but i'm not sure if it matters either
			}
			// asm volatile ("sti");
			irq_restore(&irq);
			return current_thread;
		}
		current_thread = current_thread->next_thread;
	}
	kprintf_interruptable("\nthread %d not found\n", pid);
	// asm volatile ("sti");
	irq_restore(&irq);
	return NULL;//might run into issues with it being a null pointer
}

void unblock_thread(volatile thread_context* thread) {
	// asm volatile ("cli");
	bool irq;
	irq_disable_save(&irq);
	if (thread->status[3] == 1) {
		thread->status[3] = 0;
		push_back(ready_queue, thread);//not sure if it's supposed to run immediately or just put it back onto the queue
		kprintf_interruptable("\nunblocked thread %d\n", thread->pid);
	}
	else {
		kprintf_interruptable("\nthread %d is not blocked\n", thread->pid);
	}
	// asm volatile ("sti");
	irq_restore(&irq);
}

void yield_thread() {
	// asm volatile ("cli");
	bool irq;
	irq_disable_save(&irq);
	volatile thread_context* current_thread = get_current_thread();
	kprintf_interruptable("\nyielding thread %d\n", current_thread->pid);
	push_back(ready_queue, current_thread);//maybe add a check for if it's blocked
	reschedule();
	// asm volatile ("sti");//technically no need for sti because switch_thread() inside reschedule already sti's and it eventually gets back here i think
}

volatile thread_context* sleep_thread(uint64_t pid, uint64_t ms) {//we don't have to use the return value since it automatically gets unblocked but it's just there in case we need it
	// thread_context* thread = block_thread(pid);
	//i don't want to copy the code over but thread blocking should be uninterruptable but it does sti at the end which we can't have here because this should also be uninterruptable
	asm volatile ("cli");
	volatile thread_context* current_thread = ready_queue_head;
	while (current_thread) {
		asm volatile ("cli");
		if (current_thread->pid == pid) {
			if (current_thread->status[3] == 1 || current_thread->status[4] == 1) {
				kprintf_interruptable("\nthread %d is already blocked or sleeping\n", pid);
				asm volatile ("sti");
				return NULL;
			}
			current_thread->status[3] = 1;
			current_thread->last_run_time = tsc_read_ns();
			current_thread->sleep_for_ms = ms;
			current_thread->status[4] = 1;
			kprintf_interruptable("\slept thread %d\n", pid);
			// while (1);
			if (pid == get_current_thread()->pid) {
				//no need to sti here because software interrupts i.e. int 72 does not care about the interrupt flag
				asm volatile ("int $72");//not sure if it should do this before returning if it's trying to block itself, but i'm not sure if it matters either
			}
			asm volatile ("sti");
			return current_thread;
		}
		current_thread = current_thread->next_thread;
	}
	kprintf_interruptable("\nthread %d not found\n", pid);
	asm volatile ("sti");
	return NULL;//might run into issues with it being a null pointer
}

volatile thread_context* get_thread_by_pid(uint64_t pid) {
	// asm volatile ("cli");
	bool irq;
	irq_disable_save(&irq);
	volatile thread_context* current_thread = ready_queue_head;
	while (current_thread) {
		if (current_thread->pid == pid) {
			return current_thread;
		}
		current_thread = current_thread->next_thread;
	}
	kprintf_interruptable("\nthread %d not found\n", pid);
	// asm volatile ("sti");
	irq_restore(&irq);
	return NULL;
}

void print_queue() {
	// asm volatile ("cli");
	bool irq;
	irq_disable_save(&irq);
	volatile thread_context* current_thread = ready_queue_head;
	kprintf_interruptable("\n||| ");
	// while (current_thread) {
	for (int i = 0; i < 3; i++) {
		kprintf_interruptable("%d ", current_thread->pid);
		current_thread = current_thread->next_thread;
	}
	kprintf_interruptable("|||\n");
	// asm volatile ("sti");
	irq_restore(&irq);
}