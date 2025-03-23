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
    kprintln("pic disabled");
}

void enable_lapic(void) {//this can be rewritten to be a lot cleaner but it's explicit because i wanted to understand what was going on
    uint32_t msr_high;
    uint32_t msr_low;
    uint64_t msr;
    __asm__ volatile("rdmsr" : "=a"(msr_low), "=d"(msr_high) : "c"(0x1b));
    msr = ((uint64_t) msr_high) << 32 | msr_low;//concatenate
    //msr &= ~0x800;//disable lapic
    msr |= 0x800;//enable lapic via the 11th bit (0 indexed)
    kprint("msr: ");
	kprintln_uint64_to_binary(msr);

	msr_low = (uint32_t) msr;
	msr_high = (uint32_t) (msr >> 32);

    __asm__ volatile("wrmsr" : : "a"(msr_low), "d"(msr_high) , "c"(0x1b));
}

const struct MADT *ACPI_MADT;

static void validate_sdt(const struct SDTHeader *pSDT) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < pSDT->length; ++i) {
        checksum += ((const uint8_t *)pSDT)[i];
    }
    if (checksum) panic("Invalid SDT. Expected 0, got %u\n", checksum);
}

static void validate_rsdp(const struct RSDP *pRSDP) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < sizeof(struct RSDP); ++i) {
        checksum += ((const uint8_t *)pRSDP)[i];
    }
    if (checksum) panic("Invalid RSDP. Expected 0, got %u\n", checksum);
}

static void validate_xsdp(const struct XSDP *pXSDP) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < sizeof(struct XSDP); ++i) {
        checksum += ((const uint8_t *)pXSDP)[i];
    }
    if (checksum) panic("Invalid XSDP. Expected 0, got %u\n", checksum);
}

static void parse_sdt(const struct SDTHeader *pSDT) {
    validate_sdt(pSDT);
    if (!strncmp(pSDT->signature, "APIC", 4)) {
        ACPI_MADT = (const void *)pSDT;
    }
}

void acpi_parse_rsdp(const void *pRSDP) {
    const struct XSDP *pXSDP = pRSDP;
    validate_rsdp(&pXSDP->rsdp);

    bool has_xsdp = pXSDP->rsdp.revision > 0;
    if (has_xsdp) {
        validate_xsdp(pXSDP);

        const struct XSDT *pXSDT = phy_to_virt(pXSDP->xsdt_address);
        validate_sdt(&pXSDT->h);

        const size_t num_sdts = (pXSDT->h.length - offsetof(struct XSDT, sdt64)) / sizeof(uint64_t);
        for (size_t i = 0; i < num_sdts; ++i) {
            parse_sdt(phy_to_virt(pXSDT->sdt64[i]));
        }
    } else {
        const struct RSDT *pRSDT = phy_to_virt(pXSDP->rsdp.rsdt_address);
        validate_sdt(&pRSDT->h);

        const size_t num_sdts = (pRSDT->h.length - offsetof(struct RSDT, sdt32)) / sizeof(uint32_t);
        for (size_t i = 0; i < num_sdts; ++i) {
            parse_sdt(phy_to_virt(pRSDT->sdt32[i]));        
        }
    }
}