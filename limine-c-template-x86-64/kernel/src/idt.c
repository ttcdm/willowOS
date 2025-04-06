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
    
    asm volatile ("sti");//this seems kinda wrong to do but it works


    lapic_periodic(300, 72, 0b0011, 0);



    asm volatile (
		"push %rax\n"
		"push %rbx\n"
		"push %rcx\n"
		"push %rdx\n"
		"push %rsi\n"
		"push %rdi\n"
		"push %rbp\n"
		"push %r8\n"
		"push %r9\n"
		"push %r10\n"
		"push %r11\n"
		"push %r12\n"
		"push %r13\n"
		"push %r14\n"
		"push %r15\n"
		);

    uint64_t return_rsp;
	asm volatile ("mov %%rsp, %0 " : "=r"(return_rsp) :);//save current rsp
    uint64_t misaligned_by = 16 - (return_rsp % 16);//calculate misalignment
    current_thread->return_rsp = (uint64_t*) return_rsp;//save current rsp. i'm not actually sure if i need to do this because i can just store it as a normal variable since this is also a variable but just global (which shouldn't make a difference)
    current_thread->misaligned_by = misaligned_by;//save misalignment

    
    
    
    if (current_thread->total_run_time == 0) {
        current_thread->total_run_time += 1;//need to put this here because it's unreachable if the thread doesn't finish in time
        uint64_t thread_rsp = ((uint64_t) current_thread->stack_base) + THREAD_STACK_SIZE;//we land on the 15999th index (0 indexed)
        
        asm volatile ("mov %0, %%rsp" : : "r"(thread_rsp-(15*sizeof(uint64_t))));//HERE we subtract by the number of elements we popped because we're still at the default stack pointer and not the modified one after pushing everything during thread initialization
        
        asm volatile (//i think this actually works because the rsp gets restored after the function call and it'll still be at 15*8 under the top of the stack
            "pop %r15\n"
            "pop %r14\n"
            "pop %r13\n"
            "pop %r12\n"
            "pop %r11\n"
            "pop %r10\n"
            "pop %r9\n"
            "pop %r8\n"
            "pop %rbp\n"
            "pop %rdi\n"
            "pop %rsi\n"
            "pop %rdx\n"
            "pop %rcx\n"
            "pop %rbx\n"
            "pop %rax\n"
        );
        
        current_thread->thread_entry();
        // asm volatile ("mov %%rsp, %0 " : "=r"((uint64_t)current_thread->current_rsp) :);
    }
    else {

        kprint("rsp: ");
        kprintln_uint64(current_thread->current_rsp);
        current_thread->current_misaligned_by = 16-(((uint64_t)current_thread->current_rsp) % 16);
        // asm volatile ("mov %0, %%rsp" : : "r"((uint64_t)current_thread->current_rsp - current_thread->current_misaligned_by));//load back in aligned rsp
        // asm volatile ("add %0, %%rsp" : : "r"((uint64_t)current_thread->current_misaligned_by));//move rsp back to its original position

        // switch_thread((uint64_t**)current_thread->return_rsp, (uint64_t*)current_thread->current_rsp);
        

        

        // kprintln("hi");

    }

    asm volatile (
		"push %rax\n"
		"push %rbx\n"
		"push %rcx\n"
		"push %rdx\n"
		"push %rsi\n"
		"push %rdi\n"
		"push %rbp\n"
		"push %r8\n"
		"push %r9\n"
		"push %r10\n"
		"push %r11\n"
		"push %r12\n"
		"push %r13\n"
		"push %r14\n"
		"push %r15\n"
	);

    asm volatile ("mov %0, %%rsp" : : "r"((uint64_t)current_thread->return_rsp-current_thread->misaligned_by));//load back in aligned rsp
    asm volatile ("add %0, %%rsp" : : "r"((uint64_t)current_thread->misaligned_by));//move rsp back to its original position

    asm volatile (
		"pop %r15\n"
		"pop %r14\n"
		"pop %r13\n"
		"pop %r12\n"
		"pop %r11\n"
		"pop %r10\n"
		"pop %r9\n"
		"pop %r8\n"
		"pop %rbp\n"
		"pop %rdi\n"
		"pop %rsi\n"
		"pop %rdx\n"
		"pop %rcx\n"
		"pop %rbx\n"
		"pop %rax\n"
	);
    // kprintln("bye");

    
    
    //kpass(500);//it seems to stall when i kpass() here. it's probably because the sleep handler's the same priority and this one hasn't cleared out yet, but even if i clear the eoi and do it it somehow doesn't work idk

    // kprintln("hi");
    //read tsc; if tsc > last start time + quantum, stop thread and return
    *lapic_eoi = 0;
}

void thread_interrupter_handler(struct interrupt_frame* frame) {//72?? stack overflow said bits 3 to 7 which is for every 8
    asm volatile ("mov %%rsp, %0 " : "=r"(current_thread->current_rsp) :);
    volatile uint32_t* lapic_id = (uint32_t*)(ACPI_MADT->lapic_addr + 0x20);
    volatile uint32_t* lapic_eoi = (uint32_t*)(ACPI_MADT->lapic_addr + 0xb0);
    //kprintln("hi");
    current_thread = current_thread->next_thread;
    //*lapic_eoi = 0;
    switch_thread((uint64_t)current_thread->current_rsp);
    kprint("\n");
    kprintln_uint64(current_thread->current_rsp);
    //push_all_regs();
    //scheduler_return();
    *lapic_eoi = 0;

    // asm volatile ("ret");
    //kprint("rsp: ");
    //kprintln_uint64((uint64_t)current_thread->current_rsp);
    //// current_thread->rip = (uint64_t*) frame->rip;
    //// kprintln_uint64((uint64_t)current_thread->current_rsp);
    //current_thread->frame[0] = frame->rflags;
    //current_thread->frame[1] = frame->cs;
    //current_thread->frame[2] = frame->rip;
    // kprintln_uint64(frame->rflags);
    // kprintln_uint64(frame->cs);
    // kprintln_uint64(frame->rip);

    //kprintln("\n");
    //kprintln_uint64(frame->rip);
    //kprintln_uint64(frame->cs);
    //kprintln_uint64(frame->rflags);
    //kprintln_uint64(frame->r15);
    //kprintln_uint64(frame->r14);
    //kprintln_uint64(frame->r13);
    //kprintln_uint64(frame->r12);
    //kprintln_uint64(frame->r11);
    //kprintln_uint64(frame->r10);
    //kprintln_uint64(frame->r9);
    //kprintln_uint64(frame->r8);
    //kprintln_uint64(frame->rbp);
    //kprintln_uint64(frame->rdi);
    //kprintln_uint64(frame->rsi);
    //kprintln_uint64(frame->rdx);
    //kprintln_uint64(frame->rcx);
    //kprintln_uint64(frame->rbx);
    //kprintln_uint64(frame->rax);
    //if (current_thread->pid == 2) {
    //    // while (1) {}
    //}
    


 //   asm volatile ("mov %0, %%rsp" : : "r"((uint64_t)current_thread->return_rsp-current_thread->misaligned_by));//load back in aligned rsp
 //   asm volatile ("add %0, %%rsp" : : "r"(current_thread->misaligned_by));//move rsp back to its original position

 //   asm volatile (
	//	"pop %r15\n"
	//	"pop %r14\n"
	//	"pop %r13\n"
	//	"pop %r12\n"
	//	"pop %r11\n"
	//	"pop %r10\n"
	//	"pop %r9\n"
	//	"pop %r8\n"
	//	"pop %rbp\n"
	//	"pop %rdi\n"
	//	"pop %rsi\n"
	//	"pop %rdx\n"
	//	"pop %rcx\n"
	//	"pop %rbx\n"
	//	"pop %rax\n"
	//);
    //*lapic_eoi = 0;
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

    for (uint8_t vector = 0; vector < 129; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = true;
    }
    //HERE not sure if i have to manuall set them for gpf and page faults as well since i did define them in idt.asm as separate things

    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag

}
