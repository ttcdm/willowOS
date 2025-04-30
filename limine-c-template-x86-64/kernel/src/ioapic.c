#include <ioapic.h>

#include <uacpi/uacpi.h>//only include inside the c files i think
#include <uacpi/event.h>
#include <uacpi/tables.h>
#include <uacpi/acpi.h>

void init_ioapic(void) {
    //we only set up for ps/2 keyboard for now
    //irq 1 is used for keyboard interrupts
    //chatgpt said that we just use 00 as xy for the ioapic registers' memory addresses
    
    // outb(0x20,0x11);
    // outb(0xA0,0x11);
    // outb(0x21,0x20);
    // outb(0xA1,0x28);
    // outb(0x21,0x2);
    // outb(0xA1,0x4);
    // outb(0x21,1);
    // outb(0xA1,1);
    // outb(0x21,0xFF);
    // outb(0xA1,0xFF);
    volatile uint32_t* ioapic_base = (uint32_t*) 0xfec00000;
    map_page((uint64_t*) pml4_address_virt_glob, (uint64_t) ioapic_base, (uint64_t) ioapic_base, 0b11);//always remember to map devices and stuff since we're not kmalloc'ing
    volatile uint32_t* ioapic_ioregsel = (uint32_t*) (ioapic_base);
    volatile uint32_t* ioapic_iowin = (uint32_t*) (ioapic_base + 0x4);//4 because sizeof(uint32_t) is 4; 4 * 8 = 32
    uint64_t irq = 1;//1 is for keyboard interrupts
    volatile ioapic_reg_t keyboard_redtbl;

    keyboard_redtbl.vector = 224;
    keyboard_redtbl.destination = 0;
    keyboard_redtbl.mask = 0;

    keyboard_redtbl.delivery_mode = 0;
    keyboard_redtbl.destination_mode = 0;
    keyboard_redtbl.delivery_status = 0;
    keyboard_redtbl.pin_polarity = 0;
    keyboard_redtbl.remote_irr = 0;
    keyboard_redtbl.trigger_mode = 0;

    *ioapic_ioregsel = 0x10 + (irq * 2);//index for ioapic register (irq redirects);
    volatile uint32_t* keyboard_redtbl_low = (uint32_t*) (ioapic_iowin);//we write the low and then the high
    *keyboard_redtbl_low = (keyboard_redtbl.vector & 0xFF) |
                           (keyboard_redtbl.delivery_mode << 8) |
                           (keyboard_redtbl.destination_mode << 11) |
                           (keyboard_redtbl.destination << 12) |
                           (keyboard_redtbl.delivery_status << 13) |
                           (keyboard_redtbl.pin_polarity << 14) |
                        //    (keyboard_redtbl.remote_irr << 15) |
                           (keyboard_redtbl.trigger_mode << 15) |
                           (keyboard_redtbl.mask << 16);

    *ioapic_ioregsel = 0x10 + (irq * 2) + 1;//we only incrememnt by 1 because it's an index and not the actual address
    volatile uint32_t* keyboard_redtbl_high = (uint32_t*) (ioapic_iowin);
    *keyboard_redtbl_high = (keyboard_redtbl.destination << 24);//we shift left because msb is on the left hand side
    
    uint8_t status = inb(0x64);

    // uint64_t* a = find_ioapic();
    // kprintf("%x", a);

}

void *find_ioapic(void) {
    // uint64_t* aa = kmalloc_byte(4096);
    // uacpi_setup_early_table_access(aa, 4096);
    // struct MADT *madt = (struct MADT *)uacpi_get_table("APIC", 0);
    // uint8_t *ptr = (uint8_t *)madt->records;
    // uint8_t *end = (uint8_t *)madt + madt->h.length;

    // while (ptr < end) {
    //     struct MADTEntryHeader *hdr = (struct MADTEntryHeader *)ptr;

    //     if (hdr->type == 1 && hdr->length >= sizeof(struct MADTIOAPIC)) {
    //         struct MADTIOAPIC *io = (struct MADTIOAPIC *)ptr;
    //         return uacpi_kernel_map(io->address, 0x20); // map 32 bytes
    //     }

    //     ptr += hdr->length;
    // }

    // return NULL; // not found
}

