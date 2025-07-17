#include <syscalls.h>
#include <vfs.h>
#include <tmpfs.h>

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


    // asm volatile ("mov %0, %%rax" :: "r" (syscall_num) : "rax");
    // asm volatile ("syscall");

    // int a = syscall_log("hi");
    int a;
    if (a == 0) {
        // syscall_log("bye");
    }

    while (1) {
        syscall_log("hi from test_a");
        // syscall_test();
        // syscall_yield();
        test_b();

        int i = 0;
        i++;
        if (i == 1) i = 0;
        // kprint("hi");
        return;
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

int syscall7(uint64_t num) {//vm map
    return 0;
}

int syscall8(uint64_t num, void *pointer, size_t size) {//vm unmap
    assert(pointer);
    bool irq;
    irq_disable_save(&irq);
    uint64_t* cr3 = (uint64_t*) (get_cr3() + hhdm_offset);
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
    *pointer = (void*) kmalloc_byte(size);
    return 0;
}

int syscall14(uint64_t num, void *pointer) {
    kfree((uint64_t*) pointer);
    return 0;
}

int syscall15(uint64_t num, char* str) {//log; remember to always have the syscall number as the first arg because syscall_handler calls every syscall with all the args
    kprintf("%s\n", str);//no checks against non null terminated strings
    return 0;
}







int syscall_log(char* str) {//15
    int ret;
    size_t num = 15;
    // asm volatile("syscall" : "=a"(ret): "D"(id) : "memory");
    asm volatile ("syscall" : "=a"(ret) : "D"(num), "S"(str) : "memory");
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


int sys_vm_map(void *hint, size_t size, int prot, int flags, int fd, uint64_t offset, void **window) {
    int ret;
    uint64_t num = 7;
}
int sys_vm_unmap(void *pointer, size_t size) {
    int ret;
    uint64_t num = 8;
    asm volatile("syscall" : "=a"(ret): "D"(num), "S"(pointer), "d"(size): "memory");
}