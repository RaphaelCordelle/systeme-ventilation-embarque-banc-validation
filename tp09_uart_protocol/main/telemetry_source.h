#ifndef TELEMETRY_SOURCE_H
#define TELEMETRY_SOURCE_H

#include <stdbool.h>
typedef struct
{
    float temperature;
    float humidity;
    float pressure_hpa;
    bool valid;
} dut_telemetry_t;

bool telemetry_source_read(dut_telemetry_t *telemetry);

#endif
