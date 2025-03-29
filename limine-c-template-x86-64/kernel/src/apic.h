//#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <kutils.h>
#include <paging.h>



#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

#define PIT_FREQ 1193182//in hz

#define NUM_CORES 4//HERE remember to set this as the # of cores we use

// https://wiki.osdev.org/8259_PIC#Disabling


// https://wiki.osdev.org/Inline_Assembly/Examples#I/O_access
void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);

void pic_disable(void);
void init_bsp_lapic(void);
void init_ap_lapic(void);

void acpi_parse_rsdp(const void *pRSDP);

void init_mp(struct limine_mp_request* );

extern const struct MADT* ACPI_MADT;//HERE

void start_ap(void);

extern uint32_t lapic_count_before;
extern uint32_t lapic_count_difference;
extern uint32_t sleep_locks[NUM_CORES];


struct SDTHeader {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__ ((packed));

struct RSDP {//thanks to hildorthedorf for the help
    char signature[8];
    uint8_t checksum;
    char OEMID[6];
    uint8_t revision;
    uint32_t rsdt_address;
} __attribute__ ((packed));

struct XSDP {
    struct RSDP rsdp;

    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__ ((packed));

struct RSDT {
    struct SDTHeader h;
    uint32_t sdt32[];
} __attribute__ ((packed));

struct XSDT {
    struct SDTHeader h;
    uint64_t sdt64[];
} __attribute__ ((packed));

struct MADT {
    struct SDTHeader h;
    uint32_t lapic_addr;
    uint32_t flags;
    char records[];
} __attribute__ ((packed));

struct MADTEntryHeader {
    uint8_t type;
    uint8_t length;
} __attribute__ ((packed));

struct MADTProcessorLocalAPIC {
    struct MADTEntryHeader h;
    uint8_t processor_id;
    uint8_t apic_id;
    uint32_t flags;
} __attribute__ ((packed));

struct MADTIOAPIC {
    struct MADTEntryHeader h;
    uint8_t apic_id;
    uint8_t reserved;
    uint32_t address;
    uint32_t global_system_interrupt_base;
} __attribute__ ((packed));

struct MADTIOAPICInterruptSourceOverride {
    struct MADTEntryHeader h;
    uint8_t bus_source;
    uint8_t irq_source;
    uint32_t global_system_interrupt;
    uint16_t flags;
} __attribute__ ((packed));

struct MADTIOAPICNMISource {
    struct MADTEntryHeader h;
    uint8_t nmi_source;
    uint8_t reserved;
    uint16_t flags;
    uint32_t global_system_interrupt;
} __attribute__ ((packed));

struct MADTLocalAPICNMI {
    struct MADTEntryHeader h;
    uint8_t processor_id;
    uint16_t flags;
    uint8_t lint;
} __attribute__ ((packed));