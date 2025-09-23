#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <flanterm/flanterm.h>
#include <flanterm/backends/fb.h>

//this is the non chatgpt'ed version of the idt. it may be more error free

typedef struct idt_actual{
	uint16_t    isr_low;      // The lower 16 bits of the ISR's address
	uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
	uint8_t	    ist;          // The IST in the TSS that the CPU will load into RSP; set to zero for now
	uint8_t     attributes;   // Type and attributes; see the IDT page
	uint16_t    isr_mid;      // The higher 16 bits of the lower 32 bits of the ISR's address
	uint32_t    isr_high;     // The higher 32 bits of the ISR's address
	uint32_t    reserved;     // Set to zero
} __attribute__((packed)) idt_entry_t;

typedef struct idtr_t_actual {
	uint16_t	limit;
	uint64_t	base;
} __attribute__((packed)) idtr_t;


struct interrupt_frame {
	uint64_t rax;
	uint64_t rbx;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rsi;
	uint64_t rdi;
	uint64_t rbp;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;//rflags gets pushed first
	uint64_t rsp;
	uint64_t ss;
};



__attribute__((noreturn))
void exception_handler(void);

//__attribute__((interrupt))//apparently this disallowed the interrupt handler from returning for whatever reason
void interrupt_handler_custom(struct interrupt_frame* frame);
// void interrupt_handler_custom(void* frame);

void apic_tick_handler(struct interrupt_frame* frame);

void sleep_handler(struct interrupt_frame* frame);

void thread_handler(struct interrupt_frame* frame);

void thread_interrupter_handler(struct interrupt_frame* frame);

void thread_sleep_handler(struct interrupt_frame* frame);

void ps2_keyboard_handler(struct interrupt_frame* frame);

void page_fault_handler(struct interrupt_frame* frame);

void gpf_handler(struct interrupt_frame* frame);

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags);

void idt_init(void);





extern idtr_t idtr;

typedef struct {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed)) idtr_value;

// static idtr_value idtr_v;
