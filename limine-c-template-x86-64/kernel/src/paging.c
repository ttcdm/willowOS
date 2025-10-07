#include <paging.h>
#include <kutils.h>
#include <scheduler.h>

#include <errno.h>

typedef struct pml4_page_struct {//not sure if we need __attribute__((packed))
    uint64_t entries[512];
} page_struct;

uint64_t starting_address;
uint8_t last_alloced_frame;
uint64_t hhdm_offset;

uint64_t alloc_frame(void) {//can only allocate usable memmaps for now
    // starting_address = memmap_arr[0].base + 300000;//first 100k is self reserved for alloc_frame()'s bitmap
    struct usable_memmaps_region* current = &memmap_arr[0];
    // while (current->next != NULL) {
    while (current != NULL) {//fix to reoccuring mistake that leads to off by one error. there's no next because we want to land on the last element, and the loop checks the next element which is the last element before jumping to it
        for (uint64_t i = 0; i < current->length / 4096; i++) {//hopefully there's no off by 1 error
            if ((current->frame_bitmap[i] == 0x00) && (current->type == 0)) {
                current->frame_bitmap[i] = 0x01;
                last_alloced_frame = i;//idek if this is even supposed to be here atp
                memset((void*)(current->base + hhdm_offset + (i * 4096)), 0x00, 4096);//clear the now initialized frame's memory
                // kprintln("page allocated successfully");
                return current->base + (i * 4096);
            }
        }
        current = current->next;
    }
	kprintln("no more frames to allocate. returning 0");
    return 0;
}

//does NOT clear the allocated section
uint64_t alloc_frame_no_clear(void) {//can only allocate usable memmaps for now
    return alloc_frame();
    // starting_address = memmap_arr[0].base + 300000;//first 100k is self reserved for alloc_frame()'s bitmap
    struct usable_memmaps_region* current = &memmap_arr[0];
    // while (current->next != NULL) {
    while (current != NULL) {//fix to reoccuring mistake that leads to off by one error. there's no next because we want to land on the last element, and the loop checks the next element which is the last element before jumping to it
        for (uint64_t i = 0; i < current->length / 4096; i++) {//hopefully there's no off by 1 error
            if ((current->frame_bitmap[i] == 0x00) && (current->type == 0)) {
                current->frame_bitmap[i] = 0x01;
                last_alloced_frame = i;//idek if this is even supposed to be here atp
                //HERE removed memset because it was causing page faults. it honestly shouldn't matter though because we don't know the page map that we're currently using so we probably shouldn't touch the memory itself
                // memset((void*)(current->base + hhdm_offset + (i * 4096)), 0x00, 4096);//clear the now initialized frame's memory
                // kprintln("page allocated successfully");
                return current->base + (i * 4096);
            }
        }
        current = current->next;
    }
	kprintln("no more frames to allocate. returning 0");
    return 0;
}

void* pmm_alloc_bytes(uint64_t size) {//make sure we're ALWAYS returning a value regardless of if it was successful or not so the depending functions don't take in undefined values
    bool irq;//HERE should probably change this because we're trying to allocate a contiguous chunks by disabling interupts but it might be really costly because this might be slow
    irq_disable_save(&irq);
    if (size == 0) {
        irq_restore(&irq);
        return NULL;//always remember to assert this
    }
    if (size <= PAGE_SIZE_DEFINED) {//always remember to set the value you're comparing size to to the intended value
        irq_restore(&irq);
        return (void*) alloc_frame_no_clear();
    }
    else {
        if (size % PAGE_SIZE_DEFINED == 0) {
            uint64_t ret = alloc_frame_no_clear();
            for (int i = 1; i < (size / PAGE_SIZE_DEFINED); i++) {
                alloc_frame_no_clear();
            }
            irq_restore(&irq);
            return (void*) ret;
        }
        else {
            uint64_t ret = alloc_frame_no_clear();
            for (int i = 1; i < (size / PAGE_SIZE_DEFINED) + 1; i++) {
                alloc_frame_no_clear();
            }
            irq_restore(&irq);
            return (void*) ret;
        }
    }

}

uint8_t* memmap_bitmap;

uint64_t alloc_frame_better(void) {
    kprintf("%d", usable_memmaps_amount);
    
    for (uint64_t i = 0; i < usable_memmaps_amount; i++) {
        // struct limine_memmap_entry* current_memmap = usable_memmaps_pointer[i];
    }


    struct usable_memmaps_region* current = &memmap_arr[0];
    while (current != NULL) {//fix to reoccuring mistake that leads to off by one error. there's no next because we want to land on the last element, and the loop checks the next element which is the last element before jumping to it
        for (uint64_t i = 0; i < current->length / 4096; i++) {
            for (uint64_t j = 0; j < 8; j++) {
                if (((current->frame_bitmap[i] << (8-j)) >> j) == 0x00) {
                }
            }
            // if ((current->frame_bitmap[i] == 0x00) && (current->type == 0)) {
            //     current->frame_bitmap[i] = 0x01;
            //     last_alloced_frame = i;//idek if this is even supposed to be here atp
            //     memset((void*)(current->base + hhdm_offset + (i * 4096)), 0x00, 4096);//clear the now initialized frame's memory
            //     // kprintln("page allocated successfully");
            //     return current->base + (i * 4096);
            // }
        }
        current = current->next;
    }
	kprintln("no more frames to allocate. returning 0");
    return 0;
}

//HERE fix this??
//only allow 4kib aligned input?? and assert that it is as well?
void free_frame(uint64_t phys_address) {//pretty sure this works. may have to align input to 4kib??
    // uint8_t index;
    // return;
    bool irq;
    irq_disable_save(&irq);
    if (phys_address & 0xfff != 0) {
        return;
    }
    struct usable_memmaps_region* current = &memmap_arr[0];
    while (current != NULL) {//sorta wastes an iteration at the beginning but oh well
        if (current->next != NULL) {
            if ((phys_address >= current->base) && (phys_address < current->next->base)) {
                if (current->frame_bitmap[(phys_address - current->base) / 4096] == 0) {
                    kprintf("free_frame(): frame already free\n");
                    while (1);
                }
                current->frame_bitmap[(phys_address - current->base) / 4096] = 0;
                static int a = 0;
                a++;
                if (a == 9) {
                    kprintf("\n%ld\n", (phys_address - current->base) / 4096);
                    // while (1);
                }
                irq_restore(&irq);
                return;
            }
            else if (phys_address > current->base + current->length) {
                current = current->next;
            }
        }
        else if (current->next == NULL) {
            current->frame_bitmap[(phys_address - current->base) / 4096] = 0;
            // kprintln_uint64((phys_address-current->base)/4096);
            irq_restore(&irq);
            return;
        }
    }
    irq_restore(&irq);
}


uint64_t virt_lookup(uint64_t virt_address) {//currently returns 0
    uint64_t pml4_index = (virt_address >> 39) & 0x1FF;
    uint64_t pdpt_index = (virt_address >> 30) & 0x1FF;
    uint64_t pd_index = (virt_address >> 21) & 0x1FF;
    uint64_t pt_index = (virt_address >> 12) & 0x1FF;
    uint64_t offset = virt_address & 0xFFF;

    kprintln("Virtual Address Breakdown:");
    kprintln_uint64(virt_address);
    kprintln("PML4 Index:"); kprintln_uint64(pml4_index);
    kprintln("PDPT Index:"); kprintln_uint64(pdpt_index);
    kprintln("PD Index:"); kprintln_uint64(pd_index);
    kprintln("PT Index:"); kprintln_uint64(pt_index);
    kprintln("Offset:"); kprintln_uint64(offset);
    return 0;
}

// void* get_cr3(void);//moved to paging.h

uint64_t pml4_address_virt_glob;

uint64_t cr3_global;
void init_paging() {
    // memmap_bitmap = usable_memmaps_pointer[0]->base;
    for (uint64_t i = 0; i < usable_memmaps_amount; i++) {
        if (usable_memmaps_pointer[i]->type == 0) {
            // memmap_bitmap = usable_memmaps_pointer[i]->base + hhdm_offset;
            break;
        }
    }
    //get total size of usable ram and adjust the memmap size accordingly

    // alloc_frame_better();
    // while (1) {}

    kprint("cr3: ");
    cr3_global = (uint64_t)get_cr3();//get_cr3() somehow returns the wrong value after this so i just set it as a variable
    kprintln_uint64(cr3_global);

    uint64_t* page = (void*) (alloc_frame() + hhdm_offset);
    // alloc_frame();//HERE not sure how much space i actually need for this. maybe call alloc_frame again for more space i guess
    uint64_t* cr3 = (void*) ((uint64_t) get_cr3() + hhdm_offset);
    for (int i = 0; i < 512; i++) {
        page[i] = cr3[i];//not 100% sure what's going on honestly
    }


    uint64_t pml4_address_phys = ((uint64_t) page) - hhdm_offset;
    pml4_address_virt_glob = (uint64_t) page;
    asm volatile ("mov %0, %%cr3" :: "r"(pml4_address_phys));
    kprintln("successfully initialized pml4");
}

uint64_t map_page(uint64_t* pml4_address, uint64_t phys_address, uint64_t virt_address, uint64_t permissions) {
    bool irq;
    irq_disable_save(&irq);
    uint64_t pml4_index = (virt_address >> 39) & 0x1FF;
    uint64_t pdpt_index = (virt_address >> 30) & 0x1FF;
    uint64_t pd_index = (virt_address >> 21) & 0x1FF;
    uint64_t pt_index = (virt_address >> 12) & 0x1FF;
    // uint64_t offset = virt_address & 0xFFF;

    page_struct* pml4 = (void*)pml4_address;
    uint64_t pml4_entry = pml4->entries[pml4_index];
    if (!(pml4_entry & 1)) {
        uint64_t new_entry = (alloc_frame()) | permissions;//we can save a lotta space by only only calling alloc_frame once and we just offset into that frame
        pml4->entries[pml4_index] = new_entry;
        pml4_entry = new_entry;
    }

    page_struct* pdpt = (page_struct*)((pml4_entry & ~0xfff) + hhdm_offset);
    uint64_t pdpt_entry = pdpt->entries[pdpt_index];
    if (!(pdpt_entry & 1)) {
        uint64_t new_entry = (alloc_frame()) | permissions;
        pdpt->entries[pdpt_index] = new_entry;
        pdpt_entry = new_entry;
    }

    page_struct* pd = (page_struct*)((pdpt_entry & ~0xfff) + hhdm_offset);
    uint64_t pd_entry = pd->entries[pd_index];
    if (!(pd_entry & 1)) {
        uint64_t new_entry = (alloc_frame()) | permissions;
        pd->entries[pd_index] = new_entry;
        pd_entry = new_entry;
    }

    page_struct* pt = (page_struct*)((pd_entry & ~0xfff) + hhdm_offset);
    uint64_t* pt_entry = &pt->entries[pt_index];
    *pt_entry = phys_address | permissions;
    asm volatile ("invlpg (%0)" :: "r" (virt_address) : "memory");

    if (scheduling_started) {
        // kprintf("%llx virt_address aa\n", virt_address);
        thread_context* t = get_current_thread();//make sure that this always returns the correct thread
        // kprintf("PID %d %d\n", t->pid, t->mappings.num_mappings);
        uint64_t ret = 0;
        for (uint64_t i = 0; i < t->mappings.max_mappings; i++) {//i think we do need the equal sign in the <= here
            if (t->mappings.mapped_virt_addresses_array[i].used == 0) {
                t->mappings.mapped_virt_addresses_array[i].virt_address = virt_address;
                t->mappings.mapped_virt_addresses_array[i].used = 1;
                //we set the flag in the wrapper function ie sys vm map
                t->mappings.num_mappings++;
                ret = i;
                break;//always remember to break when necessary
            }
        }
        if (t->mappings.num_mappings == t->mappings.max_mappings) {
            t->mappings.mapped_virt_addresses_array = (mapped_virt_addresses_t*) krealloc_byte((uint64_t*) (t->mappings.mapped_virt_addresses_array), (t->mappings.max_mappings + 8) * sizeof(mapped_virt_addresses_t));//remember to do sizeof
            // memset(t->mappings.mapped_virt_addresses_array + t->mappings.max_mappings, UINT64_MAX, 8 * sizeof(uint64_t));//we're using pointer arithmetic here
            for (uint64_t i = t->mappings.max_mappings; i < t->mappings.max_mappings + 8; i++) {
                t->mappings.mapped_virt_addresses_array[i].virt_address = 0;
                t->mappings.mapped_virt_addresses_array[i].used = 0;
                t->mappings.mapped_virt_addresses_array[i].flag = 0;
            }
            t->mappings.max_mappings += 8;
        }
        //we return the index. ik it's weird but it's the easiest way to return where the struct is so the wrapper function doesn't have to look for it again
        return ret;
    }
    irq_restore(&irq);
    return UINT64_MAX;
    // return 0ULL;
}

int map_page_bytes(uint64_t* pml4_address, uint64_t phys_address, uint64_t virt_address, uint64_t permissions, uint64_t size, uint64_t flag) {//map size bytes starting from arg
    //make sure that we always have a call to map_page() or at least do something with the thread's mapping array whenever we call this
    if (size == 0) {
        return EINVAL;//always remember to assert this
    }
    if (size <= PAGE_SIZE_DEFINED) {//always remember to set the value you're comparing size to to the intended value
        if ((virt_address & 0xfff + size) > PAGE_SIZE_DEFINED) {

        }
        uint64_t ret = map_page(pml4_address, phys_address, virt_address, permissions);
        get_current_thread()->mappings.mapped_virt_addresses_array[ret].flag = flag;//map_page returns the index for the struct
        return 0;
    }
    else {
        if (size % PAGE_SIZE_DEFINED == 0) {
            for (int i = 0; i < (size / PAGE_SIZE_DEFINED); i++) {
                uint64_t ret = map_page(pml4_address, phys_address + (i * PAGE_SIZE_DEFINED), virt_address + (i*PAGE_SIZE_DEFINED), permissions);
                get_current_thread()->mappings.mapped_virt_addresses_array[ret].flag = flag;

            }
            return 0;
        }
        else {
            for (int i = 0; i < (size / PAGE_SIZE_DEFINED) + 1; i++) {
                uint64_t ret = map_page(pml4_address, phys_address + (i * PAGE_SIZE_DEFINED), virt_address + (i*PAGE_SIZE_DEFINED), permissions);
                get_current_thread()->mappings.mapped_virt_addresses_array[ret].flag = flag;
            }
            return 0;
        }
    }
}

void unmap_page(uint64_t* pml4_address, uint64_t virt_address) {
    bool irq;
    irq_disable_save(&irq);
    // kprintf("%llx virt address\n", virt_address);
    uint64_t pml4_index = (virt_address >> 39) & 0x1FF;
    uint64_t pdpt_index = (virt_address >> 30) & 0x1FF;
    uint64_t pd_index = (virt_address >> 21) & 0x1FF;
    uint64_t pt_index = (virt_address >> 12) & 0x1FF;
    // uint64_t offset = virt_address & 0xFFF;

    page_struct* pml4 = (void*)pml4_address;
    uint64_t pml4_entry = pml4->entries[pml4_index];

    page_struct* pdpt = (page_struct*)((pml4_entry & ~0xfff) + hhdm_offset);
    uint64_t pdpt_entry = pdpt->entries[pdpt_index];

    page_struct* pd = (page_struct*)((pdpt_entry & ~0xfff) + hhdm_offset);
    uint64_t pd_entry = pd->entries[pd_index];

    page_struct* pt = (page_struct*)((pd_entry & ~0xfff) + hhdm_offset);
    free_frame(pd->entries[pd_index]);//HERE may have an issue with reallocating a freed frame but not 100% sure
    pt->entries[pt_index] = (uint64_t) NULL;
    asm volatile ("invlpg (%0)" :: "r" (virt_address) : "memory");

    if (scheduling_started) {
        thread_context* t = get_current_thread();
        for (uint64_t i = 0; i < t->mappings.max_mappings; i++) {//make sure that there's no off by 1 error
            // kprintf("%d %d\n", t->mappings.num_mappings, t->mappings.max_mappings);
            // if (t->mappings.mapped_virt_addresses[i] != UINT64_MAX) kprintf("%llx\n", t->mappings.mapped_virt_addresses[i]);
            if (t->mappings.mapped_virt_addresses_array[i].virt_address == virt_address) {
                // kprintf("%llx virt_address %d\n", virt_address, i);
                t->mappings.mapped_virt_addresses_array[i].used = 0;
                t->mappings.mapped_virt_addresses_array[i].flag = 0;
                t->mappings.mapped_virt_addresses_array[i].virt_address = 0;
                t->mappings.num_mappings--;
                break;
            }
        }
        //remember to add array shrinking as well
    }

    irq_restore(&irq);
}

void change_page_map(uint64_t* cr3, uint64_t virt_address, uint64_t permissions) {
    bool irq;
    irq_disable_save(&irq);
    // uint64_t* pml4_address = (uint64_t*) pml4_address_virt_glob;
    uint64_t* pml4_address = cr3;

    uint64_t pml4_index = (virt_address >> 39) & 0x1FF;
    uint64_t pdpt_index = (virt_address >> 30) & 0x1FF;
    uint64_t pd_index = (virt_address >> 21) & 0x1FF;
    uint64_t pt_index = (virt_address >> 12) & 0x1FF;
    // uint64_t offset = virt_address & 0xFFF;

    page_struct* pml4 = (void*)pml4_address;
    uint64_t pml4_entry = pml4->entries[pml4_index];
    pml4_entry |= permissions;
    pml4->entries[pml4_index] = pml4_entry;//HERE ALWAYS REMEMBER TO DO WRITEBACKS

    page_struct* pdpt = (page_struct*)((pml4_entry & ~0xfff) + hhdm_offset);
    uint64_t pdpt_entry = pdpt->entries[pdpt_index];
    pdpt_entry |= permissions;
    pdpt->entries[pdpt_index] = pdpt_entry;//HERE ALWAYS REMEMBER TO DO WRITEBACKS

    page_struct* pd = (page_struct*)((pdpt_entry & ~0xfff) + hhdm_offset);
    uint64_t pd_entry = pd->entries[pd_index];
    pd_entry |= permissions;
    pd->entries[pd_index] = pd_entry;//HERE ALWAYS REMEMBER TO DO WRITEBACKS

    page_struct* pt = (page_struct*)((pd_entry & ~0xfff) + hhdm_offset);
    uint64_t* pt_entry = &pt->entries[pt_index];
    *pt_entry |= permissions;

    asm volatile ("invlpg (%0)" :: "r" (virt_address) : "memory");
    irq_restore(&irq);
}

uint64_t virt_to_phys(uint64_t virt_address, uint64_t cr3) {//REMEMBER TO USE THE CORRECT CR3
    uint64_t* pml4_address = (uint64_t*) cr3;

    uint64_t pml4_index = (virt_address >> 39) & 0x1FF;
    uint64_t pdpt_index = (virt_address >> 30) & 0x1FF;
    uint64_t pd_index = (virt_address >> 21) & 0x1FF;
    uint64_t pt_index = (virt_address >> 12) & 0x1FF;
    uint64_t offset = virt_address & 0xFFF;

    page_struct* pml4 = (void*)pml4_address;
    uint64_t pml4_entry = pml4->entries[pml4_index];

    page_struct* pdpt = (page_struct*)((pml4_entry & ~0xfff) + hhdm_offset);
    uint64_t pdpt_entry = pdpt->entries[pdpt_index];

    page_struct* pd = (page_struct*)((pdpt_entry & ~0xfff) + hhdm_offset);
    uint64_t pd_entry = pd->entries[pd_index];

    page_struct* pt = (page_struct*)((pd_entry & ~0xfff) + hhdm_offset);
    uint64_t* pt_entry = &pt->entries[pt_index];

    return (uint64_t) ((uint64_t*) ((uint64_t) *pt_entry & ~0xfff) + offset);
}