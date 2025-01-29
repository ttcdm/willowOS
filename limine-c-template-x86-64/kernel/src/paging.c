#include <paging.h>
#include <kutils.h>


uint64_t page_directory[1024] __attribute__((aligned(4096)));
uint64_t first_page_table[1024] __attribute__((aligned(4096)));





//my own implemementation

uint8_t frame_bitmap[10000];//10000*4096 = 40.96 mb of memory. make sure it doesn't exceed the max amount of allocatable memory

uint64_t starting_address;
uint8_t last_alloced_frame;
void alloc_frame(void) {//not sure how this would be used with hhdm yet
    starting_address = (*usable_memmaps_1_ptr)->base;
    for (int i = 0; i < 10000; i++) {
        if (frame_bitmap[i] == 0x00) {
            frame_bitmap[i] == 0x01;
            last_alloced_frame = i;
        }
    }
}

void free_frame(void) {//kind of a stack allocator
    if (last_alloced_frame > (uint8_t) 0) {//to prevent 
        frame_bitmap[last_alloced_frame] = 0x00;
        last_alloced_frame--;
    }
}