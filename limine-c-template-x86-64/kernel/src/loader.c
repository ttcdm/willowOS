#include <loader.h>
#include <scheduler.h>

void init_loader(vfs_fd_t* file) {
    kprintf("loader init\n");

    //so basically, for nonrelocatable elfs, we just call alloc_frame() and map it to the appropriate addresses.
    //the alloc_frame() bitmap already takes care of everything for us so it doesn't matter for virt addresses that are the same cuz alloc_frame() will put them at different phys addresses

    //HERE REMEMBER TO SWAP OUT CR3

    //we should probably copy out or save cr3 before we actually run anything for the first time

    //vvv this is out of date so don't use vvv

    // Elf64_Ehdr* ehdr = file->data;
    // for (uint64_t i = 0; i < ehdr->e_phnum; i++) {
    //     Elf64_Phdr* phdr = (void*) ((ehdr->e_phoff + (i * ehdr->e_phentsize)) + (uint64_t) file->data);//not sure if casting to void instead of the actual thing is the proper way to do it
    //     if (phdr->p_type == 1)  {//PT_LOAD
    //         uint64_t segment_start;
    //         if (phdr->p_filesz < 4096) {
    //             segment_start = alloc_frame();
    //             map_page((uint64_t*) cr3, segment_start, (uint64_t) phdr->p_vaddr, 0b111);

    //             change_page_map((uint64_t) phdr->p_vaddr, 0b111);//HERE we have to change page map to also make sure that the parent entries are also mapped with the same permissions. ALWAYS REMEMBER TO CHECK THE PARENT ENTRIES
    //         }
    //         else if (phdr->p_filesz % 4096 == 0) {//it doesn't matter that the physical frames aren't contiguous because the virtual addresses are contiguous and it takes care of it so you can just memcpy everything in one go
    //             segment_start = alloc_frame();
    //             map_page((uint64_t*) cr3, segment_start, (uint64_t) phdr->p_vaddr, 0b111);

    //             change_page_map((uint64_t) phdr->p_vaddr, 0b111);

    //             for (uint64_t j = 0; j < ((phdr->p_filesz / 4096) - 1); j++) {
    //                 uint64_t next_frame = alloc_frame();
    //                 map_page((uint64_t*) cr3, next_frame, (uint64_t) phdr->p_vaddr + (4096 * (j + 1)), 0b111);

    //                 change_page_map((uint64_t) phdr->p_vaddr + (4096 * (j + 1)), 0b111);
    //             }
    //         }
    //         else if ((phdr->p_filesz % 4096 != 0) && (phdr->p_filesz > 4096)) {//i think this overlaps with the if block above it
    //             segment_start = alloc_frame();
    //             map_page((uint64_t*) cr3, segment_start, (uint64_t) phdr->p_vaddr, 0b111);

    //             change_page_map((uint64_t) phdr->p_vaddr, 0b111);

    //             for (uint64_t j = 1; j < (phdr->p_filesz / 4096) + 1; j++) {
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


            //just pass in the actual address instead of the aligned one because we need the offset if it's misaligned because map_range() needs it because it does the alignment for us
            map_range((uint64_t*) cr3, (uint64_t) (phdr->p_vaddr), 0b111, phdr->p_memsz, 0b111);

            /*
            uint64_t segment_start;
            if (phdr->p_memsz < 4096) {
                segment_start = alloc_frame();
                //can't call get_current_thread()->pid here because scheduling might not have started yet. it currently hasn't
                map_page((uint64_t*) cr3, segment_start, (uint64_t) (phdr->p_vaddr), 0b111);

                change_page_map((uint64_t*) cr3, (uint64_t) (phdr->p_vaddr), 0b111);//HERE we have to change page map to also make sure that the parent entries are also mapped with the same permissions. ALWAYS REMEMBER TO CHECK THE PARENT ENTRIES
            }
            else if (phdr->p_memsz % 4096 == 0) {//it doesn't matter that the physical frames aren't contiguous because the virtual addresses are contiguous and it takes care of it so you can just memcpy everything in one go
                segment_start = alloc_frame();
                map_page((uint64_t*) cr3, segment_start, (uint64_t) (phdr->p_vaddr), 0b111);

                change_page_map((uint64_t*) cr3, (uint64_t) (phdr->p_vaddr), 0b111);
                
                for (uint64_t j = 1; j < (((phdr->p_vaddr & 0xfff) + phdr->p_memsz) / 4096); j++) {
                    uint64_t next_frame = alloc_frame();
                    map_page((uint64_t*) cr3, next_frame, (uint64_t) (phdr->p_vaddr) + (4096 * (j)), 0b111);

                    change_page_map((uint64_t*) cr3, (uint64_t) (phdr->p_vaddr) + (4096 * (j)), 0b111);
                }
            }
            else if ((phdr->p_memsz % 4096 != 0) && (phdr->p_memsz > 4096)) {//i think this overlaps with the if block above it
                segment_start = alloc_frame();
                map_page((uint64_t*) cr3, segment_start, (uint64_t) (phdr->p_vaddr), 0b111);

                change_page_map((uint64_t*) cr3, (uint64_t) (phdr->p_vaddr), 0b111);
                
                //HERE fixed the off by one error because p_vaddr isn't 4kib aligned. we need to get the full size from the nearest page down

                for (uint64_t j = 1; j < (((phdr->p_vaddr & 0xfff) + phdr->p_memsz) / 4096) + 1; j++) {
                    uint64_t next_frame = alloc_frame();
                    map_page((uint64_t*) cr3, next_frame, (uint64_t) (phdr->p_vaddr) + (4096 * j), 0b111);

                    change_page_map((uint64_t*) cr3, (uint64_t) (phdr->p_vaddr) + (4096 * j), 0b111);

                }
            }
            */

            //copy from file data + offset of code contents or something from the start of the file and we copy that to p_vaddr
            //HERE remember to reread the docs and calculate the offsets correctly
            // memset((void*) phdr->p_vaddr, 0, phdr->p_memsz);
            memset((void*) (phdr->p_vaddr & ~0xfff), 0, phdr->p_memsz + (phdr->p_vaddr & 0xfff));
            memcpy((void*) phdr->p_vaddr, (void*) (((uint64_t) file->data) + phdr->p_offset), phdr->p_filesz);

            // map_page((uint64_t*) cr3, (uint64_t) phdr->p_vaddr + phdr->p_filesz, (uint64_t) phdr->p_vaddr + phdr->p_filesz, 0b111);
            // change_page_map((uint64_t*) cr3, (uint64_t) phdr->p_vaddr + phdr->p_filesz, 0b111);
            if (phdr->p_memsz > phdr->p_filesz) {
                // memset((void*) ((uint64_t)phdr->p_vaddr + phdr->p_filesz), 0, 1);

                kprintf("%llx %llx\n", phdr->p_vaddr + phdr->p_filesz, phdr->p_memsz - phdr->p_filesz);
                kprintf("%llx %llx\n", phdr->p_filesz, phdr->p_memsz);
                memset((void*) (((uint64_t)phdr->p_vaddr + phdr->p_filesz)), 0, phdr->p_memsz - phdr->p_filesz);

            }
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
                free_frame(virt_to_phys_page(phdr->p_vaddr, cr3));//need to free before we unmap because we need to still be able to translate it
                unmap_page((uint64_t*) cr3, (uint64_t) phdr->p_vaddr);
            }
            else if (phdr->p_memsz % 4096 == 0) {//it doesn't matter that the physical frames aren't contiguous because the virtual addresses are contiguous and it takes care of it so you can just memcpy everything in one go
                free_frame(virt_to_phys_page(phdr->p_vaddr, cr3));
                unmap_page((uint64_t*) cr3, (uint64_t) phdr->p_vaddr);
                for (uint64_t j = 0; j < ((((phdr->p_vaddr & 0xfff) + phdr->p_memsz) / 4096) - 1); j++) {
                    free_frame(virt_to_phys_page((uint64_t) phdr->p_vaddr + (4096 * (j + 1)), cr3));
                    unmap_page((uint64_t*) cr3, (uint64_t) phdr->p_vaddr + (4096 * (j + 1)));
                }
            }
            else if ((phdr->p_memsz % 4096 != 0) && (phdr->p_memsz > 4096)) {//i think this overlaps with the if block above it
                free_frame(virt_to_phys_page(phdr->p_vaddr, cr3));
                unmap_page((uint64_t*) cr3, (uint64_t) phdr->p_vaddr);
                for (uint64_t j = 1; j < (((phdr->p_vaddr & 0xfff) + phdr->p_memsz) / 4096) + 1; j++) {
                    free_frame(virt_to_phys_page((uint64_t) phdr->p_vaddr + (4096 * j), cr3));
                    unmap_page((uint64_t*) cr3, (uint64_t) phdr->p_vaddr + (4096 * j));
                }
            }
        }

    }

    kprintf_interruptable("unloaded elf\n");
}


//this function is for getting both elfs and whatever functions/stuff you want into userspace
void userspace_run_elf() {//HERE i think it's okay if this gets interrupted but i'm not 100% sure
    // asm volatile ("cli");
    bool irq;
    irq_disable_save(&irq);
    volatile thread_context* t = get_current_thread();
    
    assert(t != NULL);
    //remember to always allocate both a user and kernel stack for each thread
    //remember that we allocate 16000 bytes and not not 16kib
    uint64_t* user_stack = (uint64_t*) (((uint64_t) kmalloc_byte_interruptable(THREAD_STACK_SIZE)) + THREAD_STACK_SIZE);
    t->user_rsp = user_stack;//new stack and the rsp is set to the top of the stack
    for (size_t i = 0; i < 5; i++) {
        //maybe i should have a stack in the lower half only but idk
        change_page_map(t->cr3, (((uint64_t) t->user_rsp) - THREAD_STACK_SIZE) + (i*4000), 0b111);
    }

    memset(t->user_rsp - THREAD_STACK_SIZE, 0, THREAD_STACK_SIZE);

    //this is kinda dirty but oh well
    // change_page_map(t->cr3, (uint64_t) t, 0b111);//map the entire thread context struct. not sure if it's needed but just in case. also might be bad for safety but idk
    change_page_map(t->cr3, (uint64_t) &scheduling_started, 0b111);//because scheduling_started is a global variable but it's originally mapped without userspace permissions
    // change_page_map(t->cr3, (uint64_t) running_thread, 0b111);
    // change_page_map(t->cr3, (uint64_t) get_current_thread, 0b111);

    t->status[4] = 1;//userspace = true


    // com_elf_data_t* elf_data = (com_elf_data_t*) kmalloc_byte(sizeof(com_elf_data_t));
    com_elf_data_t elf_data = {

    };
    
    
    //from salernos
    char* const argv[] = {};
    char* const envp[] = {};
    com_sys_elf64_prepare_stack(elf_data, ((uint64_t) t->user_rsp), ((uint64_t) t->user_rsp), argv, envp);


    
    // kprintf_interruptable("\npid: %d\n", t->pid);
    // jump_to_user(t->elf_entry, (void*) (((uint64_t) t->stack_base) - THREAD_STACK_SIZE));
    // irq_restore(&irq);
    if (t->elf_entry != NULL) jump_to_user(t->elf_entry, t->user_rsp);

    // asm volatile ("sti");
}


//credit salernos

#define AT_NULL   0
#define AT_IGNORE 1
#define AT_EXECFD 2
#define AT_PHDR   3
#define AT_PHENT  4
#define AT_PHNUM  5
#define AT_PAGESZ 6
#define AT_BASE   7
#define AT_FLAGS  8
#define AT_ENTRY  9
#define AT_NOTELF 10
#define AT_UID    11
#define AT_EUID   12
#define AT_GID    13
#define AT_EGID

uintptr_t com_sys_elf64_prepare_stack(com_elf_data_t elf_data,
                                      size_t         stack_end_phys,
                                      size_t         stack_end_virt,
                                      char *const    argv[],
                                      char *const    envp[]) {
#define PUSH(x) *(--stackptr) = (x)
    // uintptr_t *stackptr       = (uintptr_t *)ARCH_PHYS_TO_HHDM(stack_end_phys),
    // uintptr_t *stackptr       = (uintptr_t *) (stack_end_phys + hhdm_offset),
        // uintptr_t* stackptr = (uintptr_t*) stack_end_phys,
    uintptr_t* stackptr = (uintptr_t*) stack_end_virt,

              *orig           = stackptr;
    size_t envc               = 0;


    for (; envp[envc]; envc++) {
        size_t len = strlen(envp[envc]) + 1;
        stackptr   = (uintptr_t *)((uintptr_t)stackptr - len);
        memcpy(stackptr, envp[envc], len);
    }

    size_t argc = 0;
    for (; argv[argc]; argc++) {
        size_t len = strlen(argv[argc]) + 1;
        stackptr   = (uintptr_t *)((uintptr_t)stackptr - len);
        memcpy(stackptr, argv[argc], len);
    }

    stackptr = (uintptr_t *)((uintptr_t)stackptr & (~0xF));
    if (((argc + envc + 1) & 1) != 0) {
        stackptr--;
    }

    PUSH(0);
    PUSH(0);

    PUSH(elf_data.entry);
    PUSH(AT_ENTRY);

    PUSH(elf_data.phdr);
    PUSH(AT_PHDR);

    PUSH(elf_data.phent_sz);
    PUSH(AT_PHENT);

    PUSH(elf_data.phent_num);
    PUSH(AT_PHNUM);

    uintptr_t off = 0;

    PUSH(0);
    stackptr -= envc;
    for (size_t i = 0; i < envc; i++) {
        stackptr[i] = stack_end_virt - (off += strlen(envp[i]) + 1);
    }

    PUSH(0);
    stackptr -= argc;
    for (size_t i = 0; i < argc; i++) {
        stackptr[i] = stack_end_virt - (off += strlen(argv[i]) + 1);
    }

    PUSH(argc);


    uintptr_t* temp = stackptr;

    stackptr -= 300;
    for (int i = 0; i < 100; i++) {
        PUSH(0xDEADBEEFCAFEBABE);
    }

    stackptr = temp;



    return stack_end_virt - ((uintptr_t)orig - (uintptr_t)stackptr);
#undef PUSH
}