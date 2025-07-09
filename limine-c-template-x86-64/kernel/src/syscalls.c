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
*/

int syscall0() {//open
    return 0;
}

int syscall1() {//close
    kprintf("syscall1\n");
    while (1);
}

int syscall2() {//read
    return 0;
}

int syscall3() {//write
    return 0;
}

int syscall4(size_t num, char* str) {//log; remember to always have the syscall number as the first arg because syscall_handler calls every syscall with all the args
    kprintf("%s\n", str);//no checks against non null terminated strings
    return 0;
}

int syscall5(uint64_t num) {
    // asm volatile ("cli");
    // change_tss(tss, get_current_thread()->stack_base);
    // asm volatile ("sti");
    return 0;
}

int syscall6(uint64_t num) {
    yield_thread();
    return 0;
}

int syscall7() {
    return 0;
}

int syscall8() {
    return 0;
}

__attribute__((noreturn))
int syscall9(uint64_t num) {
    scheduler_return();
    // return 0;
}

int syscall_log(char* str) {//4
    int ret;
    size_t num = 4;
    // asm volatile("syscall" : "=a"(ret): "D"(id) : "memory");
    asm volatile ("syscall" : "=a"(ret) : "D"(num), "S"(str) : "memory");
    return ret;
}

int syscall_test() {//5
    // return 0;
    int ret;
    uint64_t num = 5;
    asm volatile("syscall" : "=a"(ret): "D"(num) : "memory");
    return ret;
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