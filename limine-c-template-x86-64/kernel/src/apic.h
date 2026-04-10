#pragma once
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

#define NUM_CORES 64//HERE remember to set this as the # of cores we use. might actually be fine to leave it as more than we have beacuse it's only used to creating arrays and not loops

// https://wiki.osdev.org/8259_PIC#Disabling


// https://wiki.osdev.org/Inline_Assembly/Examples#I/O_access
void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);

void pic_disable(void);
void init_bsp_lapic(void);
void init_ap_lapic(void);

void acpi_parse_rsdp(const void *pRSDP);

void uacpi_init(void);

void init_mp(struct limine_mp_request* mp_request);

extern const struct MADT* ACPI_MADT;//HERE
extern const struct HPET* ACPI_HPET;

void start_ap(void);
// volatile uint64_t lapic_timer_converted[NUM_CORES];//commented because i don't think they're used in other files
extern volatile uint32_t sleep_locks[NUM_CORES];

void lapic_oneshot(uint64_t ms, uint8_t vector, uint8_t divider, bool ms_or_ticks);//0 for ms, 1 for ticks
void lapic_periodic(uint64_t ms, uint8_t vector, uint8_t divider, bool ms_or_ticks);//0 for ms, 1 for ticks



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

struct HPET_address_structure {
    uint8_t address_space_id;    // 0 - system memory, 1 - system I/O
    uint8_t register_bit_width;
    uint8_t register_bit_offset;
    uint8_t reserved;
    uint64_t address;
} __attribute__((packed));

struct HPET {
    struct SDTHeader h;
    uint8_t hardware_rev_id;
    uint8_t comparator_count:5;
    uint8_t counter_size:1;
    uint8_t reserved:1;
    uint8_t legacy_replacement:1;
    uint16_t pci_vendor_id;
    struct HPET_address_structure address;
    uint8_t hpet_number;
    uint16_t minimum_tick;
    uint8_t page_protection;
} __attribute__((packed));

struct HPET_registers {
    uint64_t capabilities;
    uint64_t reserved0;//spacers for offsets
    uint64_t configuration;
    uint64_t reserved1;
    uint64_t interrupt_status;
    uint64_t reserved2[25];
    uint64_t main_counter;
    //there's more but we don't implement them for now because we don't need them yet i don't think
} __attribute__ ((packed));