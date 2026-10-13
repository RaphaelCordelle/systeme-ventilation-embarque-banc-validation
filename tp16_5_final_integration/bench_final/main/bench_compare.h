#ifndef BENCH_COMPARE_H
#define BENCH_COMPARE_H

#include <stdbool.h>

typedef struct
{
    float temperature_delta;
    float humidity_delta;
    float pressure_delta;
    bool valid;

} bench_comparison_t;

bench_comparison_t bench_compare_measurements(
    float dut_temperature,
    float dut_humidity,
    float dut_pressure,
    float ref_temperature,
    float ref_humidity,
    float ref_pressure
);

#endif
