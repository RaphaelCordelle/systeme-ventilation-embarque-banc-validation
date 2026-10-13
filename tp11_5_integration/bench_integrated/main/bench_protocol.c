#include "bench_protocol.h"

#include <string.h>

#include "esp_log.h"

#include "bench_version.h"
#include "bench_sensor.h"
#include "bench_uart.h"
#include "bench_timing.h"

static const char *TAG = "BENCH_PROTOCOL";

#define PROTOCOL_SOF 0xAA
#define PROTOCOL_VERSION 0x00

#define MSG_GET_STATUS 0x02
#define MSG_SET_MODE 0x03
#define MSG_GET_TELEMETRY 0x04
#define MSG_GET_VERSION 0x05

#define MSG_RESPONSE 0x80
#define MSG_ERROR 0x81


static uint8_t compute_xor(
    const uint8_t *data,
    size_t length
)
{
    uint8_t check = 0;

    for (size_t i = 0; i < length; i++)
    {
        check ^= data[i];
    }

    return check;
}


static uint16_t read_u16_le(
    const uint8_t *buffer
)
{
    return
        ((uint16_t)buffer[0]) |
        ((uint16_t)buffer[1] << 8);
}


static int16_t read_i16_le(
    const uint8_t *buffer
)
{
    return (int16_t)read_u16_le(buffer);
}


static uint32_t read_u32_le(
    const uint8_t *buffer
)
{
    return
        ((uint32_t)buffer[0]) |
        ((uint32_t)buffer[1] << 8) |
        ((uint32_t)buffer[2] << 16) |
        ((uint32_t)buffer[3] << 24);
}


bench_command_t bench_protocol_parse_command(
    const char *command
)
{
    if (
        strcmp(
            command,
            "GET_VERSIONS"
        ) == 0
    )
    {
        return BENCH_CMD_GET_VERSIONS;
    }

    if (
        strcmp(
            command,
            "GET_REFERENCE"
        ) == 0
    )
    {
        return BENCH_CMD_GET_REFERENCE;
    }

    if (
        strcmp(
            command,
            "FORWARD_COMMAND"
        ) == 0 ||
        strcmp(
            command,
            "FORWARD_COMMAND GET_STATUS"
        ) == 0
    )
    {
        return BENCH_CMD_FORWARD_COMMAND;
    }

    if (
        strcmp(
            command,
            "FORWARD_COMMAND OPEN"
        ) == 0
    )
    {
        return BENCH_CMD_OPEN;
    }

    if (
        strcmp(
            command,
            "FORWARD_COMMAND CLOSE"
        ) == 0
    )
    {
        return BENCH_CMD_CLOSE;
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

    uint64_t start =
        bench_timing_now_us();

    int sent =
        bench_uart_send(
            command,
            command_length
        );

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

        int received =
            bench_uart_receive(
                &byte,
                1
            );

        if (received <= 0)
        {
            break;
        }

        /*
         * Recherche du debut d'une trame valide.
         * Les octets residuels sont ignores jusqu'au SOF.
         */
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

        response[total_received] = byte;
        total_received++;

        /*
         * Une fois l'en-tete recu, calcul de la taille
         * totale attendue.
         */
        if (
            expected_length == 0 &&
            total_received >= 4
        )
        {
            expected_length =
                5U + response[3];

            /*
             * En-tete incoherent :
             * on abandonne cette fausse trame et
             * on recherche un nouveau SOF.
             */
            if (
                expected_length >
                response_max_length
            )
            {
                synchronized = false;
                total_received = 0;
                expected_length = 0;
                skipped_bytes++;

                continue;
            }
        }

        if (
            expected_length != 0 &&
            total_received >= expected_length
        )
        {
            break;
        }
    }

    uint64_t end =
        bench_timing_now_us();

    *latency_us =
        bench_timing_elapsed_us(
            start,
            end
        );

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


bool bench_get_dut_state(
    uint8_t *state
)
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

    request[4] =
        compute_xor(
            request,
            4
        );

    uint8_t response[6];
    uint64_t latency_us = 0;

    int received =
        bench_forward_to_dut(
            request,
            sizeof(request),
            response,
            sizeof(response),
            &latency_us
        );

    if (received != 6)
    {
        ESP_LOGW(
            TAG,
            "GET_STATUS invalid length: %d",
            received
        );

        return false;
    }

    if (
        response[0] != PROTOCOL_SOF ||
        response[1] != PROTOCOL_VERSION ||
        response[2] != MSG_RESPONSE ||
        response[3] != 1
    )
    {
        ESP_LOGW(
            TAG,
            "GET_STATUS invalid frame"
        );

        return false;
    }

    if (
        response[5] !=
        compute_xor(
            response,
            5
        )
    )
    {
        ESP_LOGW(
            TAG,
            "GET_STATUS invalid checksum"
        );

        return false;
    }

    *state =
        response[4];

    ESP_LOGI(
        TAG,
        "DUT state=%u latency=%llu us",
        (unsigned int)*state,
        (unsigned long long)latency_us
    );

    return true;
}


bool bench_get_dut_measurement(
    dut_measurement_t *measurement
)
{
    if (measurement == NULL)
    {
        return false;
    }

    measurement->valid =
        false;

    uint8_t request[5] = {
        PROTOCOL_SOF,
        PROTOCOL_VERSION,
        MSG_GET_TELEMETRY,
        0x00,
        0x00
    };

    request[4] =
        compute_xor(
            request,
            4
        );

    uint8_t response[14];
    uint64_t latency_us = 0;

    int received =
        bench_forward_to_dut(
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
            response[5] ==
                compute_xor(
                    response,
                    5
                )
        )
        {
            ESP_LOGW(
                TAG,
                "GET_TELEMETRY DUT error: %u",
                (unsigned int)response[4]
            );
        }
        else
        {
            ESP_LOGW(
                TAG,
                "GET_TELEMETRY invalid length: %d",
                received
            );
        }

        return false;
    }

    if (
        response[0] != PROTOCOL_SOF ||
        response[1] != PROTOCOL_VERSION ||
        response[2] != MSG_RESPONSE ||
        response[3] != 9
    )
    {
        ESP_LOGW(
            TAG,
            "GET_TELEMETRY invalid frame"
        );

        return false;
    }

    if (
        response[13] !=
        compute_xor(
            response,
            13
        )
    )
    {
        ESP_LOGW(
            TAG,
            "GET_TELEMETRY invalid checksum"
        );

        return false;
    }

    measurement->state =
        response[4];

    int16_t temperature_x100 =
        read_i16_le(
            &response[5]
        );

    uint16_t humidity_x100 =
        read_u16_le(
            &response[7]
        );

    uint32_t pressure_pa =
        read_u32_le(
            &response[9]
        );

    measurement->temperature =
        temperature_x100 /
        100.0f;

    measurement->humidity =
        humidity_x100 /
        100.0f;

    measurement->pressure =
        pressure_pa /
        100.0f;

    measurement->valid =
        true;

    ESP_LOGI(
        TAG,
        "DUT telemetry: state=%u T=%.2f H=%.2f P=%.2f latency=%llu us",
        (unsigned int)measurement->state,
        measurement->temperature,
        measurement->humidity,
        measurement->pressure,
        (unsigned long long)latency_us
    );

    return true;
}


bool bench_get_dut_version(
    uint8_t *major,
    uint8_t *minor,
    uint8_t *patch
)
{
    if (
        major == NULL ||
        minor == NULL ||
        patch == NULL
    )
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

    request[4] =
        compute_xor(
            request,
            4
        );

    uint8_t response[8];
    uint64_t latency_us = 0;

    int received =
        bench_forward_to_dut(
            request,
            sizeof(request),
            response,
            sizeof(response),
            &latency_us
        );

    if (received != 8)
    {
        ESP_LOGW(
            TAG,
            "GET_VERSION invalid length: %d",
            received
        );

        return false;
    }

    if (
        response[0] != PROTOCOL_SOF ||
        response[1] != PROTOCOL_VERSION ||
        response[2] != MSG_RESPONSE ||
        response[3] != 3 ||
        response[7] !=
            compute_xor(
                response,
                7
            )
    )
    {
        ESP_LOGW(
            TAG,
            "GET_VERSION invalid frame"
        );

        return false;
    }

    *major =
        response[4];

    *minor =
        response[5];

    *patch =
        response[6];

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


static bool bench_set_dut_mode(
    uint8_t mode
)
{
    uint8_t request[6] = {
        PROTOCOL_SOF,
        PROTOCOL_VERSION,
        MSG_SET_MODE,
        0x01,
        mode,
        0x00
    };

    request[5] =
        compute_xor(
            request,
            5
        );

    uint8_t response[6];
    uint64_t latency_us = 0;

    int received =
        bench_forward_to_dut(
            request,
            sizeof(request),
            response,
            sizeof(response),
            &latency_us
        );

    if (received != 6)
    {
        ESP_LOGW(
            TAG,
            "SET_MODE invalid length: %d",
            received
        );

        return false;
    }

    if (
        response[0] != PROTOCOL_SOF ||
        response[1] != PROTOCOL_VERSION
    )
    {
        ESP_LOGW(
            TAG,
            "SET_MODE invalid frame"
        );

        return false;
    }

    if (
        response[2] ==
        MSG_ERROR
    )
    {
        ESP_LOGW(
            TAG,
            "SET_MODE DUT error: %u",
            (unsigned int)response[4]
        );

        return false;
    }

    if (
        response[2] != MSG_RESPONSE ||
        response[3] != 1 ||
        response[4] != mode ||
        response[5] !=
            compute_xor(
                response,
                5
            )
    )
    {
        ESP_LOGW(
            TAG,
            "SET_MODE invalid response"
        );

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


void bench_protocol_handle_command(
    bench_command_t command
)
{
    switch (command)
    {
        case BENCH_CMD_GET_VERSIONS:
        {
            uint8_t major = 0;
            uint8_t minor = 0;
            uint8_t patch = 0;

            ESP_LOGI(
                TAG,
                "BENCH_VERSION=%s",
                BENCH_VERSION
            );

            if (
                bench_get_dut_version(
                    &major,
                    &minor,
                    &patch
                )
            )
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
                ESP_LOGW(
                    TAG,
                    "DUT_VERSION unavailable"
                );
            }

            break;
        }


        case BENCH_CMD_GET_REFERENCE:
        {
            bench_measurement_t measurement =
                bench_sensor_read();

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
                ESP_LOGW(
                    TAG,
                    "REF INVALID"
                );
            }

            break;
        }


        case BENCH_CMD_FORWARD_COMMAND:
        {
            uint8_t state = 0;

            if (
                bench_get_dut_state(
                    &state
                )
            )
            {
                ESP_LOGI(
                    TAG,
                    "FORWARD_COMMAND GET_STATUS -> state=%u",
                    (unsigned int)state
                );
            }
            else
            {
                ESP_LOGW(
                    TAG,
                    "FORWARD_COMMAND GET_STATUS failed"
                );
            }

            break;
        }


        case BENCH_CMD_OPEN:
        {
            if (
                bench_set_dut_mode(
                    1
                )
            )
            {
                ESP_LOGI(
                    TAG,
                    "FORWARD_COMMAND OPEN -> OK"
                );
            }
            else
            {
                ESP_LOGW(
                    TAG,
                    "FORWARD_COMMAND OPEN -> FAILED"
                );
            }

            break;
        }


        case BENCH_CMD_CLOSE:
        {
            if (
                bench_set_dut_mode(
                    0
                )
            )
            {
                ESP_LOGI(
                    TAG,
                    "FORWARD_COMMAND CLOSE -> OK"
                );
            }
            else
            {
                ESP_LOGW(
                    TAG,
                    "FORWARD_COMMAND CLOSE -> FAILED"
                );
            }

            break;
        }


        default:
        {
            ESP_LOGW(
                TAG,
                "Unknown command"
            );

            break;
        }
    }
}