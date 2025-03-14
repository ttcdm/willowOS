#include <paging.h>
#include <kutils.h>


typedef struct pml4_page_struct {//not sure if we need __attribute__((packed))
    uint64_t entries[512];
} page_struct;


//use hhdm

/*
so cr3 gives a pointer to an array of pml4 entries.
each entry has a pointer to an array of pdpt entries.
each pdpt entry has a pointer to an array of page directory entries.
each page directory entry has a pointer to an array of page table entries
each page table entry has a pointer to a frame

so now i use the hhdm to map the cr3 address to an actual address
with that address, i can access the array of pml4 entries
after the array of pml4 entries, i can offset the original cr3 address create another array in physical memory to store the pdpt entries
and then after that i do the same thing for each page dir entry
and then after that i do the same thing for each page table entry
like |pml4[512]|pdpt[512^2]|page_dir[512^3]|page_table[512^4]|

and then to allocate a page, i just have a gigantic bitmap (for now) to store each individual frame
*/


/*
You're mixing up what goes into the entry with how the CPU calculates the physical address of the entry. You've also skipped one level.

Each entry contains bits 51:12 of the physical address of the next level's table in bits 51:12, and various other data in the remaining bits.

CR3 contains the physical address of a PML5 or a PML4 depending on CR4.

Each PML5 contains 512 PML5Es, and each PML5E contains the physical address of a PML4.
Each PML4 contains 512 PML4Es, and each PML4E contains the physical address of a PDPT.
Each PDPT contains 512 PDPTEs, and each PDPTE contains the physical address of a PD.
Each PD contains 512 PDEs, and each PDE contains the physical address of a PT.
Each PT contains 512 PTEs, and each PTE contains the physical address of a 4kiB page.

It's easier to follow if you throw out Intel's dumb names and call them PML3, PML2, and PML1 instead of PDPT, PD, and PT.
*/


//uintptr_t pml4;
//uintptr_t pdpt;
//uintptr_t pd;
//uintptr_t pt;


void walk_page(uintptr_t structure) {

}

//uint8_t frame_bitmap[10000];//10000*4096 = 40.96 mb of memory. make sure it doesn't exceed the max amount of allocatable memory
//remember to prevent overflows and underflows. make this more secure
uint64_t starting_address;
uint8_t last_alloced_frame;
uint64_t hhdm_offset;


//init_physical_memory() is put in main.c because hhdm_request is static and can only be used in main.c i think

//uint64_t alloc_frame(void) {//not sure how this would be used with hhdm yet
//    //starting_address = (*usable_memmaps_1_ptr)->base;//might need to align to 4096
//    for (int i = 0; i < 10000; i++) {
//        if (frame_bitmap[i] == 0x00) {
//            frame_bitmap[i] = 0x01;
//            last_alloced_frame = i;
//            return starting_address + (i * 4096);
//        }
//    }
//    kprint("no more frames to allocate. returning last allocated frame's address\n");
//    return starting_address + (last_alloced_frame * 4096);//returns an address instead of a pointer
//}

uint64_t alloc_frame(void) {
    starting_address = memmap_arr[0].base;
    struct usable_memmaps_region* current = &memmap_arr[0];
    while (current->next != NULL) {
        for (int i = 0; i < current->length/4096; i++) {//hopefully there's no off by 1 error
            if (current->frame_bitmap[i] == 0x00) {
				current->frame_bitmap[i] = 0x01;
				last_alloced_frame = i;//idek if this is even supposed to be here atp
                kprint("current region: ");
                kprintln_uint64(current->base);
                kprint("index: ");
                kprintln_uint64(i);//i'm honestly not sure why the 0th index is 1. it's set to 0 when it's first initialized
                memset((void*)(current->base + hhdm_offset + (i * 4096)), 0x00, 4096);//clear the now initialized frame's memory
				return current->base + (i * 4096);
			}
		}
        current = current->next;
    }
	kprint("no more frames to allocate. returning last allocated frame's address\n");
    return 0;
}

//void free_frame(void) {//kind of a stack allocator
//    if (last_alloced_frame > (uint8_t)0) {//to prevent 
//        frame_bitmap[last_alloced_frame] = 0x00;
//        last_alloced_frame--;
//    }
//    else if (last_alloced_frame == (uint8_t)0) {
//        kprint("no more frames to free\n");
//    }
//}

void free_frame(uint64_t address) {//right now, it's using an address instead of a pointer; uses physical address
    //uint8_t index = (address - hhdm_offset - starting_address) / 4096;//revert hhdm
    uint8_t index = (address - starting_address) / 4096;//uses physical address
    kprint("index: ");
    kprint_uint64(index);
    kprint("\n");
    //frame_bitmap[index] = 0x00;
    //HERE
}

//void init_paging() {//right now, i'm loading the pml4 address into the already provided address in cr3
//    //pml4 = (uint64_t*) alloc_frame();
//    kprintln("allocating pml4");
//    //__asm__ volatile ("mov %%cr3, %0" : "=r"(pml4));//AT&T syntax so it's [src] [dest]
//    //for (int i = 0; i < 512; i++) pml4[i] = 0;
//    //asm volatile ("mov %0, %%cr3" :: "r"(pml4));
//    kprintln("pml4:");
//    //kprintln_uint64(pml4);
//    //kprintln_uint64(pml4[0]);
//}

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

void* get_cr3(void);
void init_paging() {
    //pml4_address = 0;//initialize memory
    //kprintln("allocating pml4");
    //asm volatile ("mov %%cr3, %0" : "=r"(pml4_address));//=r for output register and r for input register
    //pml4_address += hhdm_offset;
    //pml4 = (uint64_t*)pml4_address;

    
    //uint64_t pml4_address;
    //uint64_t* pml4;
    //pml4_address = alloc_frame() & ~0xfff;
    //
    ////asm volatile ("mov %%cr3, %0" : "=r"(pml4_address));//=r for output register and r for input register
    ////__asm__ volatile ("mov %%cr3, %0" : "=r"(pml4_address));

    //uint64_t pml4_address_phys = pml4_address;
    //pml4_address += hhdm_offset;
    //pml4 = (uint64_t*)pml4_address;
    //kprintln("pml4:");
    //kprintln_uint64(pml4);
    ////for (int i = 0; i < 512; i++) pml4[i] = 0;
    ////for (int i = 0; i < 512; i++) kprintln_uint64(pml4[i]);

    //virt_lookup(pml4_address);
    //kprintln_uint64(pml4_address-hhdm_offset);




    //uint64_t self_map_addr = (uint64_t)511 << 39;
    //uint64_t pml4_address_phys = (uint64_t) page;
    //kprintln("mapping pml4");
    //map_page(page, pml4_address_phys, self_map_addr, 0b11);//maps pml4 to itself
    //kprintln("pml4 mapped to itself");

    //kprintln("cr3 loaded successfully");
    //kprintln_uint64(pml4_address_phys);
    //asm volatile ("invlpg (%0)" :: "r" (self_map_addr) : "memory");


    kprint("cr3:   ");
    kprintln_uint64((uint64_t)get_cr3());


    uint64_t* page = (void*)alloc_frame() + hhdm_offset;
    uint64_t* cr3 = (void*)get_cr3() + hhdm_offset;
    for (int i = 0; i < 512; i++) {
        page[i] = cr3[i];
    }
    map_page(((void*)page), 0x1000, 0x100000, 0b11);
    kprintln("hi");
    uint64_t pml4_address_phys = (uint64_t) page - hhdm_offset;
    asm volatile ("mov %0, %%cr3" :: "r"(pml4_address_phys));
    kprintln("bye");




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



void map_page_bad(uint64_t* pml4_address, uint64_t phys_address, uint64_t virt_address, uint64_t permissions) {//tried debugging with chatgpt. doesn't work. don't use
    uint64_t pml4_index = (virt_address >> 39) & 0x1FF;
    uint64_t pdpt_index = (virt_address >> 30) & 0x1FF;
    uint64_t pd_index = (virt_address >> 21) & 0x1FF;
    uint64_t pt_index = (virt_address >> 12) & 0x1FF;
    uint64_t offset = virt_address & 0xFFF;

    page_struct* pml4 = (void*) pml4_address;

    pml4->entries[511] =  ((((uint64_t)pml4_address)-hhdm_offset) & ~0xfff) | 0b11;

    uint64_t pml4_entry = pml4->entries[pml4_index];
    if (!(pml4_entry & 1)) {
        uint64_t new_entry = (alloc_frame() & ~0xfff) | 0b11;
        pml4->entries[pml4_index] = new_entry;
        pml4_entry = (new_entry & 0x000FFFFFFFFFF000) + hhdm_offset;
    }
    // page_struct* pdpt = (page_struct*) ((pml4_entry & ~0xfff) + hhdm_offset);
    page_struct* pdpt = (page_struct*) pml4_entry;
    uint64_t pdpt_entry = pdpt->entries[pdpt_index];
	if (!(pdpt_entry & 1)) {
        uint64_t new_entry = (alloc_frame() & ~0xfff) | 0b11;
        pdpt->entries[pdpt_index] = new_entry;
        pdpt_entry = (new_entry & 0x000FFFFFFFFFF000) + hhdm_offset;
	}
	// page_struct* pd = (page_struct*) ((pdpt_entry & ~0xfff) + hhdm_offset);
    page_struct* pd = (page_struct*) pdpt_entry;
	uint64_t pd_entry = pd->entries[pd_index];
    if (!(pd_entry & 1)) {
        uint64_t new_entry = (alloc_frame() & ~0xfff) | 0b11;
        pd->entries[pd_index] = new_entry;
		pd_entry = (new_entry & 0x000FFFFFFFFFF000) + hhdm_offset;
    }
    
	// page_struct* pt = (page_struct*) ((pd_entry & ~0xfff) + hhdm_offset);
    page_struct* pt = (page_struct*) pd_entry;
    pt->entries[pt_index] = (phys_address & ~0xfff) | permissions;
    asm volatile ("invlpg (%0)" :: "r" (virt_address) : "memory");
}
