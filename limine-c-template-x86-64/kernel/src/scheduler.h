#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <kutils.h>
#include <paging.h>
#include <tsc.h>
#include <idt.h>
#include <apic.h>
#include <vmm.h>
#include <gdt.h>
#include <vfs.h>


#define THREAD_STACK_SIZE 16000//16kb stack for each thread. i don't think it overflows because 16k starts at 0 so we end up with 15999 as the last thing
#define THREAD_QUANTUM 30//not sure if quantum is the right word

typedef struct mapped_virt_addresses {
	uint64_t virt_address;
	uint64_t flag;//map_shared, map_private, or none
	bool used;
} mapped_virt_addresses_t;
typedef struct thread_mappings {//used by map_page but it's only used if there's a running thread. maybe i should change it for the original cr3 as well idk
	mapped_virt_addresses_t* mapped_virt_addresses_array;
	uint64_t num_mappings;
	uint64_t max_mappings;
} thread_mappings_t;

typedef struct thread_context_declared {
	uint64_t start_time;
	uint64_t last_start_time;
	uint64_t total_run_time;
	uint64_t last_run_time;
	uint64_t sleep_for_ms;//maybe change it to ns in the future
	uint64_t quantum_ns;

	uint64_t pid;
	void (*thread_entry)(void);
	uint64_t* stack_base;
	uint64_t* return_rsp;
	uint64_t misaligned_by;
	uint64_t* current_rsp;
	uint64_t current_misaligned_by;
	uint64_t status[10];//RUNNING, READY, BLOCKED, SLEEPING, USERSPACE?.//running flag isn't used for now. also this should probably be bool but oh well
	// void (*elf_entry)(void);
	void* elf_entry;
	vfs_fd_t* elf_file;
	thread_mappings_t mappings;
	vfs_fd_table_t* fd_table;
	// uint64_t cr3[512];
	uint64_t* cr3;//HERE MAKE SURE CR3 IS 4KIB ALIGNED; virt_address of cr3

	struct thread_context_declared* next_thread;
	struct thread_context_declared* prev_thread;

} thread_context;

//extern thread_context* current_thread;

extern volatile thread_context* ready_queue;
extern volatile thread_context* ready_queue_head;
extern volatile thread_context* ready_queue_end;
extern volatile thread_context* ready_queue_second_last;
// extern volatile thread_context** current_actual;
extern volatile thread_context* running_thread;

extern uint64_t num_threads;

extern bool scheduling_started;//0 for no and 1 for yes

void init_scheduler(void);

volatile thread_context* create_thread(uint64_t pid, void (*thread_entry)(void));
void push_thread(volatile thread_context* thread);
volatile thread_context* block_thread(uint64_t pid);//might run into issues if we recycle the pids later on. also not sure if i should search by thread context or pid
//thread_context* block_by_pid(uint64_t* pid);
void unblock_thread(volatile thread_context* thread);
volatile thread_context* sleep_thread(uint64_t pid, uint64_t ms);
void yield_thread();//not sure if you're supposed to yield current thread or yield a thread of your choosing
volatile thread_context* get_thread_by_pid(uint64_t pid);
void switch_thread(uint64_t** old_rsp, uint64_t* new_rsp);
void start_thread(unsigned long **sp, void *entry);

void print_queue();

extern void push_all_regs();
extern void pop_all_regs();

void scheduler_return();

volatile thread_context* pop_front(volatile thread_context* thread); // Removes the thread from the front and returns its pointer, or null if empty
void push_back(volatile thread_context* ready_queue, volatile thread_context* thread); // Pushes thread to the queue
void hot_create_and_push_thread(uint64_t pid, void (*thread_entry)(void));
void hot_exec_elf(uint64_t pid, void* file);
void hot_create_and_push_user_thread(uint64_t pid, void (*thread_entry)(void));

void hot_reschedule();

volatile thread_context* get_current_thread(); // Returns the running thread
void reschedule();

void enable_preemption();
void disable_preemption();

void gen0();
void gen1();
void gen2();
void gen3();