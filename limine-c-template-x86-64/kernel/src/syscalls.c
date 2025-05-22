#include <syscalls.h>

#define MSR_LSTAR   0xC0000082
#define MSR_STAR    0xC0000081
#define MSR_SFMASK  0xC0000084
#define MSR_EFER    0xC0000080
#define EFER_SCE    0x1

#define USER_CS     0x1B  // 0x18 | 3
#define USER_SS     0x23  // 0x20 | 3
#define KERNEL_CS   0x08


uint64_t top;

void* user_code;

void test_a() {
    // kprintf("hi");
    // while (1) asm volatile ("hlt");
    while (1) {
        int i = 0;
        i++;
    }
}

typedef struct pml4_page_struct {//not sure if we need __attribute__((packed))
    uint64_t entries[512];
} page_struct;


void init_syscalls() {

    top = (uint64_t) kmalloc_byte(4096) + 4096;

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

    uint64_t* pml4_address = pml4_address_virt_glob;
    uint64_t virt_address = 0xffffffff8000cdba;
    uint64_t permissions = 0b111;


    uint64_t pml4_index = (virt_address >> 39) & 0x1FF;
    uint64_t pdpt_index = (virt_address >> 30) & 0x1FF;
    uint64_t pd_index = (virt_address >> 21) & 0x1FF;
    uint64_t pt_index = (virt_address >> 12) & 0x1FF;
    uint64_t offset = virt_address & 0xFFF;

    page_struct* pml4 = (void*)pml4_address;
    uint64_t pml4_entry = pml4->entries[pml4_index];
    // if (!(pml4_entry & 1)) {
    //     uint64_t new_entry = (alloc_frame()) | permissions;
    //     pml4->entries[pml4_index] = new_entry;
    //     pml4_entry = new_entry;
    // }

    page_struct* pdpt = (page_struct*)((pml4_entry & ~0xfff) + hhdm_offset);
    uint64_t pdpt_entry = pdpt->entries[pdpt_index];
    // if (!(pdpt_entry & 1)) {
    //     uint64_t new_entry = (alloc_frame()) | permissions;
    //     pdpt->entries[pdpt_index] = new_entry;
    //     pdpt_entry = new_entry;
    // }

    page_struct* pd = (page_struct*)((pdpt_entry & ~0xfff) + hhdm_offset);
    uint64_t pd_entry = pd->entries[pd_index];
    // if (!(pd_entry & 1)) {
    //     uint64_t new_entry = (alloc_frame()) | permissions;
    //     pd->entries[pd_index] = new_entry;
    //     pd_entry = new_entry;
    // }

    page_struct* pt = (page_struct*)((pd_entry & ~0xfff) + hhdm_offset);
    // pt->entries[pt_index] = phys_address | permissions;
    // asm volatile ("invlpg (%0)" :: "r" (virt_address) : "memory");

    kprintf("pml4_entry: %b\n", pml4_entry);
    kprintf("pdpt_entry: %b\n", pdpt_entry);
    kprintf("pd_entry: %b\n", pd_entry);
    kprintf("pt_entry: %b\n", *(&pt->entries[pt_index]));
    kprintf("pml4_index = %d, pdpt_index = %d, pd_index = %d, pt_index = %d\n", pml4_index, pdpt_index, pd_index, pt_index);
    kprintf("pt virt addr: %llx\n", (uint64_t)pt);

    // map_page((uint64_t*)pml4_address_virt_glob, test_a, (uint64_t) test_a, 0b111);


    jump_to_user();
}