#include <stdint.h>
#include <stddef.h>
#include <gdt.h>

#include <limine.h>


// Declare a global IDT with 256 entries
struct IDTEntry idt[256];//chatgpt generated. not sure if i should actually do this.
struct IDTPtr idtr;



/*
void encodeGdtEntry(uint8_t* target, struct GDT source)
{
    // Check the limit to make sure that it can be encoded
    if (source.limit > 0xFFFFF) { kerror("GDT cannot encode limits larger than 0xFFFFF"); }

    // Encode the limit
    target[0] = source.limit & 0xFF;
    target[1] = (source.limit >> 8) & 0xFF;
    target[6] = (source.limit >> 16) & 0x0F;

    // Encode the base
    target[2] = source.base & 0xFF;
    target[3] = (source.base >> 8) & 0xFF;
    target[4] = (source.base >> 16) & 0xFF;
    target[7] = (source.base >> 24) & 0xFF;

    // Encode the access byte
    target[5] = source.access_byte;

    // Encode the flags
    target[6] |= (source.flags << 4);
}
*/

//uint64_t create_descriptor(uint32_t base, uint32_t limit, uint16_t flag) {
uint64_t create_descriptor(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {//chatgpt version included the access byte
    /*
    uint64_t descriptor = 0;

    // Create the high 32 bit segment
    descriptor = limit & 0x000F0000;         // set limit bits 19:16
    descriptor |= (flag << 8) & 0x00F0FF00;         // set type, p, dpl, s, g, d/b, l and avl fields
    descriptor |= (base >> 16) & 0x000000FF;         // set base bits 23:16
    descriptor |= base & 0xFF000000;         // set base bits 31:24

    // Shift by 32 to allow for low part of segment
    descriptor <<= 32;

    // Create the low 32 bit segment
    descriptor |= base << 16;                       // set base bits 15:0
    descriptor |= limit & 0x0000FFFF;               // set limit bits 15:0

    //printf("0x%.16llX\n", descriptor);
    */

    //-------vvv CHATGPT GENERATED vvv-----


    uint64_t descriptor = 0;

    // Ensure the limit fits within 20 bits
    if (limit > 0xFFFFF) {
        //printf("Error: Limit exceeds 20-bit range.\n");
        return 0;
    }

    // Encode the low 32 bits
    descriptor |= (limit & 0xFFFF);             // Lower 16 bits of limit
    descriptor |= (base & 0xFFFF) << 16;        // Lower 16 bits of base
    descriptor |= ((base >> 16) & 0xFF) << 32;  // Middle 8 bits of base
    descriptor |= ((uint64_t)access) << 40;     // Access byte

    // Encode the high 32 bits
    descriptor |= ((limit >> 16) & 0xF) << 48;  // Upper 4 bits of limit
    descriptor |= ((uint64_t)(flags & 0xF)) << 52; // Flags (granularity, size)
    descriptor |= ((uint64_t)(base >> 24) & 0xFF) << 56; // Upper 8 bits of base

    return descriptor;
}




void create_tss_descriptor(uint64_t base, uint16_t limit, uint64_t* gdt_table, int index) {//chatgpt generated
    uint64_t low = 0;
    low |= (limit & 0xFFFF);                // Lower 16 bits of limit
    low |= (base & 0xFFFFFF) << 16;         // Lower 24 bits of base
    low |= (uint64_t)0x89 << 40;           // TSS type and attributes
    low |= ((uint64_t)((limit >> 16) & 0xF) << 48);    // Upper 4 bits of limit
    low |= ((base >> 24) & 0xFF) << 56;    // Next 8 bits of base

    uint64_t high = 0;
    high |= (base >> 32) & 0xFFFFFFFF;     // Higher 32 bits of base

    gdt_table[index] = low;               // Lower 64 bits
    gdt_table[index + 1] = high;          // Upper 64 bits
}


void load_gdt(struct GDTPtr* gdtr, uint64_t* gdt_table) {//chatgpt generated
    gdtr->limit = sizeof(uint64_t) * 7 - 1;  // GDT size
    gdtr->base = (uint64_t)gdt_table;        // GDT base address

    asm volatile("cli"); // Disable interrupts
    // Load the GDTR register
    asm volatile("lgdt %0" : : "m"(*gdtr));

    // Perform a far jump to reload the CS segment
    asm volatile(
        "pushq $0x08\n"        // Push kernel code segment selector (GDT entry 1)
        "lea 1f(%%rip), %%rax\n"
        "pushq %%rax\n"
        "lretq\n"              // Long return to update CS
        "1:\n"
        :
    :
        : "rax", "memory"
        );

    // Reload other segment registers
    asm volatile(
        "mov $0x10, %%ax\n"    // Kernel data segment selector (GDT entry 2)
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        :
    :
        : "memory"
        );
    asm volatile("sti"); // Re-enable interrupts
}


void load_tss() {//chatgpt generated
    asm volatile("ltr %%ax" : : "a"(0x28)); // 0x28: Selector for TSS descriptor (GDT entry 5)
}

void output_gdt_entries(uint64_t* gdt_table, size_t entry_count) {//chatgpt generated
    char buffer[19]; // Buffer for the hexadecimal representation (18 chars + null terminator)
    /*
    for (size_t i = 0; i < entry_count; i++) {
        uint64_to_hex(gdt_table[i], buffer); // Convert the GDT entry to hex
        flanterm_write(ft_ctx, "GDT Entry ", 10);
        flanterm_write(ft_ctx, buffer, 18); // Write the hex value
        flanterm_write(ft_ctx, "\n", 1);    // Newline for readability
    }
    */
}


// A simple fault handler (will halt the CPU for now)
void fault_handler() {//chatgpt generated
    asm volatile("cli; hlt");
}

// Set up an IDT entry
void set_idt_entry(int vector, void (*handler)(), uint16_t selector, uint8_t type_attr) {//chatgpt generated
    uint64_t handler_address = (uint64_t)handler;

    idt[vector].offset_low = handler_address & 0xFFFF;
    idt[vector].selector = selector;
    idt[vector].ist = 0; // No alternate stack
    idt[vector].type_attr = type_attr;
    idt[vector].offset_middle = (handler_address >> 16) & 0xFFFF;
    idt[vector].offset_high = (handler_address >> 32) & 0xFFFFFFFF;
    idt[vector].zero = 0;
}

// Load the IDT into the CPU
void load_idt() {//chatgpt generated
    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint64_t)&idt;

    asm volatile("lidt %0" : : "m"(idtr)); // Load IDT register
}

// Initialize the IDT
void setup_idt() {//chatgpt generated
    // Clear the IDT (set all entries to 0)
    memset(idt, 0, sizeof(idt));

    // Set handlers for basic exceptions
    set_idt_entry(13, fault_handler, 0x08, 0x8E); // General Protection Fault (vector 13)
    set_idt_entry(14, fault_handler, 0x08, 0x8E); // Page Fault (vector 14)

    // Load the IDT
    load_idt();
}




void setup_gdt(uint64_t* gdt_table) {//chatgpt generated
    gdt_table[0] = 0;
    gdt_table[1] = create_descriptor(0, 0xFFFFF, 0x9a, 0x0a);
    gdt_table[2] = create_descriptor(0, 0xFFFFF, 0x92, 0x0c);
    gdt_table[3] = create_descriptor(0, 0xFFFFF, 0xfa, 0x0a);
    gdt_table[4] = create_descriptor(0, 0xFFFFF, 0xf2, 0x0c);
}

void setup_tss(struct TSS* tss, uint64_t* gdt_table) {//chatgpt generated
    memset(tss, 0, sizeof(tss));
    tss->rsp[0] = 0x80000; // Kernel stack pointer for privilege level 0
    tss->ist[0] = 0x90000; // Example IST stack pointer
    tss->iomap_base = sizeof(tss); // End of TSS structure
    create_tss_descriptor((uint64_t)tss, sizeof(*tss) - 1, gdt_table, 5);//for gdt_5

}