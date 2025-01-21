#define IDT_MAX_DESCRIPTORS 64//make sure that you don't exeed 64 or something like that unless you raise this
#include <idt.h>

//this is the non chatgpt'ed version of the idt. it may be more error free


__attribute__((aligned(0x10)))
static idt_entry_t idt[256]; // Create an array of IDT entries; aligned for performance

static idtr_t idtr;

__attribute__((noreturn))
void exception_handler() {
    __asm__ volatile ("cli; hlt"); // Completely hangs the computer
}

//__attribute__((noreturn))
//void exception_handler_custom(struct flanterm_context* ft_ctx) {
//    flanter_write(ft_ctx, "helloworld", 10);
//    //__asm__ volatile ("cli; hlt"); // Completely hangs the computer
//    
//}

__attribute__((interrupt))
void interrupt_handler_custom(struct interrupt_frame* frame) {
    __asm__ volatile ("cli; hlt");
    //struct limine_framebuffer* framebuffer = framebuffer_request.response->framebuffers[0];
    //kprint(framebuffer);
}

void kprint(struct limine_framebuffer* framebuffer) {
    for (size_t i = 0; i < 100; i++) { volatile uint32_t* fb_ptr = framebuffer->address; fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xffffff; }
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

void idt_init() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1;

    for (uint8_t vector = 0; vector < 32; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = true;
    }

    idt_set_descriptor(32, isr_stub_table[32], 0x8E);
    vectors[32] = true;

    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag
}