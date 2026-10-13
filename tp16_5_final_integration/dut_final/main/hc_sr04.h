#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    float distance_cm;
    bool valid;
    int64_t timestamp_us;
} DistanceMeasurement;

void hc_sr04_init(void);

DistanceMeasurement hc_sr04_measure(void);
