#define IDT_MAX_DESCRIPTORS 200//make sure that you don't exeed 64 or something like that unless you raise this
#include <idt.h>

//this is the non chatgpt'ed version of the idt. it may be more error free


__attribute__((aligned(0x10)))
static idt_entry_t idt[256]; // Create an array of IDT entries; aligned for performance



__attribute__((noreturn))
void exception_handler() {
    kprint("oops");
    __asm__ volatile ("cli; hlt"); // Completely hangs the computer
}

//__attribute__((interrupt))
void interrupt_handler_custom(struct interrupt_frame* frame) {
    kprint("hi");
}

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    idt_entry_t* descriptor = &idt[vector];

    descriptor->isr_low = (uint64_t)isr & 0xFFFF;
    descriptor->kernel_cs = 0x08;//GDT_OFFSET_KERNEL_CODE//this value can be whatever offset your kernel code selector is in 
    descriptor->ist = 0;
    descriptor->attributes = flags;
    descriptor->isr_mid = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor->isr_high = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved = 0;
}

static bool vectors[IDT_MAX_DESCRIPTORS];

extern void* isr_stub_table[];

void bp(void) {
}

void idt_init() {
    idtr.base = (uintptr_t)&idt[0];//codeium said to use (uint64_t)&idt[0];
    idtr.limit = (uint32_t)sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1;

    for (uint8_t vector = 0; vector < 64; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = true;
    }

    idt_set_descriptor(64, isr_stub_table[64], 0x8E);
    vectors[64] = true;

    //bp();


    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag

}
