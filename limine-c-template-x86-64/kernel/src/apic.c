#include <apic.h>

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

void pic_disable(void) {//https://wiki.osdev.org/8259_PIC#Disabling
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}

uint8_t read_pic_mask(uint8_t pic_data_port) {
    uint8_t mask;
    asm volatile ("inb %1, %0" : "=a"(mask) : "Nd"(pic_data_port));
    return mask;
}

void check_pic_status() {
    uint8_t master_mask = read_pic_mask(0x21);  // Read master PIC mask
    uint8_t slave_mask = read_pic_mask(0xA1);   // Read slave PIC mask

    kprint("Master PIC Mask: ");
    kprintln_uint64(master_mask);
    kprint("Slave PIC Mask: ");
    kprintln_uint64(slave_mask);
}
