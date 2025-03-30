#include <hpet.h>
#include <paging.h>
#include <apic.h>

struct HPET_registers* hpet_regs;


uint64_t hpet_get_elapsed_ns() {
    // Convert femtoseconds to nanoseconds.
    return hpet_regs->main_counter * ((hpet_regs->capabilities >> 32) / 1000000);//tick period is in top half of capabilities
}

void hpet_reset() {
    hpet_regs->main_counter = 0;
}

static void hpet_setup(uint64_t addr) {
    map_page((uint64_t*) pml4_address_virt_glob, addr, addr, 0b11);
    hpet_regs = (volatile struct HPET_registers*) addr;

    uint64_t hpet_period = hpet_regs->capabilities >> 32;

    hpet_regs->configuration = 1;

}

void hpet_init() {
    hpet_setup(ACPI_HPET->address.address);
    kprintln("hpet initialized");
}