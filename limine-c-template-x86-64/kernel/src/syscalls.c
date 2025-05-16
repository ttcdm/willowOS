#include <syscalls.h>

#define MSR_LSTAR   0xC0000082
#define MSR_STAR    0xC0000081
#define MSR_SFMASK  0xC0000084
#define MSR_EFER    0xC0000080
#define EFER_SCE    0x1           // Enable SYSCALL/SYSRET



void test_a() {
    kprintf("hi");
}

void init_syscalls() {

    //lstar for rip
    //star for syscall and sysret ss and cs
    uint32_t msr_low;
    uint32_t msr_high;

    msr_low = (uint32_t) (((uint64_t) test_a) & 0xffffffff);
    msr_high = ((uint64_t) test_a) >> 32;

    __asm__ volatile("wrmsr" : : "a"(msr_low), "d"(msr_high), "c"(0x82));

    uint64_t star_value = ((uint64_t)(0x0810) << 32) | ((uint64_t)(0x1B23) << 48);

    msr_low = (uint32_t)(star_value & 0xFFFFFFFF);
    msr_high = (uint32_t)(star_value >> 32);

    __asm__ volatile("wrmsr" : : "a"(msr_low), "d"(msr_high), "c"(0x81));

    uint32_t fmask = 0x200; // disables IF (interrupt flag)
__asm__ volatile("wrmsr" : : "c"(MSR_SFMASK), "a"(fmask), "d"(0));
uint32_t efer_low, efer_high;
__asm__ volatile("rdmsr" : "=a"(efer_low), "=d"(efer_high) : "c"(MSR_EFER));
efer_low |= EFER_SCE;
__asm__ volatile("wrmsr" : : "c"(MSR_EFER), "a"(efer_low), "d"(efer_high));

// asm volatile ("sysretq");

    // jump_to_user();
}