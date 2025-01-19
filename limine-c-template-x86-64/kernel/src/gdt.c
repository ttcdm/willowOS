#include <stdint.h>
#include <stddef.h>
#include <gdt.h>

#include <limine.h>

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


void create_tss_descriptor(uint64_t base, uint16_t limit, uint64_t* gdt_table, int index) {
    uint64_t low = 0;
    low |= (limit & 0xFFFF);                // Lower 16 bits of limit
    low |= (base & 0xFFFFFF) << 16;         // Lower 24 bits of base
    low |= (uint64_t)0x89 << 40;           // TSS type and attributes
    low |= ((limit >> 16) & 0xF) << 48;    // Upper 4 bits of limit
    low |= ((base >> 24) & 0xFF) << 56;    // Next 8 bits of base

    uint64_t high = 0;
    high |= (base >> 32) & 0xFFFFFFFF;     // Higher 32 bits of base

    gdt_table[index] = low;               // Lower 64 bits
    gdt_table[index + 1] = high;          // Upper 64 bits
}


