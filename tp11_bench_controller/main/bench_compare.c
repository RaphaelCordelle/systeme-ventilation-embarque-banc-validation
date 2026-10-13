#include "bench_compare.h"

#include <math.h>

bench_comparison_t bench_compare_measurements(
    float dut_temperature,
    float dut_humidity,
    float dut_pressure,
    float ref_temperature,
    float ref_humidity,
    float ref_pressure
)
{
    bench_comparison_t comparison = {
        .temperature_delta = fabsf( //valeur abs
            dut_temperature - ref_temperature
        ),

        .humidity_delta = fabsf(
            dut_humidity - ref_humidity
        ),

        .pressure_delta = fabsf(
            dut_pressure - ref_pressure
        ),

        .valid = true
    };

    return comparison;
}