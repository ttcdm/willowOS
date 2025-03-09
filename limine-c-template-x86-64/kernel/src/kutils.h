#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include <flanterm/flanterm.h>
#include <flanterm/backends/fb.h>


void kprint(char* str);
void kprintln(char* str);
void kprint_uint64(uint64_t num);
void kprintln_uint64(uint64_t num);

void bp(void);

void uint64_to_string(uint64_t value, char* buffer);

//codeium said to use extern here
extern struct limine_memmap_entry** usable_memmaps_1_ptr;//for simplicity's sake i'm only gonna use the biggest entry for now which is 2gb ish

