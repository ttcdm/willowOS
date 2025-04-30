#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <kutils.h>
#include <apic.h>
#include <paging.h>
#include <vmm.h>



typedef struct ioapic_reg_declared {
    uint8_t vector;
    // unsigned short delivery_mode;
    // char delivery_mode[3];
    unsigned int delivery_mode : 3;
    bool destination_mode;
    bool delivery_status;
    bool pin_polarity;
    bool remote_irr;
    bool trigger_mode;
    bool mask;
    uint32_t reserved0;
    short reserved1;
    short reserved2;
    short reserved3;
    char reserved4;
    uint8_t destination;
} ioapic_reg_t;

void init_ioapic(void);

void *find_ioapic(void);
