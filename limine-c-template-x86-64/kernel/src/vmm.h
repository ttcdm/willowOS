// #pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <kutils.h>
#include <paging.c>

#define HEAP_START_VIRT_DEFINED 0xfffc000000000000


uint64_t init_heap();

