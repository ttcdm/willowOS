#include <vmm.h>


uint64_t init_heap() {
    uint64_t heap_start = HEAP_START_VIRT_DEFINED;
    uint64_t heap_page = alloc_frame() + hhdm_offset;

    map_page((uint64_t*) (pml4_address_virt_glob), heap_page, heap_start, 0b11);
    kprintln("successfully initalized heap");
    return heap_start;
}