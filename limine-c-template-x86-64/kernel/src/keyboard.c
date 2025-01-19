#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>//not sure if i actually need to include so many .h files
#include <flanterm/flanterm.h>
#include <flanterm/backends/fb.h>

#include <keyboard.h>


static inline uint8_t inb(uint16_t port) {//chatgpt generated. try to rewrite in the future if possible
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}


static uint8_t get_key() {//chatgpt generated. try to rewrite in the future if possible
    uint8_t scan_code = inb(0x60); // Read from the keyboard's data port
    return scan_code;
}

static uint8_t get_kb_status() {
    uint8_t status = inb(0x64);
    return status;
}

static scancode_to_string(uint8_t scancode, char* str) {
    //since the uint8_t scancode can be represented in binary, we just shift it and mask part of it to grab the 4 bits we want
    char h[] = "0123456789abcdef";
    str[0] = '0';
    str[1] = 'x';
    str[2] = h[(scancode >> 4)];//high nibble
    //str[3] = h[(scancode << 4) >> 4];//low nibble//still not exactly sure why this line causes the low nibble to get dropped
    str[3] = h[(scancode & 0x0f)];//low nibble
    str[4] = '\0';
}

static bool is_lshift() {//may be a better idea to remove get_key() from here and just pass in the key directly idk
    uint8_t shift_pressed = 0x2a;
    uint8_t shift_released = 0xaa;
    uint8_t state = get_key();
    if (state == shift_released) {//state != shift_pressed || 
        return 0;
    }
    else if (state == shift_pressed) {//state != shift_released || 
        return 1;
    }
}

void poll_kb(struct flanterm_context* ft_ctx) {//not sure if this should be static. has a quirk where left shift is detected as pressed when the kernel first runs and it only gets reset when you press left shift
    static uint8_t mode = 0;
    static uint8_t prev_key;
    static bool lshift_status = 0;

    for (;;) {//may have weird bugs but i'm 95% sure it works fine
        lshift_status = is_lshift();
        uint8_t key = get_key();
        //key != prev_key was to allow another key to be pressed once and get an output while the previous was still down and still have an output instead of having to press the other key twice in order to get its output maybe???
        if ((key < 0x80 && key != 0x2a && mode == 0)) {// || ((key != prev_key) && (prev_key != (key | 0x80)))) {//key != prev key may have an issue because prev key was not given a proper value but its value would be 0x00 or something so idk
            prev_key = key;//it doesn't matter whether shift is pressed or not for key release because the scancodes are just the same. we dictate the scanmap so we handle the differentiation
            char c[2];
            c[1] = '\0';//doesn't seem right to just have to convert a char to a string this way
            if (lshift_status == 1) {
                c[0] = scanmap_set1_upper[key];
            }
            else if (lshift_status == 0) {
                c[0] = scanmap_set1[key];
            }
            if (c[0] == '\b') {//if it's backspace
                flanterm_write(ft_ctx, "\b \b", 3);
            }
            else {
                flanterm_write(ft_ctx, c, 1);//4 chars excluding the null terminating char. also may not be the best idea to be running the print even for keys that don't print anything
            }
            mode = 1;
        }
        else if ((key > 0x80 || key != prev_key) && mode == 1) {//need to reset back to mode 0 even if other keys are pressed. i'm not sure but the extra press when pressing down a different key whilst holding down the original key might've been cuz the keyboard automatically sends the release?? if another key is pressed or something or it just can't handle that many simultaneous keys at once or something can't handle that???
            mode = 0;//maybe set key back to 0x00 but idk
        }
    }
}