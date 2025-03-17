// #pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <kutils.h>

#include <paging.h>

#define HEAP_START_VIRT_DEFINED 0xffffc00000000000


uint64_t init_heap();