#include <apic.h>
#include <vmm.h>

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
        const struct XSDT* pXSDT = (struct XSDT*) (pXSDP->xsdt_address);
        kprintln("hi");
        validate_sdt(&pXSDT->h);
        const size_t num_sdts = (pXSDT->h.length - offsetof(struct XSDT, sdt64)) / sizeof(uint64_t);
        for (size_t i = 0; i < num_sdts; ++i) {
            parse_sdt((struct SDTHeader*) (pXSDT->sdt64[i]));
        }
    }
    else {
        map_page((uint64_t*)pml4_address_virt_glob, pXSDP->rsdp.rsdt_address, pXSDP->rsdp.rsdt_address, 0b11);
        const struct RSDT* pRSDT = (struct RSDT*) ((uint64_t) (pXSDP->rsdp.rsdt_address));
        validate_sdt(&pRSDT->h);

        const size_t num_sdts = (pRSDT->h.length - offsetof(struct RSDT, sdt32)) / sizeof(uint32_t);

        for (size_t i = 0; i < num_sdts; ++i) {
            parse_sdt((struct SDTHeader*) ((uint64_t)(pRSDT->sdt32[i])));
        }
    }
}

void init_lapic(void) {//this can be rewritten to be a lot cleaner but it's explicit because i wanted to understand what was going on
    uint64_t rsdp_phys_addr = get_rsdp_physical_address();
    kprint("rsdp physical address: ");
    kprintln_uint64(rsdp_phys_addr);
    uint64_t rsdp_virt_addr = rsdp_phys_addr;
    map_page((uint64_t*)pml4_address_virt_glob, rsdp_phys_addr, rsdp_virt_addr, 0b11);
    acpi_parse_rsdp((void*)(rsdp_virt_addr));
    
    //may need to map msr but idk
    uint32_t msr_high;
    uint32_t msr_low;
    uint64_t msr;
    __asm__ volatile("rdmsr" : "=a"(msr_low), "=d"(msr_high) : "c"(0x1b));
    msr = ((uint64_t)msr_high) << 32 | msr_low;//concatenate
    //msr &= ~0x800;//disable lapic
    msr |= 0x800;//enable lapic via the 11th msr bit (0 indexed)
    kprintln("msr: ");
    kprintln_uint64_to_binary(msr);
    msr_low = (uint32_t)msr;
    msr_high = (uint32_t)(msr >> 32);

    __asm__ volatile("wrmsr" : : "a"(msr_low), "d"(msr_high), "c"(0x1b));

    map_page((uint64_t*)pml4_address_virt_glob, ACPI_MADT->lapic_addr, ACPI_MADT->lapic_addr, 0b11);//not sure where to map this
    volatile uint32_t* lapic_svr = (uint32_t*) (ACPI_MADT->lapic_addr + 0xf0);//make sure this is 32 bits and not 64 bits
    //*lapic_svr &= ~0x100;//disable lapic
    *lapic_svr |= 0x100;//enable lapic via the spurious interrupt vector register
    kprintln("lapic svr: ");
    kprintln_uint64_to_binary(*lapic_svr);
    kprintln("local apic enabled");

    volatile uint32_t* lapic_lint0 = (uint32_t*)(ACPI_MADT->lapic_addr + 0x350);
    volatile uint32_t* lapic_lint1 = (uint32_t*)(ACPI_MADT->lapic_addr + 0x360);
    volatile uint32_t*  lapic_lvt_timer = (uint32_t*)(ACPI_MADT->lapic_addr + 0x320);
    volatile uint32_t*  lapic_initial_count = (uint32_t*)(ACPI_MADT->lapic_addr + 0x380);
    volatile uint32_t*  lapic_current_count = (uint32_t*)(ACPI_MADT->lapic_addr + 0x390);
    volatile uint32_t*  lapic_divider = (uint32_t*)(ACPI_MADT->lapic_addr + 0x3e0);


    uint32_t reg = 0;
    reg = 0b01;
    reg = reg << 1;
    reg |= 0b0;
    reg = reg << 1;
    reg |= 0b1;
    reg = reg << 1;
    reg |= 0b0;
    reg = reg << 1;
    reg |= 0b0;
    reg = reg << 1;
    reg = reg << 1;
    reg = reg << 3;
    reg |= 0b000;
    reg = reg << 8;
    reg |= 0x40;

    //kprint("reg: ");
    //kprintln_uint64_to_binary((uint64_t) reg);

    //just refer to the sdm for the layout and stuff in vol 3 ch 2

    *lapic_lvt_timer = (uint32_t) 0b00100000000001000001;// reg
    *lapic_divider = 0b111;
    *lapic_initial_count = 10000000;

    //HERE remember to clear the eoi after handling the interrupt to allow for future interrupts
    volatile uint32_t* lapic_eoi = (uint32_t*) (ACPI_MADT->lapic_addr + 0xb0);
    *lapic_eoi = 0;
}



void init_mp(struct limine_mp_request* mp_request) {
    void* ap_start_address = (void*) start_ap;
    kprint("cpus: ");
    kprintln_uint64(mp_request->response->cpu_count);
    kprint("bsp lapic id: ");
    kprintln_uint64(mp_request->response->bsp_lapic_id);
    // map_page((uint64_t*) pml4_address_virt_glob, (uint64_t) ap_start_address, (uint64_t) ap_start_address + hhdm_offset, 0b11);
    for (int i = 0; i < mp_request->response->cpu_count; i++) {
        struct limine_mp_info** a = mp_request->response->cpus;
        //i don't think it actually matters that i'm writing to the goto address of the bsp because it gets ignored i think
        // map_page((uint64_t*) (pml4_address_virt_glob), (uint64_t) &start_ap, (uint64_t) &start_ap, 0b11);
        a[i]->goto_address = ap_start_address;
        kpass(100000000);//wait for ~10ms which is 1x10^7 cycles at 1ghz. gonna implement a spinlock and a sync thing later and a better wait system
    }


}

