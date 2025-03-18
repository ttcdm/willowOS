// #pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <kutils.h>

#include <paging.h>

#define HEAP_START_VIRT_DEFINED 0xffffc00000000000
#define HEAP_SIZE_DEFINED 0x400000//16mb of memory for heap
#define PAGE_SIZE_DEFINED 0x1000
#define HEAP_CHUNK_SIZE_DEFINED 64

uint64_t init_heap();

uint64_t kmalloc();

typedef struct heap_page_virt {
    uint8_t status;//0 for free and 1 for used
    uint64_t size;
    struct heap_page_virt* next;
} heap_page;

extern heap_page* heap_page_head;