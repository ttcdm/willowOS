#include <vmm.h>

heap_page* heap_page_head;
uint64_t init_heap() {
    uint64_t heap_start = HEAP_START_VIRT_DEFINED;
    heap_page_head = (heap_page*) (alloc_frame()+hhdm_offset);
    heap_page_head->size = HEAP_CHUNK_SIZE_DEFINED;
    heap_page_head->status = 0;
    heap_page_head->alloc_length = 0;
    heap_page_head->next = NULL;
    heap_page* current = heap_page_head;
    for (int i = 0; i < HEAP_SIZE_DEFINED/PAGE_SIZE_DEFINED; i++) {
        uint64_t heap_page_phys = alloc_frame();
        map_page((uint64_t*) (pml4_address_virt_glob), heap_page_phys, heap_start + (i*PAGE_SIZE_DEFINED), 0b11);

        heap_page* new_heap_page = (heap_page*) (alloc_frame()+hhdm_offset);
        new_heap_page->size = HEAP_CHUNK_SIZE_DEFINED;
        new_heap_page->status = 0;
        new_heap_page->alloc_length = 0;
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

uint64_t* kmalloc(uint64_t size) {
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
                current->alloc_length = size;//size of allocated memory
				kprint("allocated heap at index: ");
                kprintln_uint64(index);
                return (uint64_t*) (HEAP_START_VIRT_DEFINED + (index * HEAP_CHUNK_SIZE_DEFINED));//HERE hopefully there's no issue with using macros as the values for the operations
            }
        }
        current = current->next;
        index++;
    }
    kprintln("heap is full. returning 0");
    return 0;
}

void kfree(uint64_t* virt_address) {
    uint64_t index = ((uint64_t) virt_address - HEAP_START_VIRT_DEFINED) / HEAP_CHUNK_SIZE_DEFINED;
    heap_page* current = heap_page_head;
    for (int i = 0; i < index; i++) {//there's no safety against trying to clear past the end of the heap here, but kalloc() prevents you from allocating past the end, so i don't think that there's any errors
        current = current->next;//we do the second last one because at the end of the loop it moves onto the last node
    }
    uint64_t alloc_length_node = current->alloc_length;
    current->alloc_length = 0;
    for (int i = 0; i < alloc_length_node; i++) {
        current->status = 0;
        current = current->next;
    }
    kprint("freed node(s): ");
    kprint_uint64(alloc_length_node);
    kprint(" starting index: ");
    kprintln_uint64(index);
}

void print_heap(uint64_t length) {
    heap_page* current = heap_page_head;
    for (int i = 0; i < length; i++) {
        kprint("index: ");
        kprint_uint64(i);
        kprint(" status: ");
        kprint_uint64(current->status);
        kprint(" size: ");
        kprint_uint64(current->size);
        kprint(" alloc_length: ");
        kprintln_uint64(current->alloc_length);
        current = current->next;
    }
}