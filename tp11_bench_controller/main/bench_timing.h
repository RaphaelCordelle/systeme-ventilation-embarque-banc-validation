#ifndef BENCH_TIMING_H
#define BENCH_TIMING_H

#include <stdint.h>

uint64_t bench_timing_now_us(void);

uint64_t bench_timing_elapsed_us(
    uint64_t start,
    uint64_t end
);
#endif