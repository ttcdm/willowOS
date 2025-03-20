#pragma once//HERE not sure if i should put this here but it seemed to solve the memmap_arr issue where it was defined as extern in this file and as the actual thing in main.c and the compiler was complaining

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>


uint64_t alloc_frame(void);

void free_frame(uint64_t phys_address);

typedef struct page_frame {
	uint8_t index;
} page_frame_t;

extern uint64_t starting_address;
extern uint8_t last_alloced_frame;
extern uint64_t hhdm_offset;

void init_physical_memory();//init_physical_memory() is put in main.c because hhdm_request is static and can only be used in main.c i think

void init_paging();

uint64_t virt_lookup(uint64_t virt_address);

void map_page(uint64_t* pml4_address, uint64_t phys_address, uint64_t virt_address, uint64_t permissions);

struct usable_memmaps_region {
	uint64_t base;
	uint64_t length;
	uint8_t frame_bitmap[600000];//i'm so sorry. may have issues with array being so large but idk
	struct usable_memmaps_region* next;
};//HERE not sure if i need to align or use packed for general structures

extern struct usable_memmaps_region memmap_arr[16];

void* get_cr3(void);

extern uint64_t pml4_address_virt_glob;//i'm sorry
