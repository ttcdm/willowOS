#include <scheduler.h>
#include <loader.h>
#include <mutex.h>

#define OA_HASH_HEADER
#include <./oa_hash/oa_hash.h>

#include <global_vars.h>

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
	if (a != NULL) {a->start_time--; a->start_time++;}//to make gcc happy about unused variable
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
	return;
	// kprintf_interruptable("hi");
	// static int aaa = 0;//not sure if this is safe for multiple cores
	aaa += 2;
	// aaa = 2;
	int c;
	c++;
	if (c == 1) c = 0;
	if (aaa == 2) hot_create_and_push_thread(5, gen2);
	//no need to call reschedule nor hot_reschedule after because of race condition and you just set the lapic timer instead
	// if (aaa == 2) hot_create_and_push_user_thread(running_thread->pid+1000, test_a);

	// while (1) {test_b();}

	// uint64_t a = alloc_frame();
    // map_page((get_current_thread()->cr3), a, 0x10000000, 0b111);
    // memset((void*) (0x10000000), 0, 0x1000);

	while (1) { kprintf("gen2: hi from thread %d\n", get_current_thread()->pid); yield_thread(); }
}

void gen3() {
	//HERE i think it page faults because ht isn't initialized yet because we called tmpfs_open a couple of times before we called vfs_open
	// int a = vfs_open(tmpfs_root, "bye2.txt", 0);
	// kprintf("\nhi\n");
	// syscall_log("\n\nhi\n\n");
	for (uint64_t i = 0; i < 32; i++) {
		// map_page(get_current_thread()->cr3, 0x10000, (uint64_t) i, 0b111);
		// uint64_t* a;

		// sys_vm_map((uint64_t*) (i+0x1000000), 0x1000, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | 0x1000, 0, 0, (void**) &a);
	}
	for (uint64_t i = 0; i < get_current_thread()->mappings.max_mappings; i++) {//we need max because num mappings can go under an allocated index
		// kprintf("%d %llx virt address ABC\n", i, get_current_thread()->mappings.mapped_virt_addresses_array[i].virt_address);
	}
	// while (1);

	// int fd = vfs_fdopen(tmpfs_root->vnode_data, "bye2.txt", 0);
	int fd = vfs_fdopen("/test dir 4/test dir 5/hihi.txt", 0);
	char* buf = (char*) kmalloc_byte(1024);
	// tmpfs_fd_read_from_file(fd, buf, 128, 0);
	// buf[127] = '\0';
	// kprintf("%s\n", buf);
	// char buf1[] = "\nhelloworldhelloworldfjdkslafjdkla\n\n\nfjdsa";
	// tmpfs_fd_write_to_file(fd, buf1, sizeof(buf1), 128);
	
	tmpfs_fd_read_from_file(fd, buf, 256, 0);
	// kprintf("%s\n", buf);//if we just directly print it we won't get the entire thing since there's null chars littered in it i think
	for (int i = 0; i < 256; i++) {
		kprintf("%c", buf[i]);
	}

	vfs_fdclose(fd);
	
	// while (1) asm volatile ("cli");
	// while (1);
	
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

uint64_t num_threads;

bool scheduling_started = 0;//0 for no and 1 for yes

// //declared inside mutex.h
// bool GLOBAL_SCHED_OBJECT;
// mutex_t GLOBAL_SCHED_QUEUE_LOCK;

bool first_thread = 1;//for push_thread()


void init_scheduler() {
	push_thread(create_thread(1500, gen2));
	// push_thread(create_thread(3, gen1));

	lapic_periodic(5, 80, 0b0011, 0);
	while (1) reschedule();
}


thread_context* insert_thread(thread_context* left_thread, thread_context* new_thread, thread_context* right_thread) {//left and right has no meaning for specific directions. it's just threads that are adjacent to each other
	//insert between two threads
	//we "consume" threads left to right
	assert(left_thread);
	assert(new_thread);
	assert(right_thread);
	//hopefully there's no ordering issue
	left_thread->next_thread = new_thread;
	
	right_thread->prev_thread = new_thread;
	
	new_thread->next_thread = right_thread;
	new_thread->prev_thread = left_thread;

	return new_thread;
}

thread_context* discard_thread(thread_context* thread) {
	//stitches back together the doubly linked list
	// if (thread == thread->next_thread);
	thread->prev_thread->next_thread = thread->next_thread;
	thread->next_thread->prev_thread = thread->prev_thread;

	return thread;
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
	bool irq;
	irq_disable_save(&irq);
	//HERE we don't need to swap in the original cr3 because the heap was already mapped previously
	volatile thread_context* t = create_thread(pid, thread_entry);
	uint64_t new_cr3 = create_new_userspace_page_table();
	t->cr3 = (uint64_t*) new_cr3;
	t->elf_entry = NULL;//HERE always remember to set the appropriate things to NULl because some stuff will expect it to be NULL
	t->elf_file = NULL;
	push_thread(t);
	num_threads++;
	irq_restore(&irq);
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
	t->elf_file = file;
	t->cr3 = (uint64_t*) (new_cr3);
    push_thread(t);
	num_threads++;
	asm volatile ("mov %0, %%cr3" :: "r"(current_cr3));
    // reschedule();//not sure if i should add reschedule here
	
	
	irq_restore(&irq);
	// asm volatile ("sti");
}

void hot_create_and_push_user_thread(uint64_t pid, void (*thread_entry)(void)) {
	bool irq;
	irq_disable_save(&irq);
	//HERE we don't need to swap in the original cr3 because the heap was already mapped previously
	uint64_t current_cr3 = (uint64_t) get_cr3();
	volatile thread_context* t = create_thread(pid, userspace_run_elf);
	uint64_t new_cr3 = create_new_userspace_page_table();
	// asm volatile ("mov %0, %%cr3" :: "r"(new_cr3 - hhdm_offset));//not sure if we need this here
	t->elf_entry = thread_entry;//HERE always remember to not unintentionally unset stuff after you've set stuff
	t->cr3 = (uint64_t*) new_cr3;
	t->elf_file = NULL;
	push_thread(t);
	num_threads++;
	// asm volatile ("mov %0, %%cr3" :: "r"(current_cr3));
	irq_restore(&irq);
}

void hot_reschedule() {//HERE must use this to call reschedule instead of just reschedule() itself inside a thread because if you create a thread inside a thread and you don't call reschedule() right after it, it gets preempted and some stuff doesn't get pushed back and the logic breaks, so you must either do both in "atomically", i.e., without getting preempted or just have this after which takes care of it i think
	// asm volatile ("cli");
	bool irq;
	irq_disable_save(&irq);
	volatile thread_context* current_thread = get_current_thread();
	reschedule();
	irq_restore(&irq);
}

volatile thread_context* get_current_thread() {
	return running_thread;
}

void disable_preemption() {	
	asm volatile ("cli");
}
void enable_preemption() {
	asm volatile ("sti");
}

//thanks to mishakov for the code outline
uint8_t first = 0;
void reschedule() {
	// lapic_oneshot(0, 72, 0b0011, 1);
	// disable_preemption();
	asm volatile ("cli");
	asm volatile ("mov %0, %%cr3" :: "r"(((uint64_t) pml4_address_virt_glob) - hhdm_offset));
	// print_queue();

	scheduling_started = 1;

	//we clear the eoi here because i don't wanna wrap it with a cr3 switch
    volatile uint32_t* lapic_eoi = (uint32_t*) ((uintptr_t)(ACPI_MADT->lapic_addr + 0xb0));
	*lapic_eoi = 0;

	volatile thread_context* current_thread;
	
	if (first == 0) {//i could probably simplify this..
		first = 1;
		current_thread = running_thread;

		uint64_t* a;
		if (!current_thread) {
			kprintf_interruptable("no more threads");
			while (1) {asm volatile ("cli; hlt");}
			// goto end;
		}
		running_thread->last_run_time = tsc_read_ns();
		change_tss(tss, current_thread->stack_base);

		if (current_thread->status[4] == 1) {
			swap_to_user_gs();
		}


		lapic_oneshot(THREAD_QUANTUM, 72, 0b0011, 0);//HERE REMEMBER TO USE 0b0011 INSTEAD OF 16
		asm volatile ("mov %0, %%cr3" :: "r"(((uint64_t) running_thread->cr3) - hhdm_offset));
		switch_thread(&a, running_thread->current_rsp);
		// kfree_interruptable(thread_context* a));
		// kfree_interruptable(a);
		// enable_preemption();
		return;
	}

	// volatile thread_context* next_thread = pop_front(ready_queue);//&ready_queue
	current_thread = running_thread;
	running_thread = running_thread->next_thread;
	volatile thread_context* next_thread = running_thread;
	assert(next_thread);
	if (!next_thread) {
		kprintf_interruptable("error no more threads");
		while (1) {asm volatile ("cli; hlt");}
	}
	if (next_thread->pid == next_thread->prev_thread->pid) {//HERE FIX ME
		// kprintf("1 thread left");
		lapic_oneshot(THREAD_QUANTUM, 72, 0b0011, 0);//idk maybe disregard the stuff after this sentence; i think reschedule() does return so we can call it after creating a thread so we can leave this off i think. i'm not sure if i should leave this on or not. if i leave it off i leave it up to the newly created thread or the user to call reschedule, but then it also means that it can't return back to it because reschedule never returns or something idk, so leaving it on would force the isr to reschedule instead of the function so it doesn't break anything i guess?? but i'm also not 100% sure that reschedule returns or not, as yield_thread() calls it and idk if it returns??? i don't think i actually need this. because if i do end up adding another thread when there's only 1 left, next_thread will be different. HERE REMEMBER TO USE 0b0011 INSTEAD OF 16
		// asm volatile ("sti");//need to reenable it because we don't have switch thread which reenables it
		// kprintf_interruptable("byebye");
		asm volatile ("mov %0, %%cr3" :: "r"(((uint64_t) next_thread->cr3) - hhdm_offset));//ALWAYS REMEMBER TO SWAP THE CORRECT CR3 BACK IN
		return;//don't switch just return
	}

	if (next_thread->status[3] == 1) {
		while (next_thread->status[3] == 1) {//prevents the next thread from being blocked.
			running_thread = running_thread->next_thread;
			assert(next_thread);
			if (next_thread->pid == 0xDEADBEEFCAFEBABE) {
				kprintf_interruptable("idle thread...");
				lapic_oneshot(THREAD_QUANTUM, 72, 0b0011, 0);
				return;
			}
			kprintf_interruptable("\current thread blocked, switching from thread %d\n", next_thread->pid);
		}
    }
	
	if (next_thread->status[3] == 0) {
		if (next_thread->pid == next_thread->prev_thread->pid) {
			return;
		}
		#ifdef SCHEDULER_VERBOSE
		kprintf_interruptable("\nswitching from thread %d to thread %d at reschedule\n", current_thread->pid, next_thread->pid);
		#endif
		current_thread->last_run_time = tsc_read_ns();
		change_tss(tss, next_thread->stack_base);

		if (current_thread->status[4] == 1) {
			// swap_to_user_gs();
		}

		// running_thread = running_thread->next_thread;
		lapic_oneshot(THREAD_QUANTUM, 72, 0b0011, 0);//HERE REMEMBER TO USE 0b0011 INSTEAD OF 16
		asm volatile ("mov %0, %%cr3" :: "r"(((uint64_t) next_thread->cr3) - hhdm_offset));
		switch_thread(&running_thread->prev_thread->current_rsp, running_thread->current_rsp);
	}

}

void scheduler_return() {//basically pthread_exit
	//HERE remember to figure out if you need a way to return to kernelspace via a syscall something for scheduler_return() to run
	//if this whole thing is uninterruptable it's actually a pretty long process so maybe we should have several sections where it's interruptable so we still maintain normal thread scheduling times
	asm volatile ("cli");

	// kprintf("\n\n\nHIHIHI\n\n\n");

	asm volatile ("mov %0, %%cr3" :: "r"(((uint64_t) pml4_address_virt_glob) - hhdm_offset));


	volatile thread_context* current_thread = get_current_thread();

	current_thread = running_thread;
	running_thread = running_thread->next_thread;
	
	thread_context* next_thread = running_thread;

	// volatile thread_context* temp = ready_queue_second_last;//HERE not sure if i'm supposed to do double pointer or just copy it
	volatile thread_context* temp = current_thread;
	uint64_t temp_pid = temp->pid;
	// ready_queue_second_last->status[3] = 1;
	// temp->status[3] = 1;
	//add lock thing here
	// disable_preemption();

	//HERE disregard the stuff after this sentence; i think because before i was using the top? (or bottom?) and now it's the other way around. remember to free temp->stackbase+THREAD_STACK_SIZE since stack base is at the very top and we allocated from the bottom

	// ready_queue_second_last = current_thread;
	assert(next_thread);
	if (!next_thread) {
		kprintf_interruptable("no more threads");
		while (1) {asm volatile ("cli; hlt");}
	}

	if (next_thread->pid == temp->pid) {//for if there's only 1 thread left
		goto kill_and_switch_to_idle;
	}
	// ready_queue_second_last->status[3] = 1;//same thing as below


	// uint64_t temp_pid = ready_queue_second_last->pid;
	uint64_t* a;

	// asm volatile ("mov %0, %%cr3" :: "r"(((uint64_t) pml4_address_virt_glob) - hhdm_offset));
	volatile uint32_t* lapic_eoi = (uint32_t*) ((uintptr_t)(ACPI_MADT->lapic_addr + 0xb0));
	*lapic_eoi = 0;

	while (next_thread->status[3] == 1) {//prevents the next thread from being blocked.
		next_thread = next_thread->next_thread;
		if (temp_pid == next_thread->pid) {

			kill_and_switch_to_idle:

			// kprintf_interruptable("\n%d\n", num_threads);


			thread_context* actual_running_thread = running_thread;
			running_thread = temp;//i feel like this is a bad idea if something gets interrupted in the middle and the threads get mixed up
			if ((temp->elf_entry != NULL) && (temp->elf_file != NULL)) {//because elf entry only refers to the thing we wanna run in userspace regardless of whether or not it's an elf
				unload_elf(temp->elf_file, (uint64_t) temp->cr3);
			}

			for (uint64_t i = 0; i < temp->mappings.max_mappings; i++) {//we need max because num mappings can go under an allocated index
				// kprintf("%d %llx virt address ABC\n", i, temp->mappings.mapped_virt_addresses[i]);
				// continue;
				if (temp->mappings.mapped_virt_addresses_array[i].used == 0) continue;
				unmap_page(temp->cr3, temp->mappings.mapped_virt_addresses_array[i].virt_address);
			}

			kfree((uint64_t*) temp->fd_table);
			kfree((uint64_t*) ((struct oa_hash*) (temp->fd_table))->buckets);

			running_thread = actual_running_thread;

			discard_thread(temp);
			num_threads--;

			kfree_interruptable((uint64_t*) (((uint64_t) temp->stack_base)-THREAD_STACK_SIZE));
			free_frame(((uint64_t)temp->cr3) - hhdm_offset);
			kfree_interruptable((uint64_t*) temp);

			kprintf_interruptable("\nno more threads to schedule. switching to idle\n");

			hot_create_and_push_thread(0xDEADBEEFCAFEBABE, idle_thread);

			if (running_thread->next_thread->pid != 0xDEADBEEFCAFEBABE) {
				kprintf_interruptable("\nthread %d not found\n", next_thread->pid);
				while (1) asm volatile ("cli; hlt");
			}

			change_tss(tss, running_thread->next_thread->stack_base);
			lapic_oneshot(THREAD_QUANTUM, 72, 0b0011, 0);//still set a timer because a thread still may somehow get pushed idk?? not sure if it really matters
			asm volatile ("mov %0, %%cr3" :: "r"(((uint64_t) running_thread->next_thread->cr3) - hhdm_offset));
			switch_thread(&a, running_thread->next_thread->current_rsp);
		}
		assert(next_thread);
	}
	if (next_thread->status[3] == 0) {

		// kprintf_interruptable("\n%d\n", next_thread->pid);
		// while (1) {asm volatile ("cli; hlt");}

		thread_context* actual_running_thread = running_thread;
		running_thread = temp;//i feel like this is a bad idea if something gets interrupted in the middle and the threads get mixed up
			if ((temp->elf_entry != NULL) && (temp->elf_file != NULL)) {//because elf entry only refers to the thing we wanna run in userspace regardless of whether or not it's an elf
				unload_elf(temp->elf_file, (uint64_t) temp->cr3);
			}

		for (uint64_t i = 0; i < temp->mappings.max_mappings; i++) {
    		// kprintf("%d %llx virt address ABC\n", i, temp->mappings.mapped_virt_addresses[i]);
			// continue;

			if (temp->mappings.mapped_virt_addresses_array[i].used == 0) continue;
			unmap_page(temp->cr3, temp->mappings.mapped_virt_addresses_array[i].virt_address);
		}

		kfree((uint64_t*) temp->fd_table);
		kfree((uint64_t*) ((struct oa_hash*) (temp->fd_table))->buckets);

		running_thread = actual_running_thread;

		discard_thread(temp);
		num_threads--;
		kfree_interruptable((uint64_t*) (((uint64_t) temp->stack_base)-THREAD_STACK_SIZE));
		free_frame(((uint64_t)temp->cr3) - hhdm_offset);
		temp->next_thread = NULL;
		kfree_interruptable((uint64_t*) temp);
		
		kprintf_interruptable("\nthread exited!\nswitching from thread %d to thread %d at return\n", temp_pid, next_thread->pid);

		change_tss(tss, next_thread->stack_base);

		lapic_oneshot(THREAD_QUANTUM, 72, 0b0011, 0);//still set a timer because a thread may eventually get created and hot_reschedule may not get called
		asm volatile ("mov %0, %%cr3" :: "r"(((uint64_t) next_thread->cr3) - hhdm_offset));
		switch_thread(&a, next_thread->current_rsp);
	}
}

volatile thread_context* create_thread(uint64_t pid, void (*thread_entry)(void)) {
	bool irq;
	irq_disable_save(&irq);

	volatile uint64_t* thread_base = (uint64_t*) (((uint64_t) kmalloc_byte_interruptable(THREAD_STACK_SIZE)) + THREAD_STACK_SIZE);//16kb
	volatile thread_context* new_thread = (thread_context*) kmalloc_byte_interruptable(sizeof(thread_context));//HERE REMEMBER TO ALWAYS DISABLE INTERRUPTS WHEN NECESSARY OR USE THE STATE SAVING FUNCTIONS
	memset(new_thread, 0, sizeof(thread_context));//HERE always remember to zero these type of allocations

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
	new_thread->user_rsp = NULL;
	new_thread->stack_base = thread_base;
	new_thread->next_thread = NULL;
	new_thread->prev_thread = NULL;

	new_thread->elf_entry = NULL;
	new_thread->elf_file = NULL;
	new_thread->status[4] = 0;

	//fs and gs (mmio?)
	new_thread->fs_base = 0;
	new_thread->gs_base = 0;

	new_thread->current_dir = "/";

	memset(&(new_thread->ssefxsave), 0, 512);

	new_thread->mappings.num_mappings = 0;
	new_thread->mappings.max_mappings = 8;
	//i should probably use max_mappings instead of hardcoding it to 8 but oh well
	new_thread->mappings.mapped_virt_addresses_array = (mapped_virt_addresses_t*) kmalloc_byte(sizeof(mapped_virt_addresses_t) * 8);
	// memset(new_thread->mappings.mapped_virt_addresses, UINT64_MAX, sizeof(uint64_t) * 8);
	for (uint64_t i = 0; i < 8; i++) {
		new_thread->mappings.mapped_virt_addresses_array[i].virt_address = 0;
		new_thread->mappings.mapped_virt_addresses_array[i].flag = 0;
		new_thread->mappings.mapped_virt_addresses_array[i].used = 0;
	}


	struct oa_hash* ht = (struct oa_hash*) kmalloc_byte(sizeof(struct oa_hash));
	memset(ht, 0, sizeof(struct oa_hash));
	new_thread->fd_table = (vfs_fd_table_t*) ht;//the first member of the struct is the same since it's using OA_HASH_ATTRS(mut) i think
	struct oa_hash_entry* buckets;
	size_t capacity = 32;//we allocate in increments of 32
	buckets = (struct oa_hash_entry*) kmalloc_byte(capacity * sizeof(*buckets));//always remember to dereference to get the full size of the value and not just the size of the pointer
	// buckets = kmalloc_byte(capacity * sizeof(struct oa_hash_entry));//always remember to dereference to get the full size of the value and not just the size of the pointer
	oa_hash_init(ht, buckets, capacity);
	assert(ht->length == 0);

	// while (1);
	//HERE REMEMBER TO CAST TO PREVENT DOING POINTER ARITHMETIC INSTEAD OF JUST NORMAL ARITHEMETIC
    volatile uint64_t* thread_rsp = (uint64_t*) ((uint64_t) new_thread->stack_base);//i'm not actually sure if kmalloc is supposed to return an address that's been casted to a pointer. either way, this reverts it so it should be okay for now i think
	new_thread->current_rsp = thread_rsp;
	// map_page((uint64_t*) (pml4_address_virt_glob), (uint64_t)thread_entry, (uint64_t)thread_entry, 0b11);
	start_thread(&new_thread->current_rsp, thread_entry);
	// new_thread->stack_base -= 4;
	// new_thread->stack_base -= 20;
	kprintf_interruptable("created thread %d\n", pid);
	irq_restore(&irq);
	return new_thread;
}

void start_thread(uint64_t **sp, void *entry) {//thread_entry runs and then scheduler_return runs. the function never actually exits or something idk
	*sp -= 1;//apparently it moves it by 8 bytes for each index
	**sp = (uint64_t) scheduler_return;//basically pthread_exit i think. this is just for functions that are ran in kernelspace
	*sp -= 1;
	**sp = (uint64_t) disable_preemption;
	*sp -= 1;
	**sp = (uint64_t) entry;
	*sp -= 1;
	**sp = (uint64_t) enable_preemption;
	*sp -= 15;
}


void push_thread(volatile thread_context* thread) {
	if (first_thread || (num_threads == 0)) {////hopefully num_threads is correct
		running_thread = thread;
		running_thread->next_thread = running_thread;
		running_thread->prev_thread = running_thread;
		first_thread = 0;
	}
	else {
		//i don't think we can push it behind running_thread because if we save rsp to running_thread->prev_thread, we'd be saving to this instead of the intended thread, and using another variable might also get weird
		insert_thread(running_thread, thread, running_thread->next_thread);
	}
}

volatile thread_context* block_thread(uint64_t pid) {
	bool irq;
	irq_disable_save(&irq);
	// asm volatile ("cli");
	volatile thread_context* current_thread = running_thread;
	for (int i = 0; i < num_threads+1; i++) {//+1 in case there's an off by 1 error i guess
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
			//discard thread into blocked queue
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
		//insert thread
		kprintf_interruptable("\nunblocked thread %d\n", thread->pid);
	}
	else {
		kprintf_interruptable("\nthread %d is not blocked\n", thread->pid);
	}
	// asm volatile ("sti");
	irq_restore(&irq);
}

void yield_thread() {
	bool irq;
	irq_disable_save(&irq);
	volatile thread_context* current_thread = get_current_thread();
	kprintf_interruptable("\nyielding thread %d\n", current_thread->pid);
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
	volatile thread_context* current_thread = running_thread;
	for (int i = 0; i < num_threads+1; i++) {
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
	// return;
	bool irq;
	irq_disable_save(&irq);
	volatile thread_context* current_thread = running_thread;
	/*
	for (int i = 0; i < num_threads+1; i++) {
		int x = current_thread->pid;
		kprintf("%d ", x);
		current_thread = current_thread->next_thread;
	}
	irq_restore(&irq);
	return;
	*/
	#ifdef SCHEDULER_VERBOSE
	kprintf_interruptable("\n||| ");
	// while (current_thread) {
	for (int i = 0; i < num_threads+10; i++) {
		if (i == num_threads) kprintf_interruptable("||| ");
		kprintf_interruptable("%d ", current_thread->pid);
		current_thread = current_thread->next_thread;
	}
	kprintf_interruptable("|||\n");
	#endif
	irq_restore(&irq);
}