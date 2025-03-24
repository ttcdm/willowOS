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



//thanks to hildarthedorf for this code//

const struct MADT *ACPI_MADT;//HERE

static void validate_sdt(const struct SDTHeader *pSDT) {
    //uint64_t ppa = alloc_frame();
    //map_page((uint64_t*)pml4_address_virt_glob, ppa, (uint64_t)(pSDT), 0b11);
    uint8_t checksum = 0;
    for (size_t i = 0; i < pSDT->length; ++i) {
        checksum += ((const uint8_t *)pSDT)[i];
    }
    //if (checksum) panic("Invalid SDT. Expected 0, got %u\n", checksum);
    if (checksum) { kprint("Invalid SDT. Expected 0, got "); kprintln_uint64(checksum); }
}

static void validate_rsdp(const struct RSDP *pRSDP) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < sizeof(struct RSDP); ++i) {
        checksum += ((const uint8_t *)pRSDP)[i];
    }
    //if (checksum) panic("Invalid RSDP. Expected 0, got %u\n", checksum);
    if (checksum) { kprint("Invalid RSDP. Expected 0, got "); kprintln_uint64(checksum); }
}

static void validate_xsdp(const struct XSDP *pXSDP) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < sizeof(struct XSDP); ++i) {
        checksum += ((const uint8_t *)pXSDP)[i];
    }
    //if (checksum) panic("Invalid XSDP. Expected 0, got %u\n", checksum);
    if (checksum) { kprint("Invalid XSDP. Expected 0, got "); kprintln_uint64(checksum); }
}

static void parse_sdt(const struct SDTHeader *pSDT) {
    validate_sdt(pSDT);
    kprint("sdt signature: ");
    kprint_char(pSDT->signature[0]);
    kprint_char(pSDT->signature[1]);
    kprint_char(pSDT->signature[2]);
    kprint_char(pSDT->signature[3]);
    kprintln("");
    char* s = "APIC";
    bool match = 1;//match starts off as true. i should probably implement strcmp or strncmp
    for (int i = 0; i < 4; i++) {
        if (pSDT->signature[i] != s[i]) {
            match = 0;
        }
    }
    if (match == 1) {
        ACPI_MADT = (const void*)pSDT;
        kprint("madt found. lapic address: ");
        kprintln_uint64((uint64_t)(ACPI_MADT->lapic_addr));
    }
}

void acpi_parse_rsdp(const void* pRSDP) {
    const struct XSDP* pXSDP = pRSDP;
    validate_rsdp(&pXSDP->rsdp);

    bool has_xsdp = pXSDP->rsdp.revision > 0;
    char oemid_str[7];
    for (int i = 0; i < 6; i++) {
        oemid_str[i] = pXSDP->rsdp.OEMID[i];
    }
    oemid_str[6] = '\0';
    kprint("OEMID: ");
    kprintln(oemid_str);
    if (has_xsdp) {
        validate_xsdp(pXSDP);
        map_page((uint64_t*)pml4_address_virt_glob, pXSDP->xsdt_address, pXSDP->xsdt_address, 0b11);
        const struct XSDT* pXSDT = pXSDP->xsdt_address;
        kprintln("hi");
        validate_sdt(&pXSDT->h);
        const size_t num_sdts = (pXSDT->h.length - offsetof(struct XSDT, sdt64)) / sizeof(uint64_t);
        for (size_t i = 0; i < num_sdts; ++i) {
            parse_sdt(pXSDT->sdt64[i]);
        }
    }
    else {
        map_page((uint64_t*)pml4_address_virt_glob, pXSDP->rsdp.rsdt_address, pXSDP->rsdp.rsdt_address, 0b11);
        const struct RSDT* pRSDT = pXSDP->rsdp.rsdt_address;
        validate_sdt(&pRSDT->h);

        const size_t num_sdts = (pRSDT->h.length - offsetof(struct RSDT, sdt32)) / sizeof(uint32_t);

        for (size_t i = 0; i < num_sdts; ++i) {
            parse_sdt(pRSDT->sdt32[i]);
        }
    }
}