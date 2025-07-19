#include <loader.h>
#include <scheduler.h>

void init_loader(vfs_fd_t* file) {
    kprintf("loader init\n");

    //so basically, for nonrelocatable elfs, we just call alloc_frame() and map it to the appropriate addresses.
    //the alloc_frame() bitmap already takes care of everything for us so it doesn't matter for virt addresses that are the same cuz alloc_frame() will put them at different phys addresses

    //HERE REMEMBER TO SWAP OUT CR3

    //we should probably copy out or save cr3 before we actually run anything for the first time

    // Elf64_Ehdr* ehdr = file->data;
    // for (uint64_t i = 0; i < ehdr->e_phnum; i++) {
    //     Elf64_Phdr* phdr = (void*) ((ehdr->e_phoff + (i * ehdr->e_phentsize)) + (uint64_t) file->data);//not sure if casting to void instead of the actual thing is the proper way to do it
    //     if (phdr->p_type == 1)  {//PT_LOAD
    //         uint64_t segment_start;
    //         if (phdr->p_memsz < 4096) {
    //             segment_start = alloc_frame();
    //             map_page((uint64_t*) cr3, segment_start, (uint64_t) phdr->p_vaddr, 0b111);

    //             change_page_map((uint64_t) phdr->p_vaddr, 0b111);//HERE we have to change page map to also make sure that the parent entries are also mapped with the same permissions. ALWAYS REMEMBER TO CHECK THE PARENT ENTRIES
    //         }
    //         else if (phdr->p_memsz % 4096 == 0) {//it doesn't matter that the physical frames aren't contiguous because the virtual addresses are contiguous and it takes care of it so you can just memcpy everything in one go
    //             segment_start = alloc_frame();
    //             map_page((uint64_t*) cr3, segment_start, (uint64_t) phdr->p_vaddr, 0b111);

    //             change_page_map((uint64_t) phdr->p_vaddr, 0b111);

    //             for (uint64_t j = 0; j < ((phdr->p_memsz / 4096) - 1); j++) {
    //                 uint64_t next_frame = alloc_frame();
    //                 map_page((uint64_t*) cr3, next_frame, (uint64_t) phdr->p_vaddr + (4096 * (j + 1)), 0b111);

    //                 change_page_map((uint64_t) phdr->p_vaddr + (4096 * (j + 1)), 0b111);
    //             }
    //         }
    //         else if ((phdr->p_memsz % 4096 != 0) && (phdr->p_memsz > 4096)) {//i think this overlaps with the if block above it
    //             segment_start = alloc_frame();
    //             map_page((uint64_t*) cr3, segment_start, (uint64_t) phdr->p_vaddr, 0b111);

    //             change_page_map((uint64_t) phdr->p_vaddr, 0b111);

    //             for (uint64_t j = 1; j < (phdr->p_memsz / 4096) + 1; j++) {
    //                 uint64_t next_frame = alloc_frame();
    //                 map_page((uint64_t*) cr3, next_frame, (uint64_t) phdr->p_vaddr + (4096 * j), 0b111);

    //                 change_page_map((uint64_t) phdr->p_vaddr + (4096 * j), 0b111);
    //             }
    //         }

    //         //copy from file data + offset of code contents or something from the start of the file and we copy that to p_vaddr
    //         //HERE remember to reread the docs and calculate the offsets correctly
    //         memcpy((void*) phdr->p_vaddr, (void*) (((uint64_t) file->data) + phdr->p_offset), phdr->p_filesz);

    //     }

    // }

    // hot_exec_elf(57, (void*) ehdr->e_entry);


    hot_create_and_push_thread(103, gen2);
    hot_create_and_push_thread(104, gen2);
    hot_create_and_push_thread(105, gen2);
    // hot_exec_elf(11, test_a);

    for (int i = 0; i < 15; i++) {
        hot_exec_elf(i, test_a);
        // hot_create_and_push_thread(i, test_a);
        // hot_create_and_push_thread(i, gen2);
    }
    for (int i = 100; i < 200; i++) {
        // hot_create_and_push_thread(i, gen2);
    }
    // hot_create_and_push_thread(17, gen2);
    // hot_exec_elf(16, test_a);


    while (1) reschedule();
    // reschedule();
    // while (1) hot_reschedule();

}

void* load_elf(vfs_fd_t* file, uint64_t cr3) {

    //HERE REMEMBER TO FREE THE PHYSICAL FRAMES WHEN YOU'RE DONE

    Elf64_Ehdr* ehdr = file->data;
    for (uint64_t i = 0; i < ehdr->e_phnum; i++) {
        Elf64_Phdr* phdr = (void*) ((ehdr->e_phoff + (i * ehdr->e_phentsize)) + (uint64_t) file->data);//not sure if casting to void instead of the actual thing is the proper way to do it
        if (phdr->p_type == 1)  {//PT_LOAD
            uint64_t segment_start;
            if (phdr->p_memsz < 4096) {
                segment_start = alloc_frame();
                map_page((uint64_t*) cr3, segment_start, (uint64_t) phdr->p_vaddr, 0b111);

                change_page_map((uint64_t*) cr3, (uint64_t) phdr->p_vaddr, 0b111);//HERE we have to change page map to also make sure that the parent entries are also mapped with the same permissions. ALWAYS REMEMBER TO CHECK THE PARENT ENTRIES
            }
            else if (phdr->p_memsz % 4096 == 0) {//it doesn't matter that the physical frames aren't contiguous because the virtual addresses are contiguous and it takes care of it so you can just memcpy everything in one go
                segment_start = alloc_frame();
                map_page((uint64_t*) cr3, segment_start, (uint64_t) phdr->p_vaddr, 0b111);

                change_page_map((uint64_t*) cr3, (uint64_t) phdr->p_vaddr, 0b111);

                for (uint64_t j = 0; j < ((phdr->p_memsz / 4096) - 1); j++) {
                    uint64_t next_frame = alloc_frame();
                    map_page((uint64_t*) cr3, next_frame, (uint64_t) phdr->p_vaddr + (4096 * (j + 1)), 0b111);

                    change_page_map((uint64_t*) cr3, (uint64_t) phdr->p_vaddr + (4096 * (j + 1)), 0b111);
                }
            }
            else if ((phdr->p_memsz % 4096 != 0) && (phdr->p_memsz > 4096)) {//i think this overlaps with the if block above it
                segment_start = alloc_frame();
                map_page((uint64_t*) cr3, segment_start, (uint64_t) phdr->p_vaddr, 0b111);

                change_page_map((uint64_t*) cr3, (uint64_t) phdr->p_vaddr, 0b111);

                for (uint64_t j = 1; j < (phdr->p_memsz / 4096) + 1; j++) {
                    uint64_t next_frame = alloc_frame();
                    map_page((uint64_t*) cr3, next_frame, (uint64_t) phdr->p_vaddr + (4096 * j), 0b111);

                    change_page_map((uint64_t*) cr3, (uint64_t) phdr->p_vaddr + (4096 * j), 0b111);
                }
            }

            //copy from file data + offset of code contents or something from the start of the file and we copy that to p_vaddr
            //HERE remember to reread the docs and calculate the offsets correctly
            memcpy((void*) phdr->p_vaddr, (void*) (((uint64_t) file->data) + phdr->p_offset), phdr->p_filesz);

        }

    }

    return (void*) ehdr->e_entry;
}

void unload_elf(vfs_fd_t* file, uint64_t cr3) {
    
    //HERE REMEMBER TO FREE THE PHYSICAL FRAMES WHEN YOU'RE DONE

    Elf64_Ehdr* ehdr = file->data;
    for (uint64_t i = 0; i < ehdr->e_phnum; i++) {
        Elf64_Phdr* phdr = (void*) ((ehdr->e_phoff + (i * ehdr->e_phentsize)) + (uint64_t) file->data);//not sure if casting to void instead of the actual thing is the proper way to do it
        if (phdr->p_type == 1)  {//PT_LOAD
            if (phdr->p_memsz < 4096) {
                free_frame(virt_to_phys(phdr->p_vaddr, cr3));//need to free before we unmap because we need to still be able to translate it
                unmap_page((uint64_t*) cr3, (uint64_t) phdr->p_vaddr);
            }
            else if (phdr->p_memsz % 4096 == 0) {//it doesn't matter that the physical frames aren't contiguous because the virtual addresses are contiguous and it takes care of it so you can just memcpy everything in one go
                free_frame(virt_to_phys(phdr->p_vaddr, cr3));
                unmap_page((uint64_t*) cr3, (uint64_t) phdr->p_vaddr);

                for (uint64_t j = 0; j < ((phdr->p_memsz / 4096) - 1); j++) {
                    free_frame(virt_to_phys((uint64_t) phdr->p_vaddr + (4096 * (j + 1)), cr3));
                    unmap_page((uint64_t*) cr3, (uint64_t) phdr->p_vaddr + (4096 * (j + 1)));
                }
            }
            else if ((phdr->p_memsz % 4096 != 0) && (phdr->p_memsz > 4096)) {//i think this overlaps with the if block above it
                free_frame(virt_to_phys(phdr->p_vaddr, cr3));
                unmap_page((uint64_t*) cr3, (uint64_t) phdr->p_vaddr);

                for (uint64_t j = 1; j < (phdr->p_memsz / 4096) + 1; j++) {
                    free_frame(virt_to_phys((uint64_t) phdr->p_vaddr + (4096 * j), cr3));
                    unmap_page((uint64_t*) cr3, (uint64_t) phdr->p_vaddr + (4096 * j));
                }
            }
        }

    }
    kprintf_interruptable("unloaded elf\n");
}

void userspace_run_elf() {//HERE i think it's okay if this gets interrupted but i'm not 100% sure
    asm volatile ("cli");
    volatile thread_context* t = get_current_thread();
    // t->stack_base = (uint64_t*) (((uint64_t) kmalloc_byte_interruptable(THREAD_STACK_SIZE)) + THREAD_STACK_SIZE);
    if (t->elf_entry != NULL) {//i should probably directly pass it in instead of getting the current thread some other way
        for (size_t i = 0; i < 5; i++) {
            //maybe i should have a stack in the lower half only but idk
            change_page_map(t->cr3, (((uint64_t) t->stack_base) - THREAD_STACK_SIZE) + (i*4000), 0b111);
        }

        t->status[4] = 1;

        
        // kprintf_interruptable("\npid: %d\n", t->pid);
        // jump_to_user(t->elf_entry, (void*) (((uint64_t) t->stack_base) - THREAD_STACK_SIZE));
        jump_to_user(t->elf_entry, t->stack_base);

    }
    else {
        // kprintf_interruptable("no valid elf entry to execute");
        //no else because we also use this function to run functions in userspace
    }
    asm volatile ("sti");
}