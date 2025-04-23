#include <vmm.h>


/*

    uint64_t heap_page_phys;
    heap_page* new_heap_page;
    for (int i = 0; i < HEAP_SIZE_DEFINED/HEAP_CHUNK_SIZE_DEFINED; i++) {
        if (i % (PAGE_SIZE_DEFINED/HEAP_CHUNK_SIZE_DEFINED) == 0) {//i think this works
            heap_page_phys = alloc_frame();
            map_page((uint64_t*) (pml4_address_virt_glob), heap_page_phys, heap_start + (i*HEAP_CHUNK_SIZE_DEFINED), 0b11);
            new_heap_page = (heap_page*) (alloc_frame()+hhdm_offset);
            kprintln_uint64(i);
        }
        else {
            heap_page_phys = heap_page_phys + HEAP_CHUNK_SIZE_DEFINED;
            map_page((uint64_t*) (pml4_address_virt_glob), heap_page_phys, heap_start + (i*HEAP_CHUNK_SIZE_DEFINED), 0b11);
            // kprintln_uint64(i);
            new_heap_page = new_heap_page + HEAP_CHUNK_SIZE_DEFINED;
        }

        new_heap_page->size = HEAP_CHUNK_SIZE_DEFINED;
        new_heap_page->status = 0;
        new_heap_page->alloc_length = 0;
        new_heap_page->next = NULL;
        current->next = new_heap_page;
        current = current->next;
    }*/

heap_page* heap_page_head;
uint64_t init_heap() {
    uint64_t heap_start = HEAP_START_VIRT_DEFINED;
    heap_page_head = (heap_page*) (alloc_frame()+hhdm_offset);//make sure we don't call alloc_frame() more than we need to 
    map_page((uint64_t*) (pml4_address_virt_glob), ((uint64_t)heap_page_head) - hhdm_offset, heap_start, 0b11);
    heap_page_head->size = HEAP_CHUNK_SIZE_DEFINED;
    heap_page_head->status = 0;
    heap_page_head->alloc_length = 0;
    heap_page_head->next = NULL;
    heap_page* current = heap_page_head;

    // for (int i = 0; i < HEAP_SIZE_DEFINED/PAGE_SIZE_DEFINED; i++) {//old initialization that uses 4096 bytes per 64 byte heap page
    //     uint64_t heap_page_phys = alloc_frame();
    //     map_page((uint64_t*) (pml4_address_virt_glob), heap_page_phys, heap_start + (i*PAGE_SIZE_DEFINED), 0b11);

    //     heap_page* new_heap_page = (heap_page*) (alloc_frame()+hhdm_offset);
    //     new_heap_page->size = HEAP_CHUNK_SIZE_DEFINED;
    //     new_heap_page->status = 0;
    //     new_heap_page->alloc_length = 0;
    //     new_heap_page->next = NULL;
    //     current->next = new_heap_page;
    //     current = current->next;
    // }

    uint64_t heap_page_phys = alloc_frame();//this seems to work. the starting logic is a bit messy but i think it should work
    map_page((uint64_t*) (pml4_address_virt_glob), heap_page_phys, heap_start, 0b11);
    heap_page* new_heap_page = heap_page_head;
    for (uint64_t i = 1; i < HEAP_SIZE_DEFINED/HEAP_CHUNK_SIZE_DEFINED; i++) {
        if (i % (PAGE_SIZE_DEFINED/HEAP_CHUNK_SIZE_DEFINED) == 0) {//i think this works
            // kprintf("%d\n", i);
            heap_page_phys = alloc_frame();
            map_page((uint64_t*) (pml4_address_virt_glob), heap_page_phys, heap_start + (i*HEAP_CHUNK_SIZE_DEFINED), 0b11);
            new_heap_page = (heap_page*) (alloc_frame()+hhdm_offset);//(alloc_frame()+hhdm_offset);
        }
        else {
            heap_page_phys = heap_page_phys + HEAP_CHUNK_SIZE_DEFINED;
            // map_page((uint64_t*) (pml4_address_virt_glob), heap_page_phys, heap_start + (i*HEAP_CHUNK_SIZE_DEFINED), 0b11);//not sure if i need to map this as well since it's in the middle of a mapped page
            new_heap_page = (heap_page*) ((uint64_t)new_heap_page + HEAP_CHUNK_SIZE_DEFINED);
        }

        new_heap_page->size = HEAP_CHUNK_SIZE_DEFINED;
        new_heap_page->status = 0;
        new_heap_page->alloc_length = 0;
        new_heap_page->next = NULL;
        current->next = new_heap_page;
        current = current->next;
    }
    kprintln("successfully initalized heap");
    return heap_start;
}

uint64_t* kmalloc(uint64_t size) {
    // asm volatile ("cli");
    if (size == 0) {
        kprintln("allocated 0 bytes. returning 0");
        return 0;//might page fault if you try to dereference this
    }
    heap_page* current = heap_page_head;
    // current = current->next;

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
                
                
                // asm volatile ("sti");
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
    asm volatile ("cli");
    uint64_t index = (((uint64_t) virt_address) - HEAP_START_VIRT_DEFINED) / HEAP_CHUNK_SIZE_DEFINED;
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
    asm volatile ("sti");
}

void kfree_interruptable(uint64_t* virt_address) {
    // asm volatile ("cli");
    uint64_t index = (((uint64_t) virt_address) - HEAP_START_VIRT_DEFINED) / HEAP_CHUNK_SIZE_DEFINED;
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
    // asm volatile ("sti");
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

uint64_t* kmalloc_byte(uint64_t size) {//we do cli and sti here instead of inside kmalloc. kmalloc alloc's 64 bytes per unit and this is one byte per unit. pretty sure this covers all cases
    asm volatile ("cli");
    if (size == 0) {
        asm volatile ("sti");
        return kmalloc(0);
    }
    if (size <= 64) {
        asm volatile ("sti");
        return kmalloc(1);
    }
    else {
        if (size % 64 == 0) {
            asm volatile ("sti");
            return kmalloc(size / 64);
        }
        else {
            asm volatile ("sti");
            return kmalloc((size / 64) + 1);
        }
    }
}

uint64_t* kmalloc_byte_interruptable(uint64_t size) {//kmalloc alloc's 64 bytes per unit and this is one byte per unit. pretty sure this covers all cases
    if (size == 0) {
        return kmalloc(0);
    }
    if (size <= 64) {
        return kmalloc(1);
    }
    else {
        if (size % 64 == 0) {
            return kmalloc(size / 64);
        }
        else {
            return kmalloc((size / 64) + 1);
        }
    }
}