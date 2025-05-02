#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <kutils.h>
#include <paging.h>
#include <vmm.h>

#include "uacpi/uacpi.h"
#include "uacpi/kernel_api.h"

// // REQUIRED ALLOCATION
// void *uacpi_kernel_alloc(uacpi_size size) {
//     // return your_kmalloc(size);
// }

// void uacpi_kernel_free(void *ptr) {
//     // your_kfree(ptr);
// }

// // OPTIONAL BUT RECOMMENDED
// void *uacpi_kernel_map(uacpi_phys_addr phys, uacpi_size size) {
//     // return your_identity_map(phys, size); // or return (void *)phys if identity-mapped
// }

// void uacpi_kernel_unmap(void *virt, uacpi_size size) {
//     // No-op if identity-mapped
// }

// void uacpi_kernel_log(uacpi_log_level, const uacpi_char*) {

// }

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rsdp_address) {
    *out_rsdp_address = (uint64_t*) get_rsdp_physical_address();
    kprintf("rsdp physical address: %llu\n", *out_rsdp_address);
    return UACPI_STATUS_OK;//ALWAYS ALWAYS ALWAYS REMEMBER TO RETURN A VALUE IF. IF YOU'RE NOT SURE GO CHECK PLEASEEEEEEEE
}

void *uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len) {//i think this works
    uint64_t down = (addr & ~0xfff);
    uint64_t up;
    if (((addr + len) & 0xfff) == 0) {
        up = addr + len;
    }
    else {
       up = ((addr + len) & ~0xfff) + 4096;
    }
    uint64_t size = (up-down) / 4096;
    for (size_t i = 0; i < size; i++) {
        map_page((uint64_t*) pml4_address_virt_glob, down + (i * 4096), down + (i * 4096), 0b11);//identity map it i guess
    }
    return (void*) (addr);
}

void uacpi_kernel_unmap(void *addr, uacpi_size len) {
    kprintf("\x1b[31mUACPI: unmap not implemented yet\x1b[0m\n");
}

void uacpi_kernel_log(uacpi_log_level level, const uacpi_char* c) {
    kprintf("UACPI: level: %d code: %c\n", level, *c);
}