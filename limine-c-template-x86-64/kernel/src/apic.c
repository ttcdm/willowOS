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
}

void init_mp(struct limine_mp_request* mp_request) {
    volatile uint32_t* icr_low = (uint32_t*) (ACPI_MADT->lapic_addr + 0x300);
    volatile uint32_t* icr_high = (uint32_t*) (ACPI_MADT->lapic_addr + 0x310);

    *icr_high = 0;
    *icr_low = 0x000322C0;//we init before start up. i don't think i'm supposed to set the delivery status bit but apparently it gets ignored by the cpu or something so i guess that it's fine/??
    kpass(10000000);
    *icr_high = 0;//HERE not sure if i should actually zero this out which also zeros out the reserved bits as well
    *icr_low = 0x00032340;//start up all other cores exluding self. args: 110010001101000000
    kpass(10000000000000000);
    kprintln("cpus:");
    kprintln_uint64(mp_request->response->cpu_count);
    // kprintln_uint64(mp_request->response->bsp_lapic_id);
    kprintln("---");
    for (int i = 0; i < mp_request->response->cpu_count; i++) {
        struct limine_mp_info** a = mp_request->response->cpus;
        kprintln_uint64(a[i]->processor_id);
        // kprintln_uint64(a[i]->lapic_id);
        // kprintln_uint64((uint64_t) a[i]->goto_address);
        // kprintln_uint64(a[i]->extra_argument);
        kprintln("--------");

        
    }


    //https://osdev.wiki/wiki/Symmetric_Multiprocessing


    volatile uint32_t stack_top = 0x10000;

    volatile uint8_t aprunning = 0;  // count how many APs have started
    volatile uint8_t bspdone;
    uint8_t bspid = 0;      // BSP id and spinlock flag
    // get the BSP's Local APIC ID
    __asm__ __volatile__ ("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(bspid) : : );

    // copy the AP trampoline code to a fixed address in low conventional memory (to address 0x0800:0x0000)
    memcpy((void*)0x8000, &ap_trampoline, 4096);
    int numcores = mp_request->response->cpu_count;
    // for each Local APIC ID we do...
    for(int i = 0; i < numcores; i++) {
        // do not start BSP, that's already running this code
        if(mp_request->response->cpus[i]->lapic_id == bspid) continue;
        // send INIT IPI
        *((volatile uint32_t*)(ACPI_MADT->lapic_addr + 0x280)) = 0;                                                                             // clear APIC errors
        *((volatile uint32_t*)(ACPI_MADT->lapic_addr + 0x310)) = (*((volatile uint32_t*)(ACPI_MADT->lapic_addr + 0x310)) & 0x00ffffff) | (i << 24);         // select AP
        *((volatile uint32_t*)(ACPI_MADT->lapic_addr + 0x300)) = (*((volatile uint32_t*)(ACPI_MADT->lapic_addr + 0x300)) & 0xfff00000) | 0x00C500;          // trigger INIT IPI
        do { __asm__ __volatile__ ("pause" : : : "memory"); }while(*((volatile uint32_t*)(ACPI_MADT->lapic_addr + 0x300)) & (1 << 12));         // wait for delivery
        *((volatile uint32_t*)(ACPI_MADT->lapic_addr + 0x310)) = (*((volatile uint32_t*)(ACPI_MADT->lapic_addr + 0x310)) & 0x00ffffff) | (i << 24);         // select AP
        *((volatile uint32_t*)(ACPI_MADT->lapic_addr + 0x300)) = (*((volatile uint32_t*)(ACPI_MADT->lapic_addr + 0x300)) & 0xfff00000) | 0x008500;          // deassert
        do { __asm__ __volatile__ ("pause" : : : "memory"); }while(*((volatile uint32_t*)(ACPI_MADT->lapic_addr + 0x300)) & (1 << 12));         // wait for delivery
        // mdelay(10);
        kpass(100000);                                                                                                                 // wait 10 msec
        // send STARTUP IPI (twice)
        for(int j = 0; j < 2; j++) {
            *((volatile uint32_t*)(ACPI_MADT->lapic_addr + 0x280)) = 0;                                                                     // clear APIC errors
            *((volatile uint32_t*)(ACPI_MADT->lapic_addr + 0x310)) = (*((volatile uint32_t*)(ACPI_MADT->lapic_addr + 0x310)) & 0x00ffffff) | (i << 24); // select AP
            *((volatile uint32_t*)(ACPI_MADT->lapic_addr + 0x300)) = (*((volatile uint32_t*)(ACPI_MADT->lapic_addr + 0x300)) & 0xfff0f800) | 0x000608;  // trigger STARTUP IPI for 0800:0000
            // udelay(200);    
            kpass(100000);                                                                                                    // wait 200 usec
            do { __asm__ __volatile__ ("pause" : : : "memory"); }while(*((volatile uint32_t*)(ACPI_MADT->lapic_addr + 0x300)) & (1 << 12)); // wait for delivery
        }
    }
    // release the AP spinlocks
    bspdone = 1;
    // now you'll have the number of running APs in 'aprunning'
}

// this C code can be anywhere you want it, no relocation needed
void ap_startup(int apicid) {
	// do what you want to do on the AP
	while(1);
}