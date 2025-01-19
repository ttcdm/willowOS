#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <keyboard.h>
#include <gdt.h>

//#include <gdt.h>


#include <limine.h>
//#include "limine.h"//to make gcc happy when compiling by myself

#include <flanterm/flanterm.h>
#include <flanterm/backends/fb.h>
//#include "flanterm/flanterm.h"
//#include "flanterm/backends/fb.h"



// Set the base revision to 3, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

//need to make functions static so they persist and don't get overwritten for whatever reason
static void clear_framebuffer(struct limine_framebuffer* framebuffer, uint32_t color) {
    volatile uint32_t* fb_ptr = framebuffer->address;
    for (size_t i = 0; i < framebuffer->height * framebuffer->width; i++) {
        fb_ptr[i] = color;
    }
}


struct GDTPtr gdtr;



void load_gdt(uint64_t* gdt_table, struct limine_framebuffer* framebuffer) {//chatgpt generated
    gdtr.limit = sizeof(uint64_t) * 6 - 1;  // GDT size
    gdtr.base = (uint64_t)gdt_table;        // GDT base address

    // Load the GDTR register
    asm volatile("lgdt %0" : : "m"(gdtr));

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

}


void load_tss(struct limine_framebuffer* framebuffer) {//chatgpt generated
    asm volatile("ltr %%ax" : : "a"(0x28)); // 0x28: Selector for TSS descriptor (GDT entry 5)
}


void uint16_to_hex(uint16_t value, char* buffer) {
    const char* hex_digits = "0123456789ABCDEF";

    // Fill the buffer with "0x" prefix
    buffer[0] = '0';
    buffer[1] = 'x';

    // Convert each nibble (4 bits) into a hexadecimal digit
    for (int i = 3; i >= 0; i--) {
        buffer[2 + (3 - i)] = hex_digits[(value >> (i * 4)) & 0xF];
    }

    // Null-terminate the string
    buffer[6] = '\0';
}



void uint64_to_hex(uint64_t value, char* buffer) {
    const char* hex_digits = "0123456789ABCDEF";

    // Fill the buffer with "0x" prefix
    buffer[0] = '0';
    buffer[1] = 'x';

    // Convert each nibble (4 bits) into a hexadecimal digit
    for (int i = 15; i >= 0; i--) {
        buffer[2 + (15 - i)] = hex_digits[(value >> (i * 4)) & 0xF];
    }

    // Null-terminate the string
    buffer[18] = '\0';
}

void output_gdt_entries(uint64_t* gdt_table, size_t entry_count, struct flanterm_context* ft_ctx) {
    char buffer[19]; // Buffer for the hexadecimal representation (18 chars + null terminator)

    for (size_t i = 0; i < entry_count; i++) {
        uint64_to_hex(gdt_table[i], buffer); // Convert the GDT entry to hex
        flanterm_write(ft_ctx, "GDT Entry ", 10);
        flanterm_write(ft_ctx, buffer, 18); // Write the hex value
        flanterm_write(ft_ctx, "\n", 1);    // Newline for readability
    }
}



#include <stdint.h>

// Define the IDT Entry structure
struct IDTEntry {
    uint16_t offset_low;  // Lower 16 bits of the handler address
    uint16_t selector;    // Code segment selector
    uint8_t ist;          // Interrupt Stack Table (IST) offset
    uint8_t type_attr;    // Type and attributes
    uint16_t offset_mid;  // Middle 16 bits of handler address
    uint32_t offset_high; // Upper 32 bits of handler address
    uint32_t zero;        // Reserved
} __attribute__((packed));

// Define the IDT Pointer structure
struct IDTPtr {
    uint16_t limit;       // Size of the IDT - 1
    uint64_t base;        // Address of the IDT
} __attribute__((packed));

// Declare the IDT and IDTPtr
#define IDT_ENTRIES 256
struct IDTEntry idt[IDT_ENTRIES];
struct IDTPtr idtr;

// Function to set an IDT entry
void set_idt_entry(int index, void* handler, uint16_t selector, uint8_t ist, uint8_t type_attr) {
    uint64_t handler_addr = (uint64_t)handler;

    idt[index].offset_low = handler_addr & 0xFFFF;
    idt[index].selector = selector;       // Code segment selector
    idt[index].ist = ist & 0x7;           // Interrupt Stack Table (3 bits)
    idt[index].type_attr = type_attr;     // Type and attributes
    idt[index].offset_mid = (handler_addr >> 16) & 0xFFFF;
    idt[index].offset_high = (handler_addr >> 32) & 0xFFFFFFFF;
    idt[index].zero = 0;                  // Reserved
}

// Basic fault handler
void fault_handler() {
    volatile uint32_t* fb_ptr = (volatile uint32_t*)0xB8000; // Framebuffer address (replace with your framebuffer address)
    fb_ptr[0] = 0xFF0000; // Red color to indicate a fault
    while (1) asm("hlt"); // Halt the CPU
}

// Function to set up the IDT
void setup_idt() {
    // Clear the IDT
    memset(idt, 0, sizeof(idt));

    // Set a handler for General Protection Fault (interrupt vector 13)
    set_idt_entry(13, fault_handler, 0x08, 0, 0x8E); // 0x08 = Kernel code selector, 0x8E = Present, interrupt gate

    // Load the IDT
    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint64_t)&idt;
    asm volatile("lidt %0" : : "m"(idtr));
}






// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
void kmain(void) {

    /*COLOR. may not be the best idea to define them as such simple names. maybe put it in a struct in the future*/
    uint32_t RED = 0xff0000;
    uint32_t GREEN = 0x00ff00;
    uint32_t BLUE = 0x0000ff;
    uint32_t WHITE = 0xffffff;
    uint32_t BLACK = 0x000000;

    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    // Note: we assume the framebuffer model is RGB with 32-bit pixels.

    //for (size_t i = 0; i < 100; i++) {
    //    volatile uint32_t *fb_ptr = framebuffer->address;
    //    fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xffffff;
    //}


    struct flanterm_context* ft_ctx = flanterm_fb_init(//https://github.com/mintsuki/flanterm
        NULL,
        NULL,
        framebuffer->address, framebuffer->width, framebuffer->height, framebuffer->pitch,//remember to use framebuffer->address as the framebuffer arg. framebuffer is just a struct, so we need to pass its actual address in as well
        framebuffer->red_mask_size, framebuffer->red_mask_shift,
        framebuffer->green_mask_size, framebuffer->green_mask_shift,
        framebuffer->blue_mask_size, framebuffer->blue_mask_shift,
        NULL,
        NULL, NULL,
        NULL, NULL,
        NULL, NULL,
        NULL, 0, 0, 1,
        0, 0,
        0
    );

    clear_framebuffer(framebuffer, BLACK);


    /* segments
    null
    kernel mode code
    kernel mode data
    user mode code
    user mode data
    task state
    */



    uint64_t gdt_table[7];
    gdt_table[0] = 0;
    gdt_table[1] = create_descriptor(0, 0xFFFFF, 0x9a, 0x0a);
    gdt_table[2] = create_descriptor(0, 0xFFFFF, 0x92, 0x0c);
    gdt_table[3] = create_descriptor(0, 0xFFFFF, 0xfa, 0x0a);
    gdt_table[4] = create_descriptor(0, 0xFFFFF, 0xf2, 0x0c);

    struct TSS tss __attribute__((aligned(16)));

    memset(&tss, 0, sizeof(tss));
    tss.rsp[0] = 0x80000; // Kernel stack pointer for privilege level 0
    tss.ist[0] = 0x90000; // Example IST stack pointer
    tss.iomap_base = sizeof(tss); // End of TSS structure

    // Step 3: Create the TSS descriptor in the GDT
    create_tss_descriptor((uint64_t)&tss, sizeof(tss) - 1, gdt_table, 5);//for gdt_5

    setup_idt(); // Initialize the IDT
    load_gdt(gdt_table, framebuffer);

    output_gdt_entries(gdt_table, 7, ft_ctx);

    char buffer[19];
    flanterm_write(ft_ctx, "TSS Address: ", 13);
    uint64_to_hex((uint64_t)&tss, buffer);
    flanterm_write(ft_ctx, buffer, 18);
    flanterm_write(ft_ctx, "\n", 1);

    flanterm_write(ft_ctx, "GDTR Base: ", 11);
    uint64_to_hex((uint64_t)gdt_table, buffer);
    flanterm_write(ft_ctx, buffer, 18);
    flanterm_write(ft_ctx, "\nGDTR Limit: ", 14);
    uint16_to_hex(gdtr.limit, buffer);
    flanterm_write(ft_ctx, buffer, 18);
    flanterm_write(ft_ctx, "\n", 1);

    flanterm_write(ft_ctx, "TSS Low: ", 9);
    uint64_to_hex(gdt_table[5], buffer);
    flanterm_write(ft_ctx, buffer, 18);
    flanterm_write(ft_ctx, "\n", 1);

    flanterm_write(ft_ctx, "TSS High: ", 10);
    uint64_to_hex(gdt_table[6], buffer);
    flanterm_write(ft_ctx, buffer, 18);
    flanterm_write(ft_ctx, "\n", 1);

    flanterm_write(ft_ctx, "TSS RSP0: ", 10);
    uint64_to_hex(tss.rsp[0], buffer);
    flanterm_write(ft_ctx, buffer, 18);
    flanterm_write(ft_ctx, "\n", 1);

    flanterm_write(ft_ctx, "TSS IST1: ", 10);
    uint64_to_hex(tss.ist[0], buffer);
    flanterm_write(ft_ctx, buffer, 18);
    flanterm_write(ft_ctx, "\n", 1);




    //asm volatile("ltr %%ax" : : "a"(0x28));



    //load_tss(framebuffer);
    load_tss_asm();
    for (size_t i = 0; i < 100; i++) { volatile uint32_t* fb_ptr = framebuffer->address; fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xffffff; }
    while (1) { asm("hlt"); }
    flanterm_write(ft_ctx, "helloworld", 10);

    //poll_kb(ft_ctx);

    // We're done, just hang...
    hcf();
}
