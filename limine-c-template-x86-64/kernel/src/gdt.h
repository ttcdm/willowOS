#include <stdint.h>
#include <stddef.h>
//#include <stdio.h>

// Each define here is for a specific flag in the descriptor.
// Refer to the intel documentation for a description of what each one does.
#define SEG_DESCTYPE(x)  ((x) << 0x04) // Descriptor type (0 for system, 1 for code/data)
#define SEG_PRES(x)      ((x) << 0x07) // Present
#define SEG_SAVL(x)      ((x) << 0x0C) // Available for system use
#define SEG_LONG(x)      ((x) << 0x0D) // Long mode
#define SEG_SIZE(x)      ((x) << 0x0E) // Size (0 for 16-bit, 1 for 32)
#define SEG_GRAN(x)      ((x) << 0x0F) // Granularity (0 for 1B - 1MB, 1 for 4KB - 4GB)
#define SEG_PRIV(x)     (((x) &  0x03) << 0x05)   // Set privilege level (0 - 3)

#define SEG_DATA_RD        0x00 // Read-Only
#define SEG_DATA_RDA       0x01 // Read-Only, accessed
#define SEG_DATA_RDWR      0x02 // Read/Write
#define SEG_DATA_RDWRA     0x03 // Read/Write, accessed
#define SEG_DATA_RDEXPD    0x04 // Read-Only, expand-down
#define SEG_DATA_RDEXPDA   0x05 // Read-Only, expand-down, accessed
#define SEG_DATA_RDWREXPD  0x06 // Read/Write, expand-down
#define SEG_DATA_RDWREXPDA 0x07 // Read/Write, expand-down, accessed
#define SEG_CODE_EX        0x08 // Execute-Only
#define SEG_CODE_EXA       0x09 // Execute-Only, accessed
#define SEG_CODE_EXRD      0x0A // Execute/Read
#define SEG_CODE_EXRDA     0x0B // Execute/Read, accessed
#define SEG_CODE_EXC       0x0C // Execute-Only, conforming
#define SEG_CODE_EXCA      0x0D // Execute-Only, conforming, accessed
#define SEG_CODE_EXRDC     0x0E // Execute/Read, conforming
#define SEG_CODE_EXRDCA    0x0F // Execute/Read, conforming, accessed

#define GDT_CODE_PL0 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(0)     | SEG_CODE_EXRD

#define GDT_DATA_PL0 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(0)     | SEG_DATA_RDWR

#define GDT_CODE_PL3 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(3)     | SEG_CODE_EXRD

#define GDT_DATA_PL3 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(3)     | SEG_DATA_RDWR





struct GDT {//chatgpt generated
    uint16_t limit_low;    // Lower 16 bits of the segment limit
    uint16_t base_low;     // Lower 16 bits of the base
    uint8_t base_middle;   // Middle 8 bits of the base
    uint8_t access;        // Access byte
    uint8_t granularity;   // Flags and upper limit bits
    uint8_t base_high;     // Upper 8 bits of the base
};


struct TSS {//chatgpt generated
    uint32_t reserved;    // Reserved (always 0)
    uint64_t rsp[3];      // Stack pointers for privilege levels 0, 1, 2
    uint64_t reserved2;   // Reserved
    uint64_t ist[7];      // Interrupt Stack Table (IST) pointers
    uint64_t reserved3;   // Reserved
    uint16_t reserved4;   // Reserved
    uint16_t iomap_base;  // I/O map base address
} __attribute__((packed));

struct GDTPtr {//chatgpt generated
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void encodeGdtEntry(uint8_t* target, struct GDT source);

uint64_t create_descriptor(uint32_t base, uint32_t limit, uint8_t access, uint8_t flag);

void create_tss_descriptor(uint64_t base, uint16_t limit, uint64_t* gdt_table, int index);

//void load_gdt(uint64_t* gdt_table, struct limine_framebuffer* framebuffer);

//void load_tss();