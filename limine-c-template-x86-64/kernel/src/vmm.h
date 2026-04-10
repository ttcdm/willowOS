#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <kutils.h>

#include <paging.h>

#define HEAP_START_VIRT_DEFINED 0xffffc00000000000
#define HEAP_SIZE_DEFINED 0x400000//16mb of memory for heap
// #define HEAP_SIZE_DEFINED 0x4000000//256mb of memory for heap
// #define HEAP_SIZE_DEFINED 0x8000000//512mb of memory for heap


#define PAGE_SIZE_DEFINED 0x1000
#define HEAP_CHUNK_SIZE_DEFINED 64

uint64_t init_heap();

uint64_t* kmalloc(uint64_t size);

void kfree(uint64_t*);
void kfree_interruptable(uint64_t*);

void print_heap(uint64_t length);

uint64_t* kmalloc_byte(uint64_t size);
uint64_t* kmalloc_byte_interruptable(uint64_t size);

uint64_t* krealloc_byte(uint64_t* virt_address, uint64_t size);

typedef struct heap_page_virt {
    uint8_t status;//0 for free and 1 for used
    uint64_t size;
    uint64_t alloc_length;//same as the normal lengths like len() in python (1 indexed?)
    struct heap_page_virt* next;
} heap_page;

extern heap_page* heap_page_head;