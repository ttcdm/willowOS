#include <tsc.h>
#include <apic.h>

//thanks to .confusedswede for this code

#define ONE_SECOND 1000000000ULL

uint64_t tsc_freq;

uint64_t rdtsc() {//https://wiki.osdev.org/Inline_Assembly/Examples#RDTSC
    uint32_t low, high;
    asm volatile("rdtsc":"=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}

uint64_t tsc_read() { return rdtsc(); }

uint64_t tsc_read_ns() { return tsc_ticks_to_ns(tsc_read()); }

uint64_t tsc_read_ms() {return tsc_read_ns() / 1000000; }

uint64_t tsc_read_s() {return tsc_read_ms() / 1000; }

uint64_t tsc_ns_to_ticks(uint64_t time) { return tsc_freq * time / ONE_SECOND; }

uint64_t tsc_ticks_to_ns(uint64_t ticks) {
    uint64_t quot = ticks / tsc_freq;
    uint64_t rem = ticks % tsc_freq;
    return quot * ONE_SECOND + (rem * ONE_SECOND) / tsc_freq;
}

void tsc_init() {
    // uint64_t ten_ms = 10000000;
    uint64_t tsc_start = tsc_read();
    //timer_wait_ns(ten_ms);
    kpass(10);
    uint64_t tsc_end = tsc_read();

    // 10 ms * 100 = 1 s, tsc_freq in Hz
    tsc_freq = (tsc_end - tsc_start) * 100;

    kprint("tsc initialized to ");
    kprint_uint64(tsc_freq);
    kprintln(" Hz");
}

//void test