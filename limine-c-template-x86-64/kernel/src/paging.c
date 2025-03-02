#include <paging.h>
#include <kutils.h>





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


uintptr_t pml4;
uintptr_t pdpt;
uintptr_t pd;
uintptr_t pt;

void map_page(uint64_t address) {
    
}

void walk_page(uintptr_t structure) {

}






uint8_t frame_bitmap[10000];//10000*4096 = 40.96 mb of memory. make sure it doesn't exceed the max amount of allocatable memory


//remember to prevent overflows and underflows. make this more secure
uint64_t starting_address;
uint8_t last_alloced_frame;
uint64_t alloc_frame(void) {//not sure how this would be used with hhdm yet
    starting_address = (*usable_memmaps_1_ptr)->base;
    for (int i = 0; i < 10000; i++) {
        if (frame_bitmap[i] == 0x00) {
            frame_bitmap[i] = 0x01;
            last_alloced_frame = i;
            return starting_address + (i * 4096);
        }
    }
}

void free_frame(void) {//kind of a stack allocator
    if (last_alloced_frame > (uint8_t) 0) {//to prevent 
        frame_bitmap[last_alloced_frame] = 0x00;
        last_alloced_frame--;
    }
}