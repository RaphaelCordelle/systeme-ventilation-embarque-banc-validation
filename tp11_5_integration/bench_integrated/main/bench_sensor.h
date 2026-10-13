#ifndef BENCH_SENSOR_H
#define BENCH_SENSOR_H

#include <stdbool.h>

typedef struct
{
    float temperature;
    float humidity;
    float pressure;
    bool valid;
} bench_measurement_t;

void bench_sensor_init(void);

bench_measurement_t bench_sensor_read(void);

#endif