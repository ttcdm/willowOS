#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <keyboard.h>
#include <gdt.h>
#include <idt.h>
#include <kutils.h>
#include <paging.h>
#include <vmm.h>
#include <apic.h>
#include <tsc.h>
#include <hpet.h>

#include <limine.h>

#include <flanterm/flanterm.h>
#include <flanterm/backends/fb.h>


//TODO: rewrite the chatgpt'd gdt tss and idt



// Set the base revision to 3, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0//may need to change it to 3 but idk
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0//may need to change it to 3 but idk
};


__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0//may need to change it to 3 but idk
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 3//HERE it's physical when it's 0 but the protocol says that it's physical when it's >=3 so idk
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_mp_request mp_request = {
    .id = LIMINE_MP_REQUEST,
    .revision = 0//HERE it's physical when it's 0 but the protocol says that it's physical when it's >=3 so idk
};



// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

void* memcpy(void* dest, const void* src, size_t n) {
    uint8_t* pdest = (uint8_t*)dest;
    const uint8_t* psrc = (const uint8_t*)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void* memset(void* s, int c, size_t n) {
    uint8_t* p = (uint8_t*)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void* memmove(void* dest, const void* src, size_t n) {
    uint8_t* pdest = (uint8_t*)dest;
    const uint8_t* psrc = (const uint8_t*)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    }
    else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i - 1] = psrc[i - 1];
        }
    }

    return dest;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const uint8_t* p1 = (const uint8_t*)s1;
    const uint8_t* p2 = (const uint8_t*)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
        asm("hlt");
    }
}

//need to make functions static so they persist and don't get overwritten for whatever reason. idk actually
void clear_framebuffer(struct limine_framebuffer* framebuffer, uint32_t color) {
    volatile uint32_t* fb_ptr = framebuffer->address;
    for (size_t i = 0; i < framebuffer->height * framebuffer->width; i++) {
        fb_ptr[i] = color;
    }
}

struct limine_memmap_entry** usable_memmaps_1_ptr;//HERE we use linked lists now so this shouldn't really matter. (strikethrough) for simplicity's sake i'm only gonna use the biggest entry for now which is 2gb ish (strikethrough)


struct usable_memmaps_region memmap_arr[32];//HERE. might run into issues with statically declaring the amount of memmaps

struct usable_memmaps_region* init_memmaps() {//HERE it's now every memmap there is.remember that it's plural
    int usable_memmaps_number = 0;//number of usable memmaps (1 indexed)
    for (int i = 0; i < memmap_request.response->entry_count; i++) {//i'm sorry for looping through it twice. there's probably a better way but i'm too lazy rn
        //if (memmap_request.response->entries[i]->type == 0) {
        //    usable_memmaps_number++;
        //}
        usable_memmaps_number++;
    }

    //not sure if i should put this as global
    struct limine_memmap_entry* usable_memmaps[usable_memmaps_number];//array of pointers to limine memmap entries//len() is 1 indexed
    usable_memmaps_number = 0;//reset to 0
    for (int i = 0; i < memmap_request.response->entry_count; i++) {/*
        if (memmap_request.response->entries[i]->type == 0) {
            usable_memmaps[usable_memmaps_number] = memmap_request.response->entries[i];
            usable_memmaps_number++;
        }*/
        usable_memmaps[usable_memmaps_number] = memmap_request.response->entries[i];
        usable_memmaps_number++;
    }

    usable_memmaps_1_ptr = &usable_memmaps[1];//for simplicity's sake i'm only gonna use the biggest entry for now which is 2gb ish


    for (int i = 0; i < usable_memmaps_number; i++) {
        char strr[32];
        uint64_to_string(usable_memmaps[i]->base, strr);
        // kprint(strr);
        // kprint("  ");
        uint64_to_string(usable_memmaps[i]->length, strr);
        // kprint(strr);
        // kprint("\n");
    }
    //HERE
    //usable_memmaps is plural
    //usable_memmap is singular



    memmap_arr[0].base = usable_memmaps[0]->base;
	memmap_arr[0].length = usable_memmaps[0]->length;
	memmap_arr[0].type = usable_memmaps[0]->type;
    //memset(memmap_arr[0].frame_bitmap, 0x00, (memmap_arr[0].length / 4096));//not sure if i'm supposed to convert it to a virtual address here for memset
    for (int i = 0; i < memmap_arr[0].length / 4096; i++) {
        memmap_arr[0].frame_bitmap[i] = 0x00;
	}
    memmap_arr[0].frame_bitmap[memmap_arr[0].length / 4096] = 0x02;//HERE we use 2 as the terminating character/value
	memmap_arr[0].next = NULL;
    //struct usable_memmaps_region* current = &first_memmap;
	struct usable_memmaps_region* current = &memmap_arr[0];
    for (int i = 1; i < usable_memmaps_number; i++) {

		struct usable_memmaps_region* usable_memmap = &memmap_arr[i];
		usable_memmap->base = usable_memmaps[i]->base;
		usable_memmap->length = usable_memmaps[i]->length;
        usable_memmap->type = usable_memmaps[i]->type;
        //memset(usable_memmap->frame_bitmap, 0x00, (usable_memmap->length / 4096));//not sure if i'm supposed to convert it to a virtual address here for memset
        for (int i = 0; i < usable_memmap->length / 4096; i++) {
            usable_memmap->frame_bitmap[i] = 0x00;
        }
        usable_memmap->frame_bitmap[usable_memmap->length / 4096] = 0x02;//HERE we use 2 as the terminating character/value; hopefully there's no off by 1 error
		usable_memmap->next = NULL;




        current->next = usable_memmap;
        current = current->next;
    }
    kprint("number of usable memmaps (1 indexed): ");
    kprintln_uint64(usable_memmaps_number);

    kprintln("initialized memmaps");
    return &memmap_arr[0];
}

struct limine_framebuffer* framebuffer;
struct flanterm_context* ft_ctx;

size_t kstrlen(char* msg) {
    size_t s = 0;
    if (!msg) return 0;
    while (msg[s] != '\0') {
        s++;
    }
    //for (size_t i = 0;; i++) {//strlen
    //    if (msg[i] == '\0') {
    //        break;
    //    }
    //    s++;
    //}
    return s;
}

void kprint(char* msg) {
    uint64_t s = kstrlen(msg);
    flanterm_write(ft_ctx, msg, s);
}

void kprintln(char* msg) {//i think the args are being pass through fine idk
    kprint(msg);
    kprint("\n");
}

void kprint_uint64(uint64_t num) {
    char strr[64];//might be a bit wasteful
    uint64_to_string(num, strr);
    kprint(strr);
}

void kprintln_uint64(uint64_t num) {
    kprint_uint64(num);
    kprint("\n");
}

void init_physical_memory() {//REMEMBER TO CALL THIS FIRST BEFORE ANYTHING
    starting_address = memmap_arr[0].base;
    hhdm_offset = hhdm_request.response->offset;
}

void test_memory() {//mini test
    uint64_t* x = kmalloc(10);
    for (int i = 0; i < 10; i++) {
        x[i] = i;
    }
    if (!((*x == 0) && (x[5] == 5)) && heap_page_head->status == 1 && heap_page_head->alloc_length == 10) {
        kprintln("memory test failed");
        return;
    }
    kfree(x);
    if ((heap_page_head->status == 0 && heap_page_head->alloc_length == 0)) {
        kprintln("memory test passed");
    }
    else {
		kprintln("memory test failed");
        print_heap(10);
    }
}


uint64_t get_rsdp_physical_address() {
    return rsdp_request.response->address;
}


// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
void kmain(void) {

    /*COLOR. may not be the best idea to define them as such simple names. maybe put it in a struct in the future*/
    uint32_t RED = 0xff0000;
    uint32_t GREEN = 0x00ff00;
    uint32_t BLUE = 0x0000ff;
    uint32_t WHITE = 0xffffff;
    uint32_t BLACK = 0x000000;

    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
        || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    framebuffer = framebuffer_request.response->framebuffers[0];

    ft_ctx = flanterm_fb_init(//https://github.com/mintsuki/flanterm
        NULL, NULL, framebuffer->address, framebuffer->width, framebuffer->height, framebuffer->pitch,//remember to use framebuffer->address as the framebuffer arg. framebuffer is just a struct, so we need to pass its actual address in as well
        framebuffer->red_mask_size, framebuffer->red_mask_shift, framebuffer->green_mask_size, framebuffer->green_mask_shift, framebuffer->blue_mask_size, framebuffer->blue_mask_shift, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 1, 0, 0, 0
    );

    clear_framebuffer(framebuffer, BLACK);

    // kprint("helloworld\n");
    kprintln("willowOS");

    struct usable_memmaps_region* memmap = init_memmaps();

    struct usable_memmaps_region* current_memmap = memmap;
    
    for (int i = 0; i < memmap_request.response->entry_count; i++) {//using 3 for now but it will break if the # of usable memmaps changes
        if (current_memmap->type == 0) {
        kprint("memmap region's base  : ");
        kprintln_uint64(current_memmap->base);
        kprint("memmap region's length: ");
        kprintln_uint64(current_memmap->length);
        kprint("memmap region's type  : ");
        kprintln_uint64(current_memmap->type);
        }
        current_memmap = current_memmap->next;
    }


    init_physical_memory();//make sure this is called first

    init_paging();

    //bp();

    uint64_t gdt_table[7];
    setup_gdt(gdt_table);
    struct GDTPtr gdtr;
    load_gdt(&gdtr, gdt_table);

    //setup_idt();//chatgpt'ed version
    //load_idt();

    idt_init();//not chatgpt'ed version
    struct TSS tss __attribute__((aligned(16)));
    setup_tss(&tss, gdt_table);
    load_tss();

    uint64_t heap_start_virt = init_heap();//must call to initialize heap

    test_memory();//make sure this gets called right after init_heap()

    //uint64_t frame_alloc_0 = 2146541568+4096;
    //free_frame(frame_alloc_0);



    pic_disable();//we disable the pic and set up the local apic (lapic)

    init_bsp_lapic();

    // tsc_init();//don't put in interrupt because it sends a vector of the same priority twice and it doesn't continue or something

    // init_mp(&mp_request);

    //hpet is initialized inside init_bsp_lapic();

    for (int i = 0; i < 10000000; i++) {
        asm volatile ("nop");
    }
    for (int i = 0;; i++) {
        kpass(1000);
        kprintln_uint64(i);
    }

    //asm volatile ("int $64");

    for (size_t i = 0; i < 100; i++) { volatile uint32_t* fb_ptr = framebuffer->address; fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xffffff; }
    //while (1) { asm("hlt"); }


    //flanterm_write(ft_ctx, "helloworld", 10);

    // We're done, just hang...
    hcf();
}


__attribute__((noreturn))
void start_ap() {//remember to not call any non processor specific init functions here like init_memmaps()
    kprintln("\ninitializing ap");
    asm volatile ("mov %0, %%cr3" :: "r"(pml4_address_virt_glob-hhdm_offset));//HERE must remember to mov the phys changed cr3 back into the ap. we use our own cr3 but the ap tries to load its own (probably the old one from the bsp) which causes it to boot loop when i try to access any memory regions because of a page fault and/or a gpf probably
    init_ap_lapic();//pretty sure writing to msr doesn't raise any flags so this should be fine for all ap's
    volatile uint32_t* lapic_svr = (uint32_t*) (ACPI_MADT->lapic_addr + 0xf0);//make sure this is 32 bits and not 64 bits
    // *lapic_svr &= ~0x100;//disable lapic
    // *lapic_svr |= 0x100;//enable lapic via the spurious interrupt vector register
    test_memory();//make sure this gets called right after init_heap()
    kprintln("lapic svr: ");
    kprintln_uint64_to_binary(*lapic_svr);
    volatile uint32_t* lapic_id = (uint32_t*) (ACPI_MADT->lapic_addr + 0x20);
    kprint("lapic id: ");
    kprintln_uint64((*lapic_id)>>24);
    kprintln("ap initialized!\n");

    while (1) {asm volatile ("hlt");}
}