#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <kutils.h>
#include <vmm.h>


// https://www.ibm.com/docs/en/xl-c-and-cpp-aix/16.1.0?topic=functions-sync-bool-compare-swap
// rowleydownload.co.uk/arm/documentation/gnu/gcc/_005f_005fsync-Builtins.html

typedef struct mutex {
    bool locked;
    void* object;
} mutex_t;


bool acquire_mutex(mutex_t* mutex);
bool release_mutex(mutex_t* mutex);