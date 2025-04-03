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




typedef struct thread_context_declared {
	uint64_t start_time;
	uint64_t last_start_time;
	uint64_t total_run_time;
	uint64_t quantum_ns;

	uint64_t pid;
	uint64_t* stack_base;
	struct thread_context_declared* next_thread;

} thread_context;

extern thread_context* current_thread;


void init_scheduler(void);

thread_context* create_thread(uint64_t pid);