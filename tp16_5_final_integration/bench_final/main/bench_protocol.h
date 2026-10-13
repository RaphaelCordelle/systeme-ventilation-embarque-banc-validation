#ifndef BENCH_PROTOCOL_H
#define BENCH_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum
{
    BENCH_CMD_GET_VERSIONS,
    BENCH_CMD_GET_REFERENCE,
    BENCH_CMD_FORWARD_COMMAND,
    BENCH_CMD_OPEN,
    BENCH_CMD_CLOSE,
    BENCH_CMD_GET_DUT_TELEMETRY,
    BENCH_CMD_GET_DIAGNOSTICS,
    BENCH_CMD_GET_SNAPSHOT,
    BENCH_CMD_AUTO_ON,
    BENCH_CMD_AUTO_OFF,
    BENCH_CMD_RUN_SMOKE,
    BENCH_CMD_UNKNOWN

} bench_command_t;

typedef struct
{
    uint8_t state;
    float temperature;
    float humidity;
    float pressure;
    bool valid;
} dut_measurement_t;

typedef struct
{
    uint8_t state;
    float temperature;
    float humidity;
    float pressure;
    float distance_cm;

    bool sensor_valid;
    bool distance_valid;
    bool obstacle;
    bool limit_open;
    bool limit_closed;
    bool contradiction;
    bool auto_enabled;
    bool valid;
} dut_diagnostics_t;

bench_command_t bench_protocol_parse_command(const char *command);
void bench_protocol_handle_command(bench_command_t command);

int bench_forward_to_dut(
    const uint8_t *command,
    size_t command_length,
    uint8_t *response,
    size_t response_max_length,
    uint64_t *latency_us
);

bool bench_get_dut_state(uint8_t *state);
bool bench_get_dut_measurement(dut_measurement_t *measurement);
bool bench_get_dut_diagnostics(dut_diagnostics_t *diagnostics);
bool bench_get_dut_version(uint8_t *major, uint8_t *minor, uint8_t *patch);
bool bench_set_dut_auto(bool enabled);

#endif
