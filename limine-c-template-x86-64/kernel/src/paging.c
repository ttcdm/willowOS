#include <paging.h>
#include <kutils.h>


typedef struct pml4_page_struct {//not sure if we need __attribute__((packed))
    uint64_t entries[512];
} page_struct;

uint64_t starting_address;
uint8_t last_alloced_frame;
uint64_t hhdm_offset;

uint64_t alloc_frame(void) {//can only allocate usable memmaps for now
    starting_address = memmap_arr[0].base;
    struct usable_memmaps_region* current = &memmap_arr[0];
    // while (current->next != NULL) {
    while (current != NULL) {//fix to reoccuring mistake that leads to off by one error. there's no next because we want to land on the last element, and the loop checks the next element which is the last element before jumping to it
        for (int i = 0; i < current->length / 4096; i++) {//hopefully there's no off by 1 error
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


void free_frame(uint64_t phys_address) {//pretty sure this works
    uint8_t index;
    struct usable_memmaps_region* current = &memmap_arr[0];
    while (current != NULL) {//sorta wastes an iteration at the beginning but oh well
        if (current->next != NULL) {
            if ((phys_address > current->base) && (phys_address < current->next->base)) {
                current->frame_bitmap[(phys_address - current->base) / 4096] = 0;
                return;
            }
            else if (phys_address > current->base) {
                current = current->next;
            }
        }
        else if (current->next == NULL) {
            current->frame_bitmap[(phys_address - current->base) / 4096] = 0;
            // kprintln_uint64((phys_address-current->base)/4096);
            return;
        }
    }
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
    kprint("cr3: ");
    cr3_global = (uint64_t)get_cr3();//get_cr3() somehow returns the wrong value after this so i just set it as a variable
    kprintln_uint64(cr3_global);

    uint64_t* page = (void*)alloc_frame() + hhdm_offset;
    uint64_t* cr3 = (void*)get_cr3() + hhdm_offset;
    for (int i = 0; i < 512; i++) {
        page[i] = cr3[i];//not 100% sure what's going on honestly
    }

    uint64_t pml4_address_phys = (uint64_t) page - hhdm_offset;
    pml4_address_virt_glob = (uint64_t) page;
    asm volatile ("mov %0, %%cr3" :: "r"(pml4_address_phys));
    kprintln("successfully initialized pml4");
}

void map_page(uint64_t* pml4_address, uint64_t phys_address, uint64_t virt_address, uint64_t permissions) {
    uint64_t pml4_index = (virt_address >> 39) & 0x1FF;
    uint64_t pdpt_index = (virt_address >> 30) & 0x1FF;
    uint64_t pd_index = (virt_address >> 21) & 0x1FF;
    uint64_t pt_index = (virt_address >> 12) & 0x1FF;
    uint64_t offset = virt_address & 0xFFF;

    page_struct* pml4 = (void*)pml4_address;
    uint64_t pml4_entry = pml4->entries[pml4_index];
    if (!(pml4_entry & 1)) {
        uint64_t new_entry = (alloc_frame()) | 0b11;
        pml4->entries[pml4_index] = new_entry;
        pml4_entry = new_entry;
    }

    page_struct* pdpt = (page_struct*)((pml4_entry & ~0xfff) + hhdm_offset);
    uint64_t pdpt_entry = pdpt->entries[pdpt_index];
    if (!(pdpt_entry & 1)) {
        uint64_t new_entry = (alloc_frame()) | 0b11;
        pdpt->entries[pdpt_index] = new_entry;
        pdpt_entry = new_entry;
    }

    page_struct* pd = (page_struct*)((pdpt_entry & ~0xfff) + hhdm_offset);
    uint64_t pd_entry = pd->entries[pd_index];
    if (!(pd_entry & 1)) {
        uint64_t new_entry = (alloc_frame()) | 0b11;
        pd->entries[pd_index] = new_entry;
        pd_entry = new_entry;
    }

    page_struct* pt = (page_struct*)((pd_entry & ~0xfff) + hhdm_offset);
    pt->entries[pt_index] = phys_address | permissions;
    asm volatile ("invlpg (%0)" :: "r" (virt_address) : "memory");
}

void unmap_page(uint64_t* pml4_address, uint64_t virt_address) {
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
    free_frame(pd->entries[pd_index]);//HERE may have an issue with reallocating a freed frame but not 100% sure
    pt->entries[pt_index] = NULL;
    asm volatile ("invlpg (%0)" :: "r" (virt_address) : "memory");
}