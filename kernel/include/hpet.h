#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <kutils.h>

extern struct HPET_registers* hpet_regs;

//thanks to .confusedswede for the code

uint64_t hpet_get_elapsed_ns(void);

void hpet_reset(void);

void hpet_init(void);