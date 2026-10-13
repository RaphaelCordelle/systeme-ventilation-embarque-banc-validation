#include "protocol.h"

#include <stddef.h>

#include "esp_log.h"

#include "state_machine.h"
#include "telemetry_source.h"
#include "uart_link.h"

static const char *TAG = "PROTOCOL";

#define DUT_VERSION_MAJOR 0
#define DUT_VERSION_MINOR 1
#define DUT_VERSION_PATCH 0

static void write_u16_le(
    uint8_t *destination,
    uint16_t value
)
{
    destination[0] = (uint8_t)(value & 0xFFU);
    destination[1] = (uint8_t)((value >> 8) & 0xFFU);
}

static void write_i16_le(
    uint8_t *destination,
    int16_t value
)
{
    write_u16_le(destination, (uint16_t)value);
}

static void write_u32_le(
    uint8_t *destination,
    uint32_t value
)
{
    destination[0] = (uint8_t)(value & 0xFFU);
    destination[1] = (uint8_t)((value >> 8) & 0xFFU);
    destination[2] = (uint8_t)((value >> 16) & 0xFFU);
    destination[3] = (uint8_t)((value >> 24) & 0xFFU);
}

uint8_t protocol_compute_check(
    const protocol_frame_t *frame
)
{
    uint8_t check = 0;

    check ^= frame->sof;
    check ^= frame->version;
    check ^= frame->type;
    check ^= frame->length;

    for (
        uint8_t i = 0;
        i < frame->length;
        i++
    )
    {
        check ^=
            frame->payload[i];
    }

    return check;
}

static void protocol_send_frame(
    const protocol_frame_t *frame
)
{
    uint8_t buffer[
        5 +
        PROTOCOL_MAX_PAYLOAD
    ];

    size_t index = 0;

    buffer[index++] =
        frame->sof;

    buffer[index++] =
        frame->version;

    buffer[index++] =
        frame->type;

    buffer[index++] =
        frame->length;

    for (
        uint8_t i = 0;
        i < frame->length;
        i++
    )
    {
        buffer[index++] =
            frame->payload[i];
    }

    buffer[index++] =
        protocol_compute_check(
            frame
        );

    uart_link_write(
        buffer,
        index
    );
}

static void protocol_send_error(
    uint8_t error_code
)
{
    protocol_frame_t response =
        {0};

    response.sof =
        PROTOCOL_SOF;

    response.version =
        PROTOCOL_VERSION;

    response.type =
        MSG_ERROR;

    response.length = 1;

    response.payload[0] =
        error_code;

    protocol_send_frame(
        &response
    );
}

void protocol_handle_frame(
    const protocol_frame_t *frame
)
{
    uint8_t expected_check =
        protocol_compute_check(
            frame
        );

    if (
        frame->check !=
        expected_check
    )
    {
        ESP_LOGW(
            TAG,
            "Invalid CHECK"
        );

        protocol_send_error(1);

        return;
    }

    if (
        frame->version !=
        PROTOCOL_VERSION
    )
    {
        ESP_LOGW(
            TAG,
            "Unsupported version"
        );

        protocol_send_error(2);

        return;
    }

    protocol_frame_t response =
        {0};

    response.sof =
        PROTOCOL_SOF;

    response.version =
        PROTOCOL_VERSION;

    switch (frame->type)
    {
        case MSG_HELLO:

            response.type =
                MSG_RESPONSE;

            response.length = 0;

            protocol_send_frame(
                &response
            );

            break;

        case MSG_GET_STATUS:

            response.type =
                MSG_RESPONSE;

            response.length = 1;

            response.payload[0] =
                (uint8_t)
                state_machine_get_state();

            protocol_send_frame(
                &response
            );

            break;

        case MSG_SET_MODE:

            if (
                frame->length != 1
            )
            {
                protocol_send_error(3);
                break;
            }

            response.type =
                MSG_RESPONSE;

            response.length = 1;

            response.payload[0] =
                frame->payload[0];

            protocol_send_frame(
                &response
            );

            break;

        case MSG_GET_TELEMETRY:
        {
            dut_telemetry_t telemetry = {0};

            if (
                !telemetry_source_read(&telemetry) ||
                !telemetry.valid
            )
            {
                ESP_LOGW(
                    TAG,
                    "Telemetry unavailable"
                );

                protocol_send_error(5);
                break;
            }

            const int16_t temperature_x100 =
                (int16_t)(telemetry.temperature * 100.0f);

            const uint16_t humidity_x100 =
                (uint16_t)(telemetry.humidity * 100.0f);

            const uint32_t pressure_pa =
                (uint32_t)(telemetry.pressure_hpa * 100.0f);

            response.type = MSG_RESPONSE;
            response.length = 9;
            response.payload[0] =
                (uint8_t)state_machine_get_state();

            write_i16_le(
                &response.payload[1],
                temperature_x100
            );

            write_u16_le(
                &response.payload[3],
                humidity_x100
            );

            write_u32_le(
                &response.payload[5],
                pressure_pa
            );

            protocol_send_frame(&response);
            break;
        }

        case MSG_GET_VERSION:

            response.type =
                MSG_RESPONSE;

            response.length = 3;

            response.payload[0] =
                DUT_VERSION_MAJOR;

            response.payload[1] =
                DUT_VERSION_MINOR;

            response.payload[2] =
                DUT_VERSION_PATCH;

            protocol_send_frame(
                &response
            );

            break;

        default:

            ESP_LOGW(
                TAG,
                "Unknown message type"
            );

            protocol_send_error(4);

            break;
    }
}
