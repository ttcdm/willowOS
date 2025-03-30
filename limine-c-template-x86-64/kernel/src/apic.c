#include <apic.h>
#include <vmm.h>
#include <hpet.h>

void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

void pic_disable(void) {//https://wiki.osdev.org/8259_PIC#Disabling
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
    kprintln("pic disabled");
}

volatile uint64_t hpet_count_before;
volatile uint64_t hpet_count_difference;
volatile uint64_t lapic_timer_converted[NUM_CORES];
volatile uint32_t sleep_locks[NUM_CORES];

void kpass(size_t ms) {
    volatile uint32_t* lapic_id = (uint32_t*) (ACPI_MADT->lapic_addr + 0x20);
    volatile uint32_t* lapic_lvt_timer = (uint32_t*)(ACPI_MADT->lapic_addr + 0x320);
    volatile uint32_t* lapic_initial_count = (uint32_t*)(ACPI_MADT->lapic_addr + 0x380);
    volatile uint32_t* lapic_divider = (uint32_t*)(ACPI_MADT->lapic_addr + 0x3e0);
    volatile uint32_t* lapic_eoi = (uint32_t*)(ACPI_MADT->lapic_addr + 0xb0);
    *lapic_divider = 0b0011;
    *lapic_initial_count = (ms * lapic_timer_converted[(*lapic_id) >> 24]) / 1000;
    *lapic_lvt_timer = (uint32_t)0b00000000000001000010;//one shot with vector 66
    sleep_locks[(*lapic_id)>>24] = 1;
    while (sleep_locks[(*lapic_id)>>24]) {//it's offsetted by 24 bits
        asm volatile ("nop");
    }
}


//thanks to hildarthedorf for this code//

const struct MADT *ACPI_MADT;//HERE
const struct HPET* ACPI_HPET;

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
    char* apic_s = "APIC";
    bool match = 1;//match starts off as true. i should probably implement strcmp or strncmp
    for (int i = 0; i < 4; i++) {
        if (pSDT->signature[i] != apic_s[i]) {
            match = 0;
        }
    }
    if (match == 1) {
        ACPI_MADT = (const void*)pSDT;
        kprint("apic found. lapic address: ");
        kprintln_uint64((uint64_t)(ACPI_MADT->lapic_addr));
    }
    match = 1;
    char* hpet_s = "HPET";
    for (int i = 0; i < 4; i++) {
        if (pSDT->signature[i] != hpet_s[i]) {
            match = 0;
        }
    }
    if (match == 1) {
        ACPI_HPET = (const void*) pSDT;
        kprint("hpet found. register block address: ");
        kprintln_uint64((uint64_t) (ACPI_HPET->address.address));
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

void init_bsp_lapic(void) {//this can be rewritten to be a lot cleaner but it's explicit because i wanted to understand what was going on
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

    volatile uint32_t* lapic_id = (uint32_t*)(ACPI_MADT->lapic_addr + 0x20);
    volatile uint32_t* lapic_lint0 = (uint32_t*)(ACPI_MADT->lapic_addr + 0x350);
    volatile uint32_t* lapic_lint1 = (uint32_t*)(ACPI_MADT->lapic_addr + 0x360);
    volatile uint32_t*  lapic_lvt_timer = (uint32_t*)(ACPI_MADT->lapic_addr + 0x320);
    volatile uint32_t*  lapic_initial_count = (uint32_t*)(ACPI_MADT->lapic_addr + 0x380);
    volatile uint32_t*  lapic_current_count = (uint32_t*)(ACPI_MADT->lapic_addr + 0x390);
    volatile uint32_t*  lapic_divider = (uint32_t*)(ACPI_MADT->lapic_addr + 0x3e0);
    volatile uint32_t* lapic_eoi = (uint32_t*)(ACPI_MADT->lapic_addr + 0xb0);

    //just refer to the sdm for the layout and stuff in vol 3 ch 2

    
    *lapic_divider = 0b0011;//do not use 1. i don't know why but setting the divider value as 1 (1011) messes things up. remember that there's a 0 in the middle ish and that it's 0bx0xx
    //*lapic_initial_count = 10000;

    
    hpet_init();//we initialize here because of init stuff
    hpet_reset();
    *lapic_eoi = 0;
    *lapic_divider = 0b0011;
    *lapic_lvt_timer = (uint32_t)0b00000000000001000000;//reg;
    *lapic_initial_count = UINT32_MAX;

    hpet_count_before = hpet_get_elapsed_ns();//hpet_regs->main_counter;
    volatile uint64_t lapic_count_before = *lapic_current_count;
    for (int i = 1; i < 1000000; i++) {//random stuff to pass time
        volatile int a = i * i;
        if (a = i) {
            a = i / ((i * a)+1);
        }
    }
    volatile uint64_t lapic_count_after = *lapic_current_count;
    volatile uint64_t hpet_count_after = hpet_get_elapsed_ns();//hpet_regs->main_counter;
    //i'm not sure why, but running the function yields better results than getting the ticks directly and doing the calculations after
    //hpet_count_before *= ((hpet_regs->capabilities >> 32) / 1000000);
    //hpet_count_after *= ((hpet_regs->capabilities >> 32) / 1000000);

    *lapic_initial_count = 0;
    *lapic_eoi = 0;

    volatile uint64_t lapic_count_difference = lapic_count_before - lapic_count_after;
    hpet_count_difference = hpet_count_after - hpet_count_before;
    volatile uint64_t lapic_timer_multiplier = (1000000000000000 / (hpet_count_difference));
    lapic_timer_converted[(*lapic_id) >> 24] = (lapic_timer_multiplier * lapic_count_difference) / 1000000;
    kprint("apic timer ticks in 1 second: ");
    kprint_uint64(lapic_timer_converted[(*lapic_id) >> 24]);
    kprint(" divider: ");
    kprintln_uint64(16);
    *lapic_initial_count = UINT32_MAX;


}



void init_mp(struct limine_mp_request* mp_request) {
    kpass(200);
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
        kpass(3000);//wait for ~10ms which is 1x10^7 cycles at 1ghz. gonna implement a spinlock and a sync thing later and a better wait system
    }


}

void init_ap_lapic() {//same thing as init_bsp_lapic() but without the whole timer thing calculation thing. we only set the already calculated values here
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

    volatile uint32_t* lapic_id = (uint32_t*)(ACPI_MADT->lapic_addr + 0x20);
    volatile uint32_t* lapic_lint0 = (uint32_t*)(ACPI_MADT->lapic_addr + 0x350);
    volatile uint32_t* lapic_lint1 = (uint32_t*)(ACPI_MADT->lapic_addr + 0x360);
    volatile uint32_t* lapic_lvt_timer = (uint32_t*)(ACPI_MADT->lapic_addr + 0x320);
    volatile uint32_t* lapic_initial_count = (uint32_t*)(ACPI_MADT->lapic_addr + 0x380);
    volatile uint32_t* lapic_current_count = (uint32_t*)(ACPI_MADT->lapic_addr + 0x390);
    volatile uint32_t* lapic_divider = (uint32_t*)(ACPI_MADT->lapic_addr + 0x3e0);
    volatile uint32_t* lapic_eoi = (uint32_t*)(ACPI_MADT->lapic_addr + 0xb0);
    
    //hpet_init();
    hpet_reset();
    *lapic_eoi = 0;
    *lapic_divider = 0b0011;
    *lapic_lvt_timer = (uint32_t)0b00000000000001000000;//reg;
    *lapic_initial_count = UINT32_MAX;

    hpet_count_before = hpet_get_elapsed_ns();//hpet_regs->main_counter;
    volatile uint64_t lapic_count_before = *lapic_current_count;
    for (int i = 1; i < 1000000; i++) {//random stuff to pass time
        volatile int a = i * i;
        if (a = i) {
            a = i / ((i * a) + 1);
        }
    }
    volatile uint64_t lapic_count_after = *lapic_current_count;
    volatile uint64_t hpet_count_after = hpet_get_elapsed_ns();//hpet_regs->main_counter;
    //i'm not sure why, but running the function yields better results than getting the ticks directly and doing the calculations after
    //hpet_count_before *= ((hpet_regs->capabilities >> 32) / 1000000);
    //hpet_count_after *= ((hpet_regs->capabilities >> 32) / 1000000);

    *lapic_initial_count = 0;
    *lapic_eoi = 0;

    volatile uint64_t lapic_count_difference = lapic_count_before - lapic_count_after;
    hpet_count_difference = hpet_count_after - hpet_count_before;
    volatile uint64_t lapic_timer_multiplier = (1000000000000000 / (hpet_count_difference));
    lapic_timer_converted[(*lapic_id) >> 24] = (lapic_timer_multiplier * lapic_count_difference) / 1000000;
    kprint("apic timer ticks in 1 second: ");
    kprint_uint64(lapic_timer_converted[(*lapic_id) >> 24]);
    kprint(" divider: ");
    kprintln_uint64(16);
    *lapic_initial_count = UINT32_MAX;

    kprintln("local apic enabled");
}

