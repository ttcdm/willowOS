#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <flanterm/flanterm.h>
#include <flanterm/backends/fb.h>

// #include <stdatomic.h>







size_t kstrlen(char* msg);

void kprintf(char* fmt, ...);
void kprintf_interruptable(char* fmt, ...);
void kprint(char* str);
void kprintln(char* str);
void kprint_uint64(uint64_t num);
void kprintln_uint64(uint64_t num);
void kprintln_uint64_to_binary(uint64_t value);
void kprint_char(char c);

void bp(void);

void uint64_to_string(uint64_t value, char* buffer);

//codeium said to use extern here
extern struct limine_memmap_entry** usable_memmaps_1_ptr;//for simplicity's sake i'm only gonna use the biggest entry for now which is 2gb ish

//we should declare these in the header file
void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* s, int c, size_t n);
void* memmove(void* dest, const void* src, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);

uint64_t get_rsdp_physical_address(void);

void kpass(size_t ms);


//assert is from linuxmaster2.0 (nyaux)
#define assert(expression)                                                     \
  do {                                                                         \
    if (!(expression)) {                                                       \
      kprintf("Assertion failed in function %s. File: %s, line %d. %s\n",      \
              __func__, __FILE__, __LINE__, #expression);                       \
    while (1) {}                                                                \
    }                                                                          \
  } while (0)

uint64_t read_flags();
void irq_disable_save(bool *old_value);
void irq_restore(bool *status);


//these are all just taken from keyboard.h but slightly changed

extern const char scanmap_set1[128];
extern const char scanmap_set1_upper[128];

char* scancode_to_string(uint8_t scancode);

uint8_t get_key();//chatgpt generated. try to rewrite in the future if possible

uint8_t get_kb_status();

bool is_lshift(uint8_t scancode);

void print_kb(uint8_t scancode);