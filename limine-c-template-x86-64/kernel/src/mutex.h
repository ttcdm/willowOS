#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <kutils.h>
#include <vmm.h>
#include <scheduler.h>


// https://www.ibm.com/docs/en/xl-c-and-cpp-aix/16.1.0?topic=functions-sync-bool-compare-swap

typedef struct mutex {
    bool locked;
    void* object;
} mutex_t;


bool acquire_mutex(mutex_t* mutex);
bool release_mutex(mutex_t* mutex);

// //need to put this here because of circular includes or something
// extern bool GLOBAL_SCHED_OBJECT;
// extern mutex_t GLOBAL_SCHED_QUEUE_LOCK;

typedef struct futex_queue {
    int* pointer;
    thread_context** threads;
    uint64_t size;
    uint64_t max_size;//we allocate 64 bytes (8 threads for max size and we realloc if we need more)
} futex_queue_t;

extern futex_queue_t** global_futex_array;
extern uint64_t global_futex_array_size;

void init_futex();

int futex_enqueue(int* pointer, thread_context* thread);