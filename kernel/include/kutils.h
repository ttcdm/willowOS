#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <apic.h>

#include <flanterm/flanterm.h>
#include <flanterm/backends/fb.h>

// #include <stdatomic.h>

// #include <oa_hash/oa_hash.h>

#include <string.h>


//the actual definitions of these functions are kinda scattered all over the place since i'm using this header file as a main link to expose util stuff

extern struct limine_framebuffer* framebuffer;
extern struct flanterm_context* ft_ctx;
//HERE not sure if this is correct but oh well
// https://stackoverflow.com/questions/4757565/what-are-forward-declarations-in-c
typedef struct mutex mutex_t;//forward declaration because we can't include mutex.h or else we'll get circular inclusions. i hope nothing breaks
extern mutex_t ft_ctx_mutex;

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
// void* memcpy(void* dest, const void* src, size_t n);
// void* memset(void* s, int c, size_t n);
// void* memmove(void* dest, const void* src, size_t n);
// int memcmp(const void* s1, const void* s2, size_t n);

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

void* scancode_to_string(uint8_t scancode);//RETURNS NULL AND NOT A STRING

uint8_t get_key();//chatgpt generated. try to rewrite in the future if possible

uint8_t get_kb_status();

bool is_lshift(uint8_t scancode);

void print_kb(uint8_t scancode);

uint64_t get_rsdp_physical_address();

// char* strncpy(char* dest, char* source, size_t num);
// int strcmp (const char *p1, const char *p2);

// //taken from libc
// char* strtok(char *s, const char *delim);
// char* strtok_r(char *s, const char *delim, char **last);

extern uint64_t smp_ticket;

//from osdev wiki on ustar
//https://wiki.osdev.org/USTAR
int oct2bin(unsigned char *str, int size);
int tar_lookup(unsigned char *archive, char *filename, char **out);

void itoa(int n, char s[]);
void itoa_reverse(char s[]);

extern struct limine_memmap_entry** usable_memmaps_pointer;
extern uint64_t usable_memmaps_amount;

uint64_t tar_lookup_bin(char* tarball, char* filename, char** file_data);

uint64_t rdmsr(uint32_t msr);
void wrmsr(uint32_t msr, uint64_t value);

// struct limine_framebuffer* framebuffer;