#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <kutils.h>

#define ONE_SECOND 1000000000ULL

static inline uint64_t rdtsc();

extern uint64_t tsc_freq;

uint64_t tsc_read();

uint64_t tsc_read_ns();

uint64_t tsc_ns_to_ticks(uint64_t time);

uint64_t tsc_ticks_to_ns(uint64_t ticks);

void tsc_init();