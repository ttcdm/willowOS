#include <vmm.h>

heap_page* heap_page_head;
uint64_t init_heap() {
    uint64_t heap_start = HEAP_START_VIRT_DEFINED;
    heap_page_head = (heap_page*) (alloc_frame()+hhdm_offset);
    heap_page_head->size = HEAP_CHUNK_SIZE_DEFINED;
    heap_page_head->status = 0;
    heap_page_head->next = NULL;
    heap_page* current = heap_page_head;
    for (int i = 0; i < HEAP_SIZE_DEFINED/PAGE_SIZE_DEFINED; i++) {
        uint64_t heap_page_phys = alloc_frame();
        map_page((uint64_t*) (pml4_address_virt_glob), heap_page_phys, heap_start + (i*PAGE_SIZE_DEFINED), 0b11);

        heap_page* new_heap_page = (heap_page*) (alloc_frame()+hhdm_offset);
        new_heap_page->size = HEAP_CHUNK_SIZE_DEFINED;
        new_heap_page->status = 0;
        new_heap_page->next = NULL;
        current->next = new_heap_page;
        current = current->next;
    }
    kprintln("successfully initalized heap");
    // current = head;
    // int i = 0;
    // while (current->next != NULL) {        
    //     i++;
    //     current = current->next;
    // }
    // kprintln_uint64(i);
    return heap_start;
}

uint64_t kmalloc(uint64_t size) {
    heap_page* current = heap_page_head;
    uint64_t index = 0;
    // while (current->next != NULL) {
    while (current != NULL) {//fixes the same off by one error in alloc_frame()
        int fits = 0;//0 for fits 1 for does not fit
        if (current->status == 0) {
            heap_page* probe = current;//probe should be stored in memory or something idk instead of stack. idk actually
            for (int i = 0; i < size-1; i++) {//we do size-1 because the last line sets it as the last node we need but it doesn't actually check it, and so it gets checked by the if block at the end
                if (probe->next == NULL) {
                    kprintln("heap is full");
                    return 0;
                }
                if (probe->status == 1) {
                    fits = 1;
                    break;
                }
                probe = probe->next;
            }
            if (fits == 1) {
                continue;
            }
            if (probe->status == 0) {
                probe = current;
                for (int set_status = 0; set_status < size; set_status++) {
                    probe->status = 1;
                    probe = probe->next;
                }
                kprintln_uint64(index);
                return HEAP_START_VIRT_DEFINED + (index * HEAP_CHUNK_SIZE_DEFINED);
            }
        }
        current = current->next;
        index++;
    }
}

