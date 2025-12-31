#include <syscalls.h>
#include <vfs.h>
#include <tmpfs.h>

#include <nanoprintf-0.5.4/nanoprintf.h>

#include <mman.h>
#include <errno.h>

#define MSR_LSTAR   0xC0000082
#define MSR_STAR    0xC0000081
#define MSR_SFMASK  0xC0000084
#define MSR_EFER    0xC0000080
#define EFER_SCE    0x1

#define USER_CS     0x1B  // 0x18 | 3
#define USER_SS     0x23  // 0x20 | 3
#define KERNEL_CS   0x08



void test_b() {
    int i = 0;
    int a = 2;
    int b = 1;
    int c = 3;
    c = a + b + i;
    if (c == 3) {
        b = 5;
    }
    return;

}

void test_a() {
    // syscall_test();
    uint64_t syscall_num = 1;

    // return;


    // asm volatile ("mov %0, %%rax" :: "r" (syscall_num) : "rax");
    // asm volatile ("syscall");

    // int a = syscall_log("hi");
    int a;
    if (a == 0) {
        // syscall_log("bye");
    }

    for (uint64_t i = 0; i < 32; i++) {
		// map_page(get_current_thread()->cr3, 0x10000, (uint64_t) i, 0b111);
		uint64_t* a;

        //it breaks in the memset if i use non 4kib aligned addresses but i think that we're only supposed to pass in 4kib aligned stuff. we should probably do a check inside the function(s) for it tho
		sys_vm_map((uint64_t*) ((i*0x1000)+0x1000000), 0x1000, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_EXECUTABLE, 0, 0, (void**) &a);
        // break;
    }
    // thread_context* t;
    // get_current_thread_syscall(&t);
    // t = get_current_thread();
    // return;
    // t = running_thread;
    //HERE modify syscall log for fmt and verify that all the mappings are correct
    // syscall_log("%llx", t);
	// for (uint64_t i = 0; i < t->mappings.max_mappings; i++) {//we need max because num mappings can go under an allocated index
	// 	syscall_log("%d %llx virt address ABC\n", i, t->mappings.mapped_virt_addresses_array[i].virt_address);
	// }

    int aa = 1;
    int b = 2;
    // syscall_log("abc %d %d", aa, b);

    while (1) {
        syscall_log("hi from test_a");
        // syscall_test();
        // syscall_yield();
        test_b();

        int i = 0;
        i++;
        if (i == 1) i = 0;
        // kprint("hi");
        // return;
    }

    
    
}

void swap_to_user_or_kernel_gs(uint64_t cs, bool to_or_from_kernel) {//1 for to kernel and 0 for from kernel
    return;
    // uint16_t cs;
    // __asm__ volatile ("mov %%cs, %0" : "=r"(cs));
    uint64_t cpl = cs & 0x3;

    if (cpl == 3) {
        if (to_or_from_kernel) swap_to_kernel_gs();
        else swap_to_user_gs();
    }
}


void init_syscalls() {

    uint32_t msr_low, msr_high;

    // 1. Set LSTAR (RIP to jump to on syscall)
    uint64_t lstar = (uint64_t) syscall_handler;
    msr_low = (uint32_t)(lstar & 0xFFFFFFFF);
    msr_high = (uint32_t)(lstar >> 32);
    __asm__ volatile("wrmsr" : : "a"(msr_low), "d"(msr_high), "c"(MSR_LSTAR));

    wrmsr(MSR_STAR, (uint64_t)((uint64_t)0x10 << 48) | (uint64_t)((uint64_t)0x8 << 32));
    wrmsr(MSR_SFMASK, 0x202);//HERE to basically cli as syscall happens instead of having to manually cli
    wrmsr(MSR_EFER, rdmsr(MSR_EFER) | 1); // Set EFER.SCE = 1

    uint64_t gs_base = (uint64_t) kmalloc_byte(4096);//not sure how many bytes it actually needs and whether it goes up or down
    uint64_t kernel_gs_base = (uint64_t) kmalloc_byte(4096);//HERE check if it's ok for these to be so far apart and not 8 bytes apart
    change_page_map((uint64_t*) pml4_address_virt_glob, gs_base, 0b111);
    change_page_map((uint64_t*) pml4_address_virt_glob, kernel_gs_base, 0b111);
    wrmsr(0xC0000101, gs_base);
    wrmsr(0xC0000102, kernel_gs_base);

    change_page_map((uint64_t*) pml4_address_virt_glob, (uint64_t) test_a, 0b111);//make sure to map the entire function. this only maps a page and we're assuming that the function is smaller than that
    change_page_map((uint64_t*) pml4_address_virt_glob, (uint64_t) test_a+0x1000, 0b111);
    change_page_map((uint64_t*) pml4_address_virt_glob, (uint64_t) test_a+0x2000, 0b111);
    change_page_map((uint64_t*) pml4_address_virt_glob, (uint64_t) test_a+0x3000, 0b111);

    change_page_map((uint64_t*) pml4_address_virt_glob, (uint64_t) syscall_test, 0b111);
    change_page_map((uint64_t*) pml4_address_virt_glob, (uint64_t) syscall_test+0x1000, 0b111);
    change_page_map((uint64_t*) pml4_address_virt_glob, (uint64_t) syscall_test+0x2000, 0b111);

    change_page_map((uint64_t*) pml4_address_virt_glob, (uint64_t) test_b, 0b111);



    // change_page_map((uint64_t) scheduler_return, 0b111);
    // change_page_map((uint64_t) scheduler_return+0x1000, 0b111);
    // change_page_map((uint64_t) scheduler_return+0x2000, 0b111);
    // change_page_map((uint64_t) scheduler_return+0x3000, 0b111);


}

/*help from jw
int syscall0(int id) {int ret; asm volatile("syscall" : "=a"(ret): "D"(id) : "memory"); return ret;}
int syscall1(int id, int arg1) {int ret; asm volatile("syscall" : "=a"(ret): "D"(id), "S"(arg1) : "memory"); return ret;} ...

asm volatile("syscall" : "=a"(ret): "D"(num), "S"(pointer), "d"(a), "c"(b), "r8"(c), "r9"(d) : "memory");//i think this is right
*/



int syscall0(uint64_t num) {//open
    return 0;
}

int syscall1(uint64_t num) {//close
    kprintf("syscall1\n");
    while (1);
}

int syscall2(uint64_t num) {//read
    return 0;
}

int syscall3(uint64_t num) {//write
    return 0;
}

int syscall4(size_t num) {//seek
    return 0;
}

int syscall5(uint64_t num) {
    return 0;
}

int syscall6(uint64_t num) {
    yield_thread();
    return 0;
}

int syscall7(uint64_t num, struct map_page_bytes_args* mmap_args) {//vm map
    //remember to zero the entire allocation
    bool irq;
    irq_disable_save(&irq);
    //we can only assert here because cli or smth isn't allowed in userspace or smth idk it just doesn't seem to work in userspace
    if ((mmap_args->virt_address & 0xfff != 0)) {
        return EINVAL;
    }
    //hopefully there's no race condition here with getting the current thread and other stuff
    mmap_args->cr3 = get_current_thread()->cr3;
    // kprintf("syscall7 %llx %llx %llx\n", mmap_args->virt_address, mmap_args->size, mmap_args->flag);
    mmap_args->error = map_range(mmap_args->cr3, mmap_args->virt_address, mmap_args->permissions, mmap_args->size, mmap_args->flag);//HERE maybe not hint?
    memset((uint64_t*) mmap_args->virt_address, 0, mmap_args->size);//this only zeros the size and not the entire page(s) but it probably shouldn't matter??
    if (((uint64_t*) mmap_args->virt_address)[256] != 0) {
        kprintf("syscall7: memset clear failed\n");
        assert(false);
    }
    
    // if (mmap_args->virt_address == 0x101f000) {//because 0d31 is 0xf1
    //     thread_context* t;
    //     t = get_current_thread();
    //     for (uint64_t i = 0; i < t->mappings.max_mappings; i++) {//we need max because num mappings can go under an allocated index
    //         kprintf("%d %llx virt address ABC\n", i, t->mappings.mapped_virt_addresses_array[i].virt_address);
    //     }
    // }

    irq_restore(&irq);

    return mmap_args->error;
}

int syscall8(uint64_t num, void *pointer, size_t size) {//vm unmap
    assert(pointer);
    bool irq;
    irq_disable_save(&irq);
    // uint64_t* cr3 = (uint64_t*) (get_cr3() + hhdm_offset);//not sure if we sould use get_current_thread() instead of just getting the raw cr3
    uint64_t* cr3 = get_current_thread()->cr3;
    irq_restore(&irq);
    assert(cr3);
    uint64_t num_pages;
    //convert size to number of pages. i should probably put this into a function since it's a somewhat common operation
    if (size == 0) {
        num_pages = 0;
    }
    if (size <= 0x1000) {
        num_pages = 1;
    }
    else {
        if (size % 0x1000 == 0) {
            num_pages = size / 0x1000;
        }
        else {
            num_pages = (size / 0x1000) + 1;
        }
    }

    for (uint64_t i = 0; i < num_pages; i++) {
        unmap_page(cr3, ((uint64_t) pointer) + (i * 0x1000));
    }
    
    return 0;
}

__attribute__((noreturn))
int syscall9(uint64_t num) {//userspace to kernelspace return for threads
    scheduler_return();
    // return 0;
}

int syscall10(uint64_t num) {
    return 0;
}

int syscall11(uint64_t num, int* pointer, int expected) {//futex wait
    // mutex_t m = {.locked = 0, .object = pointer};
    //create a queue for the mutex? also mutex may have to be on any given value rather than an int
    
    bool irq;
    irq_disable_save(&irq);
    thread_context* thread = get_current_thread();

    if (!__sync_bool_compare_and_swap(&thread->status[3], *pointer, expected)) {
        kprintf_interruptable("futex wait: EEAGAIN");
        irq_restore(&irq);
        return 11;//EAGAIN??
    }
    else {
        pointer = (int*) virt_to_phys((uint64_t) pointer, (uint64_t) thread->cr3);//HERE remember to not cast incorrectly and accidentally truncate something
        futex_enqueue(pointer, thread);
    }
    irq_restore(&irq);
    return 0;
}

int syscall12(uint64_t num, int* pointer) {//futex wake
    // unblock_thread(get_current_thread());//try to change unblock thread to use pids instead maybe or the futex queue to use the thread contexts instead
    bool irq;
    irq_disable_save(&irq);

    for (int i = 0; i < sizeof(global_futex_array) / sizeof(futex_queue_t*); i++) {
        if (((futex_queue_t*) global_futex_array[i])->pointer == pointer) {
            futex_queue_t* queue = global_futex_array[i];
            for (int j = 0; j < queue->size; j++) {//i hope there isn't an off by one error here
                queue->threads[i]->status[3] = 0;
            }
            kfree_interruptable((uint64_t*) queue->threads);
            kfree_interruptable((uint64_t*) queue);
            global_futex_array[i] == NULL;
        }
    }

    irq_restore(&irq);
    return 0;
}

int syscall13(uint64_t num, size_t size, void **pointer) {
    // kprintf("%llx\n", size);
    *pointer = (void*) kmalloc_byte(size);
    change_page_map_range(get_current_thread()->cr3, (uint64_t)*pointer, (uint64_t) size, 0b111);
    return 0;
}

int syscall14(uint64_t num, void *pointer) {
    kfree((uint64_t*) pointer);
    return 0;
}

int syscall15(uint64_t num, struct fmt_args* args_struct) {//log; remember to always have the syscall number as the first arg because syscall_handler calls every syscall with all the args
    bool irq_status;
    irq_disable_save(&irq_status);
    // va_list args;
    // va_start(args, fmt);
    va_list args_copy;
    va_copy(args_copy, args_struct->args);
    uint64_t size = npf_vsnprintf(NULL, 0, args_struct->fmt, args_struct->args);
    // char* str = (char*) kmalloc_byte(size);//+1 byte for null terminating char. i don't think i actually need this because i'm using actual sizes instead of relying on the terminating char
    char str[size];//was told that using a variable length array was a bad idea...
    npf_vsnprintf(str, size+1, args_struct->fmt, args_copy);//+1 byte for null terminating char. we need this because it assumes that the last thing is a null terminating char or something

    acquire_mutex(&ft_ctx_mutex);
    flanterm_write(ft_ctx, str, size);
    release_mutex(&ft_ctx_mutex);

    // kfree((uint64_t*) str);
    va_end(args_struct->args);
    va_end(args_copy);
    irq_restore(&irq_status);
    
    return 0;
}

int syscall16(uint64_t num, thread_context** thread) {//get_current_thread_syscall
    *thread = get_current_thread();
    return 0;
}

int syscall17(uint64_t num, char* str, size_t len) {
    bool irq_status;
    irq_disable_save(&irq_status);

    acquire_mutex(&ft_ctx_mutex);
    flanterm_write(ft_ctx, str, len);
    release_mutex(&ft_ctx_mutex);

    irq_restore(&irq_status);
    return 0;
}

int syscall18(uint64_t num, void* pointer) {
    //mmio fs address
    wrmsr(0xC0000100, (uint64_t) pointer);
    return 0;
}


//HERE the code under this line should be 1:1 with generic.cpp for mlibc sysdeps aside from the possible type mismatches, i.e., mode_t vs int, off_t vs int64_t, and so on


int syscall_log(char* fmt, ...) {//15
    int ret;
    size_t num = 15;

    //not sure if we need to cli here
    va_list args;
    va_start(args, fmt);

    struct fmt_args args_struct = {
        .fmt = fmt,
    };

    va_copy(args_struct.args, args);

    asm volatile ("syscall" : "=a"(ret) : "D"(num), "S"(&args_struct) : "memory");
    return ret;
}


int syscall_test() {
}


int syscall_yield() {//6
    int ret;
    uint64_t num = 6;
    asm volatile("syscall" : "=a"(ret): "D"(num) : "memory");
    return ret;
}

int syscall_user_thread_exit() {//9
    int ret;
    uint64_t num = 9;
    asm volatile("syscall" : "=a"(ret): "D"(num) : "memory");
    return ret;
}



int sys_futex_wait(int *pointer, int expected, const struct timespec *time) {//11
    //we ignore time arg for now
    int ret;
    uint64_t num = 11;
    asm volatile("syscall" : "=a"(ret): "D"(num), "S"(pointer), "d"(expected): "memory");
    return ret;
}
int sys_futex_wake(int *pointer) {//12
    int ret;
    uint64_t num = 12;
    asm volatile ("syscall" : "=a"(ret) : "D"(num), "S"(pointer) : "memory");
    return ret;
}



int sys_anon_allocate(size_t size, void **pointer) {//13
    int ret;
    uint64_t num = 13;

    sys_libc_log("sys_anon_allocate called\n");

    asm volatile("syscall" : "=a"(ret): "D"(num), "S"(size), "d"(pointer): "memory");
    return ret;
}
int sys_anon_free(void *pointer, size_t size) {//14
    //i think we can disregard size for now
    int ret;
    uint64_t num = 14;
    asm volatile ("syscall" : "=a"(ret) : "D"(num), "S"(pointer) : "memory");
    return ret;
}


int sys_vm_map(void *hint, size_t size, int prot, int flags, int fd, int64_t offset, void **window) {//7
    int ret;
    uint64_t num = 7;

    // syscall_log("\n\nsyscall_log and sys_vm_map called\n\n");
    // sys_libc_log("HERE sys_vm_map called\n");
    sys_libc_log("sys_vm_map called\n");

    //we don't have permissions for file access yet so we're just gonna ignore prot

    // int64_t w_flags = (int64_t) flags;//wide flags

    //masking and extracting only a single bit https://stackoverflow.com/questions/2249731/how-do-i-get-bit-by-bit-data-from-an-integer-value-in-c

    //https://pubs.opengroup.org/onlinepubs/9799919799/
    //mmap

    //https://chuck.cranor.org/p/diss.pdf


    //HERE technically this can really only be called when scheduling is started because scheduling is more or less always enabled and we don't call this outright in userspace by just jumping to something without creating a thread
    //make it so that scheduling must be started for this to function
    // if (!scheduling_started) {
    //     // kprintf("error: scheduling not started\n");
    //     return EPERM;
    // }



    uint64_t perm = 0;
    //not sure about PROT_NONE
    //always remember to change every value
    //maybe use a mask instead of shifts
    if (((prot & ( 1 << 0 )) >> 0) == 1) {//PROT_READ
        perm &= 0 << 1;
    }
    else {
        perm |= ~(1 << 0);//PROT_NONE. should be correct
        //move down so we don't need a goto??
    }
    if (((prot & ( 1 << 1 )) >> 1) == 1) {//PROT_WRITE
        perm |= 1 << 1;
    }
    if (((prot & ( 1 << 2 )) >> 2) == 0) {//PROT_EXEC
        perm |= 1ULL << 63;//no execute
    }

    perm |= 0b01;//mark the page as present

    //we have to use 1ULL because 1 defaults to int which is 16 or 32 bits wide i don't remember 
    //HERE remember to cast 1 to a ull and maybe have a constant(s?).h to have macros for it as well


    // if (((flags & ( 1 << 0 )) >> 0) == MAP_SHARED) {
    //     // map_page((uint64_t*) get_current_thread()->cr3, hint, hint, perm);
    // }
    // else if (((flags & ( 1 << 0 )) >> 0) == MAP_PRIVATE) {

    // }

    int map_page_ret;

    struct map_page_bytes_args mmap_args = {//make sure that this persists throughout the actual syscaLL
            // .cr3 = (uint64_t*) get_current_thread()->cr3,//this is set inside the syscall function
            // .phys_address = (uint64_t) region_start,//this is also set inside the syscall function
            .virt_address = (uint64_t) hint,
            .permissions = perm,
            .size = size,
            .error = 0,
            .ret = NULL
    };

    // //the issue is because we're extracting instead of masking
    // kprintf("\n%llx\n", flags);
    // kprintf("\n%llx ", (flags & (0xF << (4 * 1))));//chatgpt'd more or less
    // while (1);

    // kprintf("%llx\n", MAP_ANON);

    // kprintf("%b\n", (flags & (0xF << (4 * 1))) == (int) MAP_ANON);
    
    if ((int) (flags & (0xF << (4 * 1))) == (int) MAP_ANON) {
        ////HERE figure out if we're gonna use hint regardless or our own thing
        //HERE remember to unmap if it's already mapped?? smth about overlaps

        //we wall map_page_bytes behind a syscall here
        //we can just shove all the args into a struct because we don't have enough syscall args for all of the args here

        if ((int) (flags & (0xF << (4 * 0))) == (int) MAP_SHARED) {
            //remember to change hint as well if necessary
            mmap_args.flag = MAP_SHARED;
        }
        else if ((int) (flags & (0xF << (4 * 0))) == (int) MAP_PRIVATE) {
            mmap_args.flag = MAP_PRIVATE;
        }
        //it should be fine passing the struct that's on the stack since the branch hasn't exited yet
        asm volatile("syscall" : "=a"(map_page_ret): "D"(num), "S"(&mmap_args): "memory");
        // assert(mmap_args.error == 0);
        //because we can't have printf here i guess
        if (mmap_args.error != 0) {
            return mmap_args.error;
        }
        // memset(hint, 0, size);

        *window = hint;
        ret = 0;
    }
    else if ((int) (flags & (0xF << (4 * 1))) == (int) MAP_FIXED) {
        
        //HERE remember to unmap if it's already mapped?? smth about overlaps
        if ((int) (flags & (0xF << (4 * 0))) == (int) MAP_SHARED) {
            mmap_args.flag = MAP_SHARED;
        }
        else if ((int) (flags & (0xF << (4 * 0))) == (int) MAP_PRIVATE) {
            mmap_args.flag = MAP_PRIVATE;
        }
        asm volatile("syscall" : "=a"(map_page_ret): "D"(num), "S"(&mmap_args): "memory");
        // assert(mmap_args.error == 0);
        if (mmap_args.error != 0) {
            return mmap_args.error;
        }
        // memset(hint, 0, size);
        *window = hint;
        ret = 0;
    }

    // always use explicit if's to make sure that we're getting the exact value and not using else's cuz we're lazy
    
    // switch ((flags & ( 1 << 0 )) >> 0) {//extracting the 0th bit
    //     case MAP_SHARED:

    //         break;
    //     case MAP_PRIVATE:

    //         break;
    //     case MAP_FIXED:

    //         break;
    //     case MAP_ANON:

    //         break;
    //     // case MAP_ANONYMOUS://MAP_ANONYMOUS is the same as MAP_ANON

    //     //     break;
    // }

    return ret;
}
int sys_vm_unmap(void *pointer, size_t size) {//8
    int ret;
    uint64_t num = 8;
    asm volatile("syscall" : "=a"(ret): "D"(num), "S"(pointer), "d"(size): "memory");
}

//HERE remember to fill these in
//also we define time_t as int64_t inside willowOS but here idk if we should
int sys_clock_get(int clock, time_t *secs, long *nanos) {
    int ret;
    //HERE add num as well
}

int sys_tcb_set(void *pointer) {//18
    int ret;
    uint64_t num = 18;
    syscall_log("sys_tcb_set called\n");
    asm volatile("syscall" : "=a"(ret): "D"(num), "S"(pointer): "memory");
    return ret;
}


//HERE hopefully replacing these types won't cause any misalignments and/or other errors with mlibc
//replaced mode_t with int
int sys_open(const char *pathname, int flags, int mode, int *fd) {
    return 0;
}

//replaced ssize_t with size_t
int sys_read(int fd, void *buf, size_t count, size_t *bytes_read) {
    return 0;
}

int sys_write(int fd, const void *buf, size_t count, size_t *bytes_written) {
    syscall_log((char*) buf);
    return 0;
}

//replaced off_t with int64_t
int sys_seek(int fd, int64_t offset, int whence, int64_t *new_offset) {
    return 0;
}

int sys_close(int fd) {
    return 0;
}

void sys_libc_log(const char *message) {
    syscall_log((char*) message);
}

[[noreturn]] void sys_libc_panic() {
    while (1);
}

//HERE not sure if this is the correct implementation
[[noreturn]] void sys_exit(int status) {
    syscall_user_thread_exit();
    while (1);
}


int get_current_thread_syscall(thread_context** thread) {//16
    int ret;
    uint64_t num = 16;
    asm volatile("syscall" : "=a"(ret): "D"(num), "S"(thread): "memory");
    return ret;
}

int sys_isatty(int fd) {
    // while (1);
    return 0;
}