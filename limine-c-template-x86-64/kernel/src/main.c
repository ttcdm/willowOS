#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <keyboard.h>
#include <gdt.h>
#include <idt.h>
#include <kutils.h>
#include <paging.h>

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

struct limine_memmap_entry** usable_memmaps_1_ptr;//for simplicity's sake i'm only gonna use the biggest entry for now which is 2gb ish


struct usable_memmaps_region memmap_arr[16];//HERE. might run into issues with statically declaring the amount of memmaps

struct usable_memmaps_region* init_memmaps() {//remember that it's plural
    int usable_memmaps_number = 0;//number of usable memmaps (1 indexed)
    for (int i = 0; i < memmap_request.response->entry_count; i++) {//i'm sorry for looping through it twice. there's probably a better way but i'm too lazy rn
        if (memmap_request.response->entries[i]->type == 0) {
            usable_memmaps_number++;
        }
    }

    //not sure if i should put this as global
    struct limine_memmap_entry* usable_memmaps[usable_memmaps_number];//array of pointers to limine memmap entries//len() is 1 indexed
    usable_memmaps_number = 0;//reset to 0
    for (int i = 0; i < memmap_request.response->entry_count; i++) {
        if (memmap_request.response->entries[i]->type == 0) {
            usable_memmaps[usable_memmaps_number] = memmap_request.response->entries[i];
            usable_memmaps_number++;
        }
    }

    usable_memmaps_1_ptr = &usable_memmaps[1];//for simplicity's sake i'm only gonna use the biggest entry for now which is 2gb ish


    for (int i = 0; i < usable_memmaps_number; i++) {
        char strr[32];
        uint64_to_string(usable_memmaps[i]->base, strr);
        kprint(strr);
        kprint("  ");
        uint64_to_string(usable_memmaps[i]->length, strr);
        kprint(strr);
        kprint("\n");
    }
    //HERE
    //usable_memmaps is plural
    //usable_memmap is singular


 //   struct usable_memmaps_region first_memmap;
 //   first_memmap.base = usable_memmaps[0]->base;
 //   first_memmap.length = usable_memmaps[0]->length;
	//first_memmap.next = NULL;
    memmap_arr[0].base = usable_memmaps[0]->base;
	memmap_arr[0].length = usable_memmaps[0]->length;
    //memset(memmap_arr[0].frame_bitmap, 0x00, (memmap_arr[0].length / 4096));//not sure if i'm supposed to convert it to a virtual address here for memset
    for (int i = 0; i < memmap_arr[0].length / 4096; i++) {
        memmap_arr[0].frame_bitmap[i] = 0x00;
	}
    memmap_arr[0].frame_bitmap[memmap_arr[0].length / 4096] = 0x02;//we use 2 as the terminating character/value
	memmap_arr[0].next = NULL;
    //struct usable_memmaps_region* current = &first_memmap;
	struct usable_memmaps_region* current = &memmap_arr[0];
    for (int i = 1; i < usable_memmaps_number; i++) {
        //struct usable_memmaps_region* usable_memmap = usable_memmaps[i];
  //      usable_memmap->base = usable_memmaps[i]->base;
		//usable_memmap->length = usable_memmaps[i]->length;
  //      usable_memmap->next = NULL;
		struct usable_memmaps_region* usable_memmap = &memmap_arr[i];
		usable_memmap->base = usable_memmaps[i]->base;
		usable_memmap->length = usable_memmaps[i]->length;
        //memset(usable_memmap->frame_bitmap, 0x00, (usable_memmap->length / 4096));//not sure if i'm supposed to convert it to a virtual address here for memset
        for (int i = 0; i < usable_memmap->length / 4096; i++) {
            usable_memmap->frame_bitmap[i] = 0x00;
        }
        usable_memmap->frame_bitmap[usable_memmap->length / 4096] = 0x02;//we use 2 as the terminating character/value; hopefully there's no off by 1 error
		usable_memmap->next = NULL;




        current->next = usable_memmap;
        current = current->next;
    }
    //struct usable_memmaps_region* current_memmap = &first_memmap;

    //for (int i = 0; i < 4; i++) {
    //    kprint("base: ");
    //    kprintln_uint64(current_memmap->base);
    //    kprint("length: ");
    //    kprintln_uint64(current_memmap->length);
    //    current_memmap = current_memmap->next;
    //}
    //return &first_memmap;
    return &memmap_arr[0];
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
    //for (int i = 0; i < 10000; i++) {
    //    frame_bitmap[i] = 0x00;//not sure if using frame_bitmap[10000] = { 0 } as a declaration is bug free so i'm doing this just to be safe
    //}
    //starting_address = (*usable_memmaps_1_ptr)->base;//might need to align to 4096. also only declare this once because i think it was causing issues when it was called multiple times because of alloc_frame() redefining it multiple times
    starting_address = memmap_arr[0].base;
    hhdm_offset = hhdm_request.response->offset;
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



    struct usable_memmaps_region* memmap = init_memmaps();

    struct usable_memmaps_region* current_memmap = memmap;

    for (int i = 0; i < 4; i++) {
        kprint("base: ");
        kprintln_uint64(current_memmap->base);
        kprint("length: ");
        kprintln_uint64(current_memmap->length);
        current_memmap = current_memmap->next;
    }

    init_physical_memory();//make sure this is called first

    //init_paging();

    //volatile uint64_t* lptr;//old r/w to memory test
    //for (int i = 0; i < 10; i++) {
    //    uint64_t pa = alloc_frame();//pa = physical address; va = virtual address
    //    uint64_t va = pa + hhdm_offset;
    //    volatile uint64_t* ptr = (uint64_t*)va;
    //    *ptr = (uint64_t) i;
    //    if (frame_bitmap[i] == 1) {
    //        //kprintln_uint64(va);
    //    }
    //}
    //for (int i = 0; i < 10; i++) {
    //    if (frame_bitmap[i] == (uint8_t) 1) {
    //        uint64_t a = ((i * 4096) + starting_address) + hhdm_offset;
    //        lptr = (uint64_t*)a;
    //        //kprintln_uint64(*lptr);
    //        free_frame(a-hhdm_offset);
    //    }
    //}
    //for (int i = 0; i < 10; i++) {
    //    //kprintln("hi");
    //    if (frame_bitmap[i] == (uint8_t) 1) {
    //        uint64_t a = ((i * 4096) + starting_address) + hhdm_offset;
    //        lptr = (uint64_t*)a;
    //        //kprintln_uint64(*lptr);
    //    }
    //}

    

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


       //char buf[64];
       //uint64_t pml4;
       //asm volatile ("mov %%cr3, %0" : "=r"(pml4));//AT&T syntax so it's [src] [dest]
       //kprint("pml4: ");
       //uint64_to_string(pml4, buf);
       //kprint(buf);
       //kprint("\n");


       //__asm__ volatile ("mv %0, cr3 : "=r"");




       //__asm__ volatile ("sidt %0" : "=m"(idtr_v));
    asm volatile ("int $64");

    for (size_t i = 0; i < 100; i++) { volatile uint32_t* fb_ptr = framebuffer->address; fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xffffff; }
    //while (1) { asm("hlt"); }


    //flanterm_write(ft_ctx, "helloworld", 10);

    // We're done, just hang...
    hcf();
}