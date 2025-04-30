#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "uacpi/uacpi.h"
#include "uacpi/kernel_api.h"

// REQUIRED ALLOCATION
void *uacpi_kernel_alloc(uacpi_size size) {
    // return your_kmalloc(size);
}

void uacpi_kernel_free(void *ptr) {
    // your_kfree(ptr);
}

// OPTIONAL BUT RECOMMENDED
void *uacpi_kernel_map(uacpi_phys_addr phys, uacpi_size size) {
    // return your_identity_map(phys, size); // or return (void *)phys if identity-mapped
}

void uacpi_kernel_unmap(void *virt, uacpi_size size) {
    // No-op if identity-mapped
}

void uacpi_kernel_log(uacpi_log_level, const uacpi_char*) {

}


