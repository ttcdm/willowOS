#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include <flanterm/flanterm.h>
#include <flanterm/backends/fb.h>


void kprint(char* str);

void bp(void);

void uint64_to_string(uint64_t value, char* buffer);