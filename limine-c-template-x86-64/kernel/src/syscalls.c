#include <syscalls.h>

#define MSR_LSTAR   0xC0000082
#define MSR_STAR    0xC0000081
#define MSR_SFMASK  0xC0000084
#define MSR_EFER    0xC0000080
#define EFER_SCE    0x1

#define USER_CS     0x1B  // 0x18 | 3
#define USER_SS     0x23  // 0x20 | 3
#define KERNEL_CS   0x08


uint64_t* top;

void* user_code;

void test_a() {
    // kprintf("hi");
    // while (1) asm volatile ("hlt");
    while (1) {
        int i = 0;
        i++;
        // kprint("hi");
    }
}

typedef struct pml4_page_struct {//not sure if we need __attribute__((packed))
    uint64_t entries[512];
} page_struct;




void init_syscalls() {

    top = (uint64_t*) kmalloc_byte(4096) + 4096;

    uint32_t msr_low, msr_high;

    // 1. Set LSTAR (RIP to jump to on syscall)
    uint64_t lstar = (uint64_t)test_a;
    msr_low = (uint32_t)(lstar & 0xFFFFFFFF);
    msr_high = (uint32_t)(lstar >> 32);
    __asm__ volatile("wrmsr" : : "a"(msr_low), "d"(msr_high), "c"(MSR_LSTAR));

    // uint64_t IA_32_STAR = 0;
    // IA_32_STAR |= ((uint64_t)0x28 << 32);
    // IA_32_STAR |= ((uint64_t)0x33 << 48);
    // wrmsr(0xC0000081, IA_32_STAR);
    // wrmsr(0xC0000082, (uint64_t)jump_to_user);
    // wrmsr(0xC0000084, (1 << 9));
    // uint64_t IA_32_EFER = rdmsr(0xC0000080);
    // IA_32_EFER |= (1);
    // wrmsr(0xC0000080, IA_32_EFER);


    // wrmsr(MSR_STAR, 0x001B000800000000ULL);
    // wrmsr(MSR_STAR, ((uint64_t)KERNEL_CS << 32) | ((uint64_t)USER_CS << 48));
    wrmsr(MSR_STAR, (uint64_t)((uint64_t)0x10 << 48) | (uint64_t)((uint64_t)0x8 << 32));
    // wrmsr(MSR_SFMASK, (1 << 9));  // mask IF
    // wrmsr(MSR_EFER, rdmsr(MSR_EFER) | EFER_SCE);
    wrmsr(MSR_EFER, rdmsr(MSR_EFER) | 1); // Set EFER.SCE = 1


    /*
    • IA32_KERNEL_GS_BASE — Used by SWAPGS instruction.
    • IA32_LSTAR — Used by SYSCALL instruction.
    • IA32_FMASK — Used by SYSCALL instruction.
    • IA32_STAR — Used by SYSCALL and SYSRET instruction.
    */


    // map_page((uint64_t*)pml4_address_virt_glob, top, top, 0b111);
    // map_page((uint64_t*)pml4_address_virt_glob, (uint64_t) test_a, (uint64_t) test_a, 0b111);
    // void* user_code = (void*)0x400000;
    // memcpy(user_code, (void*)test_a, 64); // careful: make sure size fits 
    // map_page((uint64_t*)pml4_address_virt_glob, 0x400000, 0x400000, 0b111);



    // map_page((uint64_t*)pml4_address_virt_glob, test_a, (uint64_t) test_a, 0b111);

    change_page_map((uint64_t) test_a, 0b111);
    change_page_map((uint64_t) top, 0b111);//HERE ALWAYS REMEMBER TO CHANGE THE PAGE MAP FOR EVERYTHING. PLEASE DON'T MAKE THE SAME MISTAKE
    //HERE we're only mapping the current page so it's gonna break if it goes out the current page

    jump_to_user();
}

