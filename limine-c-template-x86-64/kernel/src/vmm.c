#include <vmm.h>


uint64_t init_heap() {

    uint64_t heap_start = HEAP_START_VIRT_DEFINED;

    uint64_t heap_page = alloc_frame() + hhdm_offset;

    uint64_t* cr3 = (uint64_t*) get_cr3() + hhdm_offset;

    map_page(cr3, heap_page, heap_start, 0b11);

    return heap_start;


}