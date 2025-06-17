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


#define THREAD_STACK_SIZE 16000//16kb stack for each thread. i don't think it overflows because 16k starts at 0 so we end up with 15999 as the last thing
#define THREAD_QUANTUM 10//not sure if quantum is the right word

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
	uint64_t status[10];//RUNNING, READY, BLOCKED, SLEEPING.//running flag isn't used for now. also this should probably be bool but oh well
	struct thread_context_declared* next_thread;

} thread_context;

//extern thread_context* current_thread;

volatile extern thread_context* ready_queue;
volatile extern thread_context* ready_queue_head;
volatile extern thread_context* ready_queue_end;
volatile extern thread_context* ready_queue_second_last;
// volatile extern thread_context** current_actual;
volatile extern thread_context* running_thread;
void init_scheduler(void);

thread_context* create_thread(uint64_t pid, void (*thread_entry)(void));
void push_thread(thread_context* thread);
thread_context* block_thread(uint64_t pid);//might run into issues if we recycle the pids later on. also not sure if i should search by thread context or pid
//thread_context* block_by_pid(uint64_t* pid);
void unblock_thread(thread_context* thread);
thread_context* sleep_thread(uint64_t pid, uint64_t ms);
void yield_thread();//not sure if you're supposed to yield current thread or yield a thread of your choosing
thread_context* get_thread_by_pid(uint64_t pid);
void switch_thread(uint64_t** old_rsp, uint64_t* new_rsp);
void start_thread(unsigned long **sp, void *entry);

void print_queue();

extern void push_all_regs();
extern void pop_all_regs();

void scheduler_return();

thread_context* pop_front(thread_context* thread); // Removes the thread from the front and returns its pointer, or null if empty
void push_back(thread_context* ready_queue, thread_context* thread); // Pushes thread to the queue
void hot_push_thread(thread_context* thread);
thread_context* get_current_thread(); // Returns the running thread
void reschedule();

void enable_preemption();
void disable_preemption();

void gen0();
void gen1();
void gen2();