#include "bench_timing.h"

#include "esp_timer.h"

uint64_t bench_timing_now_us(void)
{
    return esp_timer_get_time();
}

uint64_t bench_timing_elapsed_us(
    uint64_t start,
    uint64_t end
)
{
    return end - start;
}