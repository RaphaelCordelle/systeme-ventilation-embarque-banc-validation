#include "bench_protocol.h"

#include <string.h>

#include "esp_log.h"

#include "bench_compare.h"
#include "bench_sensor.h"
#include "bench_timing.h"
#include "bench_uart.h"
#include "bench_version.h"

static const char *TAG = "BENCH_PROTOCOL";

#define PROTOCOL_SOF 0xAA
#define PROTOCOL_VERSION 0x00

#define MSG_GET_STATUS 0x02
#define MSG_SET_MODE 0x03
#define MSG_GET_TELEMETRY 0x04
#define MSG_GET_VERSION 0x05
#define MSG_SET_AUTO 0x06
#define MSG_GET_DIAGNOSTICS 0x07

#define MSG_RESPONSE 0x80
#define MSG_ERROR 0x81

static uint8_t compute_xor(const uint8_t *data, size_t length)
{
    uint8_t check = 0;

    for (size_t i = 0; i < length; i++)
    {
        check ^= data[i];
    }

    return check;
}

static uint16_t read_u16_le(const uint8_t *buffer)
{
    return ((uint16_t)buffer[0]) | ((uint16_t)buffer[1] << 8);
}

static int16_t read_i16_le(const uint8_t *buffer)
{
    return (int16_t)read_u16_le(buffer);
}

static uint32_t read_u32_le(const uint8_t *buffer)
{
    return
        ((uint32_t)buffer[0]) |
        ((uint32_t)buffer[1] << 8) |
        ((uint32_t)buffer[2] << 16) |
        ((uint32_t)buffer[3] << 24);
}

static const char *state_name(uint8_t state)
{
    switch (state)
    {
        case 0: return "UNKNOWN";
        case 1: return "CLOSED";
        case 2: return "OPENING";
        case 3: return "OPEN";
        case 4: return "CLOSING";
        case 5: return "SAFE";
        default: return "INVALID";
    }
}

bench_command_t bench_protocol_parse_command(const char *command)
{
    if (strcmp(command, "GET_VERSIONS") == 0)
    {
        return BENCH_CMD_GET_VERSIONS;
    }

    if (strcmp(command, "GET_REFERENCE") == 0)
    {
        return BENCH_CMD_GET_REFERENCE;
    }

    if (
        strcmp(command, "FORWARD_COMMAND") == 0 ||
        strcmp(command, "FORWARD_COMMAND GET_STATUS") == 0
    )
    {
        return BENCH_CMD_FORWARD_COMMAND;
    }

    if (strcmp(command, "FORWARD_COMMAND OPEN") == 0)
    {
        return BENCH_CMD_OPEN;
    }

    if (strcmp(command, "FORWARD_COMMAND CLOSE") == 0)
    {
        return BENCH_CMD_CLOSE;
    }

    if (strcmp(command, "GET_DUT_TELEMETRY") == 0)
    {
        return BENCH_CMD_GET_DUT_TELEMETRY;
    }

    if (strcmp(command, "GET_DIAGNOSTICS") == 0)
    {
        return BENCH_CMD_GET_DIAGNOSTICS;
    }

    if (strcmp(command, "GET_SNAPSHOT") == 0)
    {
        return BENCH_CMD_GET_SNAPSHOT;
    }

    if (strcmp(command, "AUTO_ON") == 0)
    {
        return BENCH_CMD_AUTO_ON;
    }

    if (strcmp(command, "AUTO_OFF") == 0)
    {
        return BENCH_CMD_AUTO_OFF;
    }

    if (strcmp(command, "RUN_SMOKE") == 0)
    {
        return BENCH_CMD_RUN_SMOKE;
    }

    return BENCH_CMD_UNKNOWN;
}

int bench_forward_to_dut(
    const uint8_t *command,
    size_t command_length,
    uint8_t *response,
    size_t response_max_length,
    uint64_t *latency_us
)
{
    if (
        command == NULL ||
        response == NULL ||
        latency_us == NULL ||
        command_length == 0 ||
        response_max_length < 5
    )
    {
        return -1;
    }

    uint64_t start = bench_timing_now_us();

    int sent = bench_uart_send(command, command_length);
    if (sent != (int)command_length)
    {
        return -1;
    }

    size_t total_received = 0;
    size_t expected_length = 0;
    size_t skipped_bytes = 0;
    bool synchronized = false;

    while (total_received < response_max_length)
    {
        uint8_t byte = 0;
        int received = bench_uart_receive(&byte, 1);

        if (received <= 0)
        {
            break;
        }

        if (!synchronized)
        {
            if (byte != PROTOCOL_SOF)
            {
                skipped_bytes++;
                continue;
            }

            synchronized = true;
            response[0] = byte;
            total_received = 1;
            continue;
        }

        response[total_received++] = byte;

        if (expected_length == 0 && total_received >= 4)
        {
            expected_length = 5U + response[3];

            if (expected_length > response_max_length)
            {
                synchronized = false;
                total_received = 0;
                expected_length = 0;
                skipped_bytes++;
                continue;
            }
        }

        if (expected_length != 0 && total_received >= expected_length)
        {
            break;
        }
    }

    uint64_t end = bench_timing_now_us();
    *latency_us = bench_timing_elapsed_us(start, end);

    if (skipped_bytes > 0)
    {
        ESP_LOGW(
            TAG,
            "UART resync: %u byte(s) ignored",
            (unsigned int)skipped_bytes
        );
    }

    return (int)total_received;
}

bool bench_get_dut_state(uint8_t *state)
{
    if (state == NULL)
    {
        return false;
    }

    uint8_t request[5] = {
        PROTOCOL_SOF,
        PROTOCOL_VERSION,
        MSG_GET_STATUS,
        0x00,
        0x00
    };
    request[4] = compute_xor(request, 4);

    uint8_t response[6];
    uint64_t latency_us = 0;

    int received = bench_forward_to_dut(
        request,
        sizeof(request),
        response,
        sizeof(response),
        &latency_us
    );

    if (
        received != 6 ||
        response[0] != PROTOCOL_SOF ||
        response[1] != PROTOCOL_VERSION ||
        response[2] != MSG_RESPONSE ||
        response[3] != 1 ||
        response[5] != compute_xor(response, 5)
    )
    {
        ESP_LOGW(TAG, "GET_STATUS invalid response length=%d", received);
        return false;
    }

    *state = response[4];

    ESP_LOGI(
        TAG,
        "DUT state=%u(%s) latency=%llu us",
        (unsigned int)*state,
        state_name(*state),
        (unsigned long long)latency_us
    );

    return true;
}

bool bench_get_dut_measurement(dut_measurement_t *measurement)
{
    if (measurement == NULL)
    {
        return false;
    }

    *measurement = (dut_measurement_t){0};

    uint8_t request[5] = {
        PROTOCOL_SOF,
        PROTOCOL_VERSION,
        MSG_GET_TELEMETRY,
        0x00,
        0x00
    };
    request[4] = compute_xor(request, 4);

    uint8_t response[14];
    uint64_t latency_us = 0;

    int received = bench_forward_to_dut(
        request,
        sizeof(request),
        response,
        sizeof(response),
        &latency_us
    );

    if (received != 14)
    {
        if (
            received == 6 &&
            response[0] == PROTOCOL_SOF &&
            response[1] == PROTOCOL_VERSION &&
            response[2] == MSG_ERROR &&
            response[3] == 1 &&
            response[5] == compute_xor(response, 5)
        )
        {
            ESP_LOGW(TAG, "GET_TELEMETRY DUT error: %u", (unsigned int)response[4]);
        }
        else
        {
            ESP_LOGW(TAG, "GET_TELEMETRY invalid length: %d", received);
        }

        return false;
    }

    if (
        response[0] != PROTOCOL_SOF ||
        response[1] != PROTOCOL_VERSION ||
        response[2] != MSG_RESPONSE ||
        response[3] != 9 ||
        response[13] != compute_xor(response, 13)
    )
    {
        ESP_LOGW(TAG, "GET_TELEMETRY invalid frame");
        return false;
    }

    measurement->state = response[4];
    measurement->temperature = read_i16_le(&response[5]) / 100.0f;
    measurement->humidity = read_u16_le(&response[7]) / 100.0f;
    measurement->pressure = read_u32_le(&response[9]) / 100.0f;
    measurement->valid = true;

    ESP_LOGI(
        TAG,
        "DUT telemetry: state=%u(%s) T=%.2f H=%.2f P=%.2f latency=%llu us",
        (unsigned int)measurement->state,
        state_name(measurement->state),
        measurement->temperature,
        measurement->humidity,
        measurement->pressure,
        (unsigned long long)latency_us
    );

    return true;
}

bool bench_get_dut_diagnostics(dut_diagnostics_t *diagnostics)
{
    if (diagnostics == NULL)
    {
        return false;
    }

    *diagnostics = (dut_diagnostics_t){0};

    uint8_t request[5] = {
        PROTOCOL_SOF,
        PROTOCOL_VERSION,
        MSG_GET_DIAGNOSTICS,
        0x00,
        0x00
    };
    request[4] = compute_xor(request, 4);

    uint8_t response[17];
    uint64_t latency_us = 0;

    int received = bench_forward_to_dut(
        request,
        sizeof(request),
        response,
        sizeof(response),
        &latency_us
    );

    if (received != 17)
    {
        ESP_LOGW(TAG, "GET_DIAGNOSTICS invalid length: %d", received);
        return false;
    }

    if (
        response[0] != PROTOCOL_SOF ||
        response[1] != PROTOCOL_VERSION ||
        response[2] != MSG_RESPONSE ||
        response[3] != 12 ||
        response[16] != compute_xor(response, 16)
    )
    {
        ESP_LOGW(TAG, "GET_DIAGNOSTICS invalid frame");
        return false;
    }

    diagnostics->state = response[4];
    diagnostics->temperature = read_i16_le(&response[5]) / 100.0f;
    diagnostics->humidity = read_u16_le(&response[7]) / 100.0f;
    diagnostics->pressure = read_u32_le(&response[9]) / 100.0f;

    uint16_t distance_mm = read_u16_le(&response[13]);
    uint8_t flags = response[15];

    diagnostics->sensor_valid = (flags & (1U << 0)) != 0;
    diagnostics->distance_valid = (flags & (1U << 1)) != 0;
    diagnostics->obstacle = (flags & (1U << 2)) != 0;
    diagnostics->limit_open = (flags & (1U << 3)) != 0;
    diagnostics->limit_closed = (flags & (1U << 4)) != 0;
    diagnostics->contradiction = (flags & (1U << 5)) != 0;
    diagnostics->auto_enabled = (flags & (1U << 6)) != 0;
    diagnostics->distance_cm = diagnostics->distance_valid
        ? distance_mm / 10.0f
        : 0.0f;
    diagnostics->valid = true;

    ESP_LOGI(
        TAG,
        "DUT diag: state=%s auto=%d T=%.2f H=%.2f P=%.2f D=%s%.1f obstacle=%d limits[O=%d C=%d X=%d] latency=%llu us",
        state_name(diagnostics->state),
        diagnostics->auto_enabled,
        diagnostics->temperature,
        diagnostics->humidity,
        diagnostics->pressure,
        diagnostics->distance_valid ? "" : "INVALID/",
        diagnostics->distance_cm,
        diagnostics->obstacle,
        diagnostics->limit_open,
        diagnostics->limit_closed,
        diagnostics->contradiction,
        (unsigned long long)latency_us
    );

    return true;
}

bool bench_get_dut_version(uint8_t *major, uint8_t *minor, uint8_t *patch)
{
    if (major == NULL || minor == NULL || patch == NULL)
    {
        return false;
    }

    uint8_t request[5] = {
        PROTOCOL_SOF,
        PROTOCOL_VERSION,
        MSG_GET_VERSION,
        0x00,
        0x00
    };
    request[4] = compute_xor(request, 4);

    uint8_t response[8];
    uint64_t latency_us = 0;

    int received = bench_forward_to_dut(
        request,
        sizeof(request),
        response,
        sizeof(response),
        &latency_us
    );

    if (
        received != 8 ||
        response[0] != PROTOCOL_SOF ||
        response[1] != PROTOCOL_VERSION ||
        response[2] != MSG_RESPONSE ||
        response[3] != 3 ||
        response[7] != compute_xor(response, 7)
    )
    {
        ESP_LOGW(TAG, "GET_VERSION invalid response length=%d", received);
        return false;
    }

    *major = response[4];
    *minor = response[5];
    *patch = response[6];

    ESP_LOGI(
        TAG,
        "DUT version=%u.%u.%u latency=%llu us",
        (unsigned int)*major,
        (unsigned int)*minor,
        (unsigned int)*patch,
        (unsigned long long)latency_us
    );

    return true;
}

static bool bench_set_dut_mode(uint8_t mode)
{
    uint8_t request[6] = {
        PROTOCOL_SOF,
        PROTOCOL_VERSION,
        MSG_SET_MODE,
        0x01,
        mode,
        0x00
    };
    request[5] = compute_xor(request, 5);

    uint8_t response[6];
    uint64_t latency_us = 0;

    int received = bench_forward_to_dut(
        request,
        sizeof(request),
        response,
        sizeof(response),
        &latency_us
    );

    if (received != 6)
    {
        ESP_LOGW(TAG, "SET_MODE invalid length: %d", received);
        return false;
    }

    if (
        response[0] != PROTOCOL_SOF ||
        response[1] != PROTOCOL_VERSION ||
        response[2] == MSG_ERROR ||
        response[2] != MSG_RESPONSE ||
        response[3] != 1 ||
        response[4] != mode ||
        response[5] != compute_xor(response, 5)
    )
    {
        ESP_LOGW(TAG, "SET_MODE invalid response");
        return false;
    }

    ESP_LOGI(
        TAG,
        "SET_MODE %u acknowledged latency=%llu us",
        (unsigned int)mode,
        (unsigned long long)latency_us
    );

    return true;
}

bool bench_set_dut_auto(bool enabled)
{
    uint8_t request[6] = {
        PROTOCOL_SOF,
        PROTOCOL_VERSION,
        MSG_SET_AUTO,
        0x01,
        enabled ? 1U : 0U,
        0x00
    };
    request[5] = compute_xor(request, 5);

    uint8_t response[6];
    uint64_t latency_us = 0;

    int received = bench_forward_to_dut(
        request,
        sizeof(request),
        response,
        sizeof(response),
        &latency_us
    );

    if (
        received != 6 ||
        response[0] != PROTOCOL_SOF ||
        response[1] != PROTOCOL_VERSION ||
        response[2] != MSG_RESPONSE ||
        response[3] != 1 ||
        response[4] != (enabled ? 1U : 0U) ||
        response[5] != compute_xor(response, 5)
    )
    {
        ESP_LOGW(TAG, "SET_AUTO invalid response length=%d", received);
        return false;
    }

    ESP_LOGI(
        TAG,
        "SET_AUTO %s acknowledged latency=%llu us",
        enabled ? "ON" : "OFF",
        (unsigned long long)latency_us
    );

    return true;
}

static void log_snapshot(void)
{
    bench_measurement_t reference = bench_sensor_read();
    dut_diagnostics_t dut = {0};

    if (!bench_get_dut_diagnostics(&dut))
    {
        ESP_LOGW(TAG, "SNAPSHOT unavailable: DUT diagnostics failed");
        return;
    }

    if (!reference.valid)
    {
        ESP_LOGW(
            TAG,
            "SNAPSHOT state=%s auto=%d sensor_valid=%d distance_valid=%d obstacle=%d T=%.2f H=%.2f P=%.2f D=%.1f open=%d closed=%d contradiction=%d ref_valid=0",
            state_name(dut.state),
            dut.auto_enabled,
            dut.sensor_valid,
            dut.distance_valid,
            dut.obstacle,
            dut.temperature,
            dut.humidity,
            dut.pressure,
            dut.distance_cm,
            dut.limit_open,
            dut.limit_closed,
            dut.contradiction
        );
        return;
    }

    if (!dut.sensor_valid)
    {
        ESP_LOGW(
            TAG,
            "SNAPSHOT state=%s auto=%d sensor_valid=0 distance_valid=%d obstacle=%d T=%.2f H=%.2f P=%.2f D=%.1f open=%d closed=%d contradiction=%d ref_T=%.2f ref_H=%.2f ref_P=%.2f",
            state_name(dut.state),
            dut.auto_enabled,
            dut.distance_valid,
            dut.obstacle,
            dut.temperature,
            dut.humidity,
            dut.pressure,
            dut.distance_cm,
            dut.limit_open,
            dut.limit_closed,
            dut.contradiction,
            reference.temperature,
            reference.humidity,
            reference.pressure
        );
        return;
    }

    bench_comparison_t comparison = bench_compare_measurements(
        dut.temperature,
        dut.humidity,
        dut.pressure,
        reference.temperature,
        reference.humidity,
        reference.pressure
    );

    ESP_LOGI(
        TAG,
        "SNAPSHOT state=%s auto=%d sensor_valid=%d distance_valid=%d obstacle=%d T=%.2f H=%.2f P=%.2f D=%.1f open=%d closed=%d contradiction=%d ref_T=%.2f ref_H=%.2f ref_P=%.2f dT=%.2f dH=%.2f dP=%.2f",
        state_name(dut.state),
        dut.auto_enabled,
        dut.sensor_valid,
        dut.distance_valid,
        dut.obstacle,
        dut.temperature,
        dut.humidity,
        dut.pressure,
        dut.distance_cm,
        dut.limit_open,
        dut.limit_closed,
        dut.contradiction,
        reference.temperature,
        reference.humidity,
        reference.pressure,
        comparison.temperature_delta,
        comparison.humidity_delta,
        comparison.pressure_delta
    );
}

void bench_protocol_handle_command(bench_command_t command)
{
    switch (command)
    {
        case BENCH_CMD_GET_VERSIONS:
        {
            uint8_t major = 0;
            uint8_t minor = 0;
            uint8_t patch = 0;

            ESP_LOGI(TAG, "BENCH_VERSION=%s", BENCH_VERSION);

            if (bench_get_dut_version(&major, &minor, &patch))
            {
                ESP_LOGI(
                    TAG,
                    "DUT_VERSION=%u.%u.%u",
                    (unsigned int)major,
                    (unsigned int)minor,
                    (unsigned int)patch
                );
            }
            else
            {
                ESP_LOGW(TAG, "DUT_VERSION unavailable");
            }
            break;
        }

        case BENCH_CMD_GET_REFERENCE:
        {
            bench_measurement_t measurement = bench_sensor_read();
            if (measurement.valid)
            {
                ESP_LOGI(
                    TAG,
                    "REF T=%.2f H=%.2f P=%.2f",
                    measurement.temperature,
                    measurement.humidity,
                    measurement.pressure
                );
            }
            else
            {
                ESP_LOGW(TAG, "REF INVALID");
            }
            break;
        }

        case BENCH_CMD_FORWARD_COMMAND:
        {
            uint8_t state = 0;
            if (bench_get_dut_state(&state))
            {
                ESP_LOGI(
                    TAG,
                    "FORWARD_COMMAND GET_STATUS -> state=%u",
                    (unsigned int)state
                );
            }
            else
            {
                ESP_LOGW(TAG, "FORWARD_COMMAND GET_STATUS failed");
            }
            break;
        }

        case BENCH_CMD_OPEN:
            ESP_LOGI(
                TAG,
                "FORWARD_COMMAND OPEN -> %s",
                bench_set_dut_mode(1) ? "OK" : "FAILED"
            );
            break;

        case BENCH_CMD_CLOSE:
            ESP_LOGI(
                TAG,
                "FORWARD_COMMAND CLOSE -> %s",
                bench_set_dut_mode(0) ? "OK" : "FAILED"
            );
            break;

        case BENCH_CMD_GET_DUT_TELEMETRY:
        {
            dut_measurement_t measurement = {0};
            if (bench_get_dut_measurement(&measurement))
            {
                ESP_LOGI(
                    TAG,
                    "DUT_TELEMETRY state=%s T=%.2f H=%.2f P=%.2f",
                    state_name(measurement.state),
                    measurement.temperature,
                    measurement.humidity,
                    measurement.pressure
                );
            }
            else
            {
                ESP_LOGW(TAG, "DUT_TELEMETRY unavailable");
            }
            break;
        }

        case BENCH_CMD_GET_DIAGNOSTICS:
        {
            dut_diagnostics_t d = {0};
            if (bench_get_dut_diagnostics(&d))
            {
                ESP_LOGI(
                    TAG,
                    "DUT_DIAG state=%s auto=%d sensor_valid=%d distance_valid=%d obstacle=%d T=%.2f H=%.2f P=%.2f D=%.1f open=%d closed=%d contradiction=%d",
                    state_name(d.state),
                    d.auto_enabled,
                    d.sensor_valid,
                    d.distance_valid,
                    d.obstacle,
                    d.temperature,
                    d.humidity,
                    d.pressure,
                    d.distance_cm,
                    d.limit_open,
                    d.limit_closed,
                    d.contradiction
                );
            }
            else
            {
                ESP_LOGW(TAG, "DUT_DIAG unavailable");
            }
            break;
        }

        case BENCH_CMD_GET_SNAPSHOT:
            log_snapshot();
            break;

        case BENCH_CMD_AUTO_ON:
            ESP_LOGI(TAG, "AUTO -> %s", bench_set_dut_auto(true) ? "ON" : "FAILED");
            break;

        case BENCH_CMD_AUTO_OFF:
            ESP_LOGI(TAG, "AUTO -> %s", bench_set_dut_auto(false) ? "OFF" : "FAILED");
            break;

        case BENCH_CMD_RUN_SMOKE:
            /* Handled by main.c because the smoke module depends on this protocol. */
            break;

        case BENCH_CMD_UNKNOWN:
        default:
            ESP_LOGW(TAG, "Unknown command");
            break;
    }
}
