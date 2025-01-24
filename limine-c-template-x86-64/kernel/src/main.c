#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <keyboard.h>
#include <gdt.h>
#include <idt.h>
#include <kutils.h>

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
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
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

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

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
        asm ("hlt");
    }
}

//need to make functions static so they persist and don't get overwritten for whatever reason. idk actually
void clear_framebuffer(struct limine_framebuffer* framebuffer, uint32_t color) {
    volatile uint32_t* fb_ptr = framebuffer->address;
    for (size_t i = 0; i < framebuffer->height * framebuffer->width; i++) {
        fb_ptr[i] = color;
    }
}




struct limine_framebuffer* framebuffer;
struct flanterm_context* ft_ctx;


void kprint(char* msg) {
    int s = 0;
    for (int i = 0;; i++) {//strlen
        if (msg[i] == '\0') {
            break;
        }
        s++;
    }
    flanterm_write(ft_ctx, msg, s);
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

    kprint("helloworld\n");
    //char strrr[32];//remember to initialize strings as an actual array instead of a pointer sometimes to avoid a seg fault
    

    int usable_memmaps_number = 0;//number of usable memmaps (1 indexed)
    for (int i = 0; i < memmap_request.response->entry_count; i++) {//i'm sorry for looping through it twice. there's probably a better way but i'm too lazy rn
        if (memmap_request.response->entries[i]->type == 0) {
            usable_memmaps_number++;
        }
    }
    struct limine_memmap_entry* usable_memmaps[usable_memmaps_number];//array of pointers to limine memmap entries//len() is 1 indexed
    usable_memmaps_number = 0;//reset to 0
    for (int i = 0; i < memmap_request.response->entry_count; i++) {
        if (memmap_request.response->entries[i]->type == 0) {
            usable_memmaps[usable_memmaps_number] = memmap_request.response->entries[i];
            usable_memmaps_number++;
        }
    }

    for (int i = 0; i < usable_memmaps_number; i++) {
        char strr[32];
        uint64_to_string(usable_memmaps[i]->length, strr);
        kprint(strr);
        kprint("\n");
    }

    bp();


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
    

    //__asm__ volatile ("sidt %0" : "=m"(idtr_v));
    asm volatile ("int $64");

    for (size_t i = 0; i < 100; i++) { volatile uint32_t* fb_ptr = framebuffer->address; fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xffffff; }
    //while (1) { asm("hlt"); }


    //flanterm_write(ft_ctx, "helloworld", 10);

    // We're done, just hang...
    hcf();
}