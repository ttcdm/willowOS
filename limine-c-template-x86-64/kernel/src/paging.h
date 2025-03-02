#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

void map_page(uint64_t address);

uint64_t alloc_frame(void);

void free_frame(void);

typedef struct page_frame {
	uint8_t index;
} page_frame_t;