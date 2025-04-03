#define IDT_MAX_DESCRIPTORS 200//make sure that you don't exeed 64 or something like that unless you raise this
#include <idt.h>
#include <kutils.h>
#include <apic.h>
#include <tsc.h>
#include <hpet.h>
#include <scheduler.h>

//this is the non chatgpt'ed version of the idt. it may be more error free


__attribute__((aligned(0x10)))
static idt_entry_t idt[256]; // Create an array of IDT entries; aligned for performance

__attribute__((noreturn))
void exception_handler() {
    kprintln("exception occurred");
    __asm__ volatile ("cli; hlt"); // Completely hangs the computer
}

//__attribute__((interrupt))
void interrupt_handler_custom(struct interrupt_frame* frame) {//64
    kprint("interrupt occurred! ");

    kprint("interrupt frame dump and the instruction address: ");
    kprintln_uint64((uint64_t)frame);/*
    kprintln_uint64(frame->r15);
    kprintln_uint64(frame->r14);
    kprintln_uint64(frame->r13);
    kprintln_uint64(frame->r12);
    kprintln_uint64(frame->r11);
    kprintln_uint64(frame->r10);
    kprintln_uint64(frame->r9);
    kprintln_uint64(frame->r8);
    kprintln_uint64(frame->rbp);
    kprintln_uint64(frame->rdi);
    kprintln_uint64(frame->rsi);
    kprintln_uint64(frame->rdx);
    kprintln_uint64(frame->rcx);
    kprintln_uint64(frame->rbx);
    kprintln_uint64(frame->rax);
    kprintln_uint64(frame->rflags);
    kprintln_uint64(frame->rip);
    kprintln_uint64(frame->cs);*/

    //asm volatile ("int $64");
    volatile uint32_t* lapic_eoi = (uint32_t*)(ACPI_MADT->lapic_addr + 0xb0);
    *lapic_eoi = 0;

}

void apic_tick_handler(struct interrupt_frame* frame) {//65
    volatile uint32_t* lapic_lvt_timer = (uint32_t*)(ACPI_MADT->lapic_addr + 0x320);
    volatile uint32_t* lapic_initial_count = (uint32_t*)(ACPI_MADT->lapic_addr + 0x380);
    volatile uint32_t* lapic_current_count = (uint32_t*)(ACPI_MADT->lapic_addr + 0x390);
    volatile uint32_t* lapic_divider = (uint32_t*)(ACPI_MADT->lapic_addr + 0x3e0);
    volatile uint32_t* lapic_eoi = (uint32_t*)(ACPI_MADT->lapic_addr + 0xb0);


    *lapic_eoi = 0;//remember to clear eoi after handling the interrupt
    kprintln("apic tick set up");
}

void sleep_handler(struct interrupt_frame* frame) {//66
    volatile uint32_t* lapic_id = (uint32_t*) (ACPI_MADT->lapic_addr + 0x20);
    volatile uint32_t* lapic_eoi = (uint32_t*)(ACPI_MADT->lapic_addr + 0xb0);
    sleep_locks[(*lapic_id)>>24] = 0;

    *lapic_eoi = 0;//remember to clear eoi after handling the interrupt
}

void thread_handler(struct interrupt_frame* frame) {//67
    volatile uint32_t* lapic_id = (uint32_t*)(ACPI_MADT->lapic_addr + 0x20);
    volatile uint32_t* lapic_eoi = (uint32_t*)(ACPI_MADT->lapic_addr + 0xb0);

    //asm volatile 
    //kpass(500);//it seems to stall when i kpass() here. it's probably because the sleep handler's the same priority and this one hasn't cleared out yet, but even if i clear the eoi and do it it somehow doesn't work idk

    kprintln("hi");
    //read tsc; if tsc > last start time + quantum, stop thread and return
    *lapic_eoi = 0;
}


void page_fault_handler(struct interrupt_frame* frame) {//not sure if i'm catching these correctly since they aren't a separate interrupt descriptor thing inside idt.asm. they just kinda rewrite it?? i also don't have a dedicated idt set descriptor line for them so idk
    kprintln("page fault occurred");
}

void gpf_handler(struct interrupt_frame* frame) {
	kprintln("general protection fault occurred. halting...");
	asm volatile("cli; hlt");
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
    idtr.base = (uintptr_t)&idt[0];//codeium said to use (uint64_t)&idt[0];
    idtr.limit = (uint32_t)sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1;

    for (uint8_t vector = 0; vector < 68; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = true;
    }
    //HERE not sure if i have to manuall set them for gpf and page faults as well since i did define them in idt.asm as separate things

    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag

}
