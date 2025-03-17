#include <vmm.h>

uint64_t kmalloc(uint64_t size) {
	uint64_t address = alloc_frame();
	return address;
}