#define IDT_MAX_DESCRIPTORS 256//make sure that you don't exeed 64 or something like that unless you raise this
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
    kprintln_uint64((uint64_t)frame);
    /*
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
    kprintln_uint64(frame->cs);
    */

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

void thread_handler(struct interrupt_frame* frame) {//67. not sure how i'm gonna use interrupt frame yet because the pushing/popping order might be messed up
    volatile uint32_t* lapic_id = (uint32_t*)(ACPI_MADT->lapic_addr + 0x20);
    volatile uint32_t* lapic_eoi = (uint32_t*)(ACPI_MADT->lapic_addr + 0xb0);
    
    
    *lapic_eoi = 0;
}


void thread_interrupter_handler(struct interrupt_frame* frame) {//72?? stack overflow said bits 3 to 7 which is for every 8
    // disable_preemption();
    asm volatile ("cli");
    volatile uint32_t* lapic_id = (uint32_t*)(ACPI_MADT->lapic_addr + 0x20);
    volatile uint32_t* lapic_eoi = (uint32_t*)(ACPI_MADT->lapic_addr + 0xb0);
    *lapic_eoi = 0;
    kprintf_interruptable("\nthread interrupted\n");
    volatile thread_context* current_thread = get_current_thread();
    // current_thread->frame[0] = 1;//signaled for rescheduling

    if (current_thread->status[3] == 0) {
        push_back(ready_queue, current_thread);//&ready_queue
    }
    reschedule();
}

void thread_sleep_handler(struct interrupt_frame* frame) {//80. every 16 is a higher priority
    // asm volatile ("cli");
    // kprintf_interruptable("HIHIHIHI");
    volatile uint32_t* lapic_id = (uint32_t*)(ACPI_MADT->lapic_addr + 0x20);
    volatile uint32_t* lapic_eoi = (uint32_t*)(ACPI_MADT->lapic_addr + 0xb0);
    *lapic_eoi = 0;
    uint64_t tsc_time = tsc_read_ns();//could also be put inside the while loop but idk how to feel about calling the function so many times. i mean i guess there's a precision benefit but ehhh
    volatile thread_context* current_thread = ready_queue_head;
	while (current_thread) {
        break;
		if (tsc_time >= (current_thread->last_run_time + current_thread->sleep_for_ms) * 1000000) {
            // current_thread->last_run_time = tsc_read_ns();
            current_thread->sleep_for_ms = 0;
            current_thread->status[4] = 0;
            kprintf_interruptable("waking thread %d", current_thread->pid);
            unblock_thread(current_thread);
		}
		current_thread = current_thread->next_thread;
        if (current_thread == ready_queue_end) {//again, i'm not sure how the whole linked list works, so ready_queue_end might not even be the last node
            break;
        }
	}
	// if (current_thread == NULL) return;
}

void ps2_keyboard_handler(struct interrupt_frame* frame) {
    volatile uint32_t* lapic_id = (uint32_t*)(ACPI_MADT->lapic_addr + 0x20);
    volatile uint32_t* lapic_eoi = (uint32_t*)(ACPI_MADT->lapic_addr + 0xb0);
    
    // kprintf_interruptable("keyboard interrupt\n");
    uint64_t scancode = inb(0x60);//MUST READ TO CLEAR OUTPUT BUFFER TO ALLOW NEXT OUTPUT (keystroke) for interrupt
    kprintf_interruptable("%c", scancode_to_string(scancode));
    *lapic_eoi = 0;
}


void page_fault_handler(struct interrupt_frame* frame) {//not sure if i'm catching these correctly since they aren't a separate interrupt descriptor thing inside idt.asm. they just kinda rewrite it?? i also don't have a dedicated idt set descriptor line for them so idk
    kprintln("page fault occurred");
    while (1) {asm volatile ("cli; hlt");};
}

void gpf_handler(struct interrupt_frame* frame) {
	kprintln("general protection fault occurred. halting...");
	while (1) asm volatile("cli; hlt");
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
    idtr.limit = (uint32_t)sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1;//may cause an issue with it having -1 but idk
    for (uint64_t vector = 0; vector < 256; vector++) {//HERE MUST USE A LARGER TYPE because it overflows and never actually hits 256
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = true;
    }
    
    //HERE not sure if i have to manually set them for gpf and page faults as well since i did define them in idt.asm as separate things

    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag

}
