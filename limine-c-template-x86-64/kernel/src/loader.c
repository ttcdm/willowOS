#include <loader.h>

void init_loader(vfs_fd_t* file) {
    kprintf("loader init\n\n");

    //so basically, for nonrelocatable elfs, we just call alloc_frame() and map it to the appropriate addresses.
    //the alloc_frame() bitmap already takes care of everything for us so it doesn't matter for virt addresses that are the same cuz alloc_frame() will put them at different phys addresses

    //HERE REMEMBER TO SWAP OUT CR3

    //we should probably copy out or save cr3 before we actually run anything for the first time

    Elf64_Ehdr* ehdr = file->data;
    for (uint64_t i = 0; i < ehdr->e_phnum; i++) {
        Elf64_Phdr* phdr = (ehdr->e_phoff + (i * ehdr->e_phentsize)) + (uint64_t) file->data;
        if (phdr->p_type == 1)  {//PT_LOAD
            uint64_t segment_start;
            if (phdr->p_memsz < 4096) {
                segment_start = alloc_frame();
                map_page((uint64_t*) pml4_address_virt_glob, segment_start, (uint64_t) phdr->p_vaddr, 0b111);
            }
            else if (phdr->p_memsz % 4096 == 0) {//it doesn't matter that the physical frames aren't contiguous because the virtual addresses are contiguous and it takes care of it so you can just memcpy everything in one go
                segment_start = alloc_frame();
                map_page((uint64_t*) pml4_address_virt_glob, segment_start, (uint64_t) phdr->p_vaddr, 0b111);
                for (uint64_t j = 0; j < ((phdr->p_memsz / 4096) - 1); j++) {
                    uint64_t next_frame = alloc_frame();
                    map_page((uint64_t*) pml4_address_virt_glob, next_frame, (uint64_t) phdr->p_vaddr + (4096 * (j + 1)), 0b111);

                }
            }
            else if ((phdr->p_memsz % 4096 != 0) && (phdr->p_memsz > 4096)) {//i think this overlaps with the if block above it
                segment_start = alloc_frame();
                map_page((uint64_t*) pml4_address_virt_glob, segment_start, (uint64_t) phdr->p_vaddr, 0b111);
                for (uint64_t j = 1; j < (phdr->p_memsz / 4096) + 1; j++) {
                    uint64_t next_frame = alloc_frame();
                    map_page((uint64_t*) pml4_address_virt_glob, next_frame, (uint64_t) phdr->p_vaddr + (4096 * j), 0b111);
                }
            }

            memcpy((void*) phdr->p_vaddr, (void*) ((ehdr->e_phoff + (i * ehdr->e_phentsize)) + (uint64_t) file->data), phdr->p_filesz);
        }

    }

    // asm volatile ("jmp %0" : : "r" (ehdr->e_entry));
    
}