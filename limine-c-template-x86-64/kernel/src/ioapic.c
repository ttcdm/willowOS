#include <ioapic.h>

void init_ioapic(void) {
    //we only set up for ps/2 keyboard for now
    //irq 1 is used for keyboard interrupts

    (uint32_t*) (ACPI_MADT->lapic_addr + 0x20);


}