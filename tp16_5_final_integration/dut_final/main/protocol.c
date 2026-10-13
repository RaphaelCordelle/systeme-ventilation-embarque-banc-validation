#include "protocol.h"

#include <stddef.h>
#include <stdint.h>

#include "app_types.h"
#include "control_task.h"
#include "dut_version.h"
#include "state_machine.h"
#include "system_snapshot.h"
#include "uart_link.h"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

static const char *TAG = "PROTOCOL";
static QueueHandle_t queue_command = NULL;

static void write_u16_le(uint8_t *destination, uint16_t value)
{
    destination[0] = (uint8_t)(value & 0xFFU);
    destination[1] = (uint8_t)((value >> 8) & 0xFFU);
}

static void write_i16_le(uint8_t *destination, int16_t value)
{
    write_u16_le(destination, (uint16_t)value);
}

static void write_u32_le(uint8_t *destination, uint32_t value)
{
    destination[0] = (uint8_t)(value & 0xFFU);
    destination[1] = (uint8_t)((value >> 8) & 0xFFU);
    destination[2] = (uint8_t)((value >> 16) & 0xFFU);
    destination[3] = (uint8_t)((value >> 24) & 0xFFU);
}

void protocol_set_command_queue(QueueHandle_t command_queue)
{
    queue_command = command_queue;
}

uint8_t protocol_compute_check(const protocol_frame_t *frame)
{
    uint8_t check = 0;

    check ^= frame->sof;
    check ^= frame->version;
    check ^= frame->type;
    check ^= frame->length;

    for (uint8_t i = 0; i < frame->length; i++)
    {
        check ^= frame->payload[i];
    }

    return check;
}

void protocol_send_frame(const protocol_frame_t *frame)
{
    uint8_t buffer[5 + PROTOCOL_MAX_PAYLOAD];
    size_t index = 0;

    buffer[index++] = frame->sof;
    buffer[index++] = frame->version;
    buffer[index++] = frame->type;
    buffer[index++] = frame->length;

    for (uint8_t i = 0; i < frame->length; i++)
    {
        buffer[index++] = frame->payload[i];
    }

    buffer[index++] = protocol_compute_check(frame);
    uart_link_write(buffer, index);
}

static void protocol_send_error(uint8_t error_code)
{
    protocol_frame_t response = {0};
    response.sof = PROTOCOL_SOF;
    response.version = PROTOCOL_VERSION;
    response.type = MSG_ERROR;
    response.length = 1;
    response.payload[0] = error_code;
    protocol_send_frame(&response);
}

static bool queue_user_command(user_command_t command)
{
    if (queue_command == NULL)
    {
        protocol_send_error(6);
        return false;
    }

    if (xQueueSend(queue_command, &command, 0) != pdTRUE)
    {
        protocol_send_error(7);
        return false;
    }

    return true;
}

static void fill_telemetry_payload(protocol_frame_t *response, const sensor_snapshot_t *snapshot)
{
    int16_t temperature_x100 = (int16_t)(snapshot->temperature * 100.0f);
    uint16_t humidity_x100 = (uint16_t)(snapshot->humidity * 100.0f);
    uint32_t pressure_pa = (uint32_t)(snapshot->pressure_hpa * 100.0f);

    response->type = MSG_RESPONSE;
    response->length = 9;
    response->payload[0] = (uint8_t)state_machine_get_state();
    write_i16_le(&response->payload[1], temperature_x100);
    write_u16_le(&response->payload[3], humidity_x100);
    write_u32_le(&response->payload[5], pressure_pa);
}

static void fill_diagnostics_payload(protocol_frame_t *response, const sensor_snapshot_t *snapshot)
{
    int16_t temperature_x100 = (int16_t)(snapshot->temperature * 100.0f);
    uint16_t humidity_x100 = (uint16_t)(snapshot->humidity * 100.0f);
    uint32_t pressure_pa = (uint32_t)(snapshot->pressure_hpa * 100.0f);
    uint16_t distance_mm = snapshot->distance_valid
        ? (uint16_t)(snapshot->distance_cm * 10.0f)
        : 0xFFFFU;

    uint8_t flags = 0;
    if (snapshot->sensor_valid) flags |= (1U << 0);
    if (snapshot->distance_valid) flags |= (1U << 1);
    if (snapshot->obstacle) flags |= (1U << 2);
    if (snapshot->limit_open) flags |= (1U << 3);
    if (snapshot->limit_closed) flags |= (1U << 4);
    if (snapshot->position_contradictory) flags |= (1U << 5);
    if (control_task_auto_enabled()) flags |= (1U << 6);

    response->type = MSG_RESPONSE;
    response->length = 12;
    response->payload[0] = (uint8_t)state_machine_get_state();
    write_i16_le(&response->payload[1], temperature_x100);
    write_u16_le(&response->payload[3], humidity_x100);
    write_u32_le(&response->payload[5], pressure_pa);
    write_u16_le(&response->payload[9], distance_mm);
    response->payload[11] = flags;
}

void protocol_handle_frame(const protocol_frame_t *frame)
{
    if (frame->check != protocol_compute_check(frame))
    {
        ESP_LOGW(TAG, "Invalid CHECK");
        protocol_send_error(1);
        return;
    }

    if (frame->version != PROTOCOL_VERSION)
    {
        ESP_LOGW(TAG, "Unsupported version");
        protocol_send_error(2);
        return;
    }

    protocol_frame_t response = {0};
    response.sof = PROTOCOL_SOF;
    response.version = PROTOCOL_VERSION;

    switch (frame->type)
    {
        case MSG_HELLO:
            response.type = MSG_RESPONSE;
            response.length = 0;
            protocol_send_frame(&response);
            break;

        case MSG_GET_STATUS:
            response.type = MSG_RESPONSE;
            response.length = 1;
            response.payload[0] = (uint8_t)state_machine_get_state();
            protocol_send_frame(&response);
            break;

        case MSG_SET_MODE:
        {
            if (frame->length != 1)
            {
                protocol_send_error(3);
                break;
            }

            user_command_t command;
            if (frame->payload[0] == 0)
            {
                command = USER_COMMAND_CLOSE;
            }
            else if (frame->payload[0] == 1)
            {
                command = USER_COMMAND_OPEN;
            }
            else
            {
                protocol_send_error(3);
                break;
            }

            if (!queue_user_command(command))
            {
                break;
            }

            response.type = MSG_RESPONSE;
            response.length = 1;
            response.payload[0] = frame->payload[0];
            protocol_send_frame(&response);
            break;
        }

        case MSG_GET_TELEMETRY:
        {
            sensor_snapshot_t snapshot = {0};
            if (!system_snapshot_get(&snapshot) || !snapshot.sensor_valid)
            {
                protocol_send_error(5);
                break;
            }

            fill_telemetry_payload(&response, &snapshot);
            protocol_send_frame(&response);
            break;
        }

        case MSG_GET_VERSION:
            response.type = MSG_RESPONSE;
            response.length = 3;
            response.payload[0] = DUT_VERSION_MAJOR;
            response.payload[1] = DUT_VERSION_MINOR;
            response.payload[2] = DUT_VERSION_PATCH;
            protocol_send_frame(&response);
            break;

        case MSG_SET_AUTO:
        {
            if (frame->length != 1 || frame->payload[0] > 1)
            {
                protocol_send_error(3);
                break;
            }

            user_command_t command = frame->payload[0]
                ? USER_COMMAND_AUTO_ON
                : USER_COMMAND_AUTO_OFF;

            if (!queue_user_command(command))
            {
                break;
            }

            response.type = MSG_RESPONSE;
            response.length = 1;
            response.payload[0] = frame->payload[0];
            protocol_send_frame(&response);
            break;
        }

        case MSG_GET_DIAGNOSTICS:
        {
            sensor_snapshot_t snapshot = {0};
            if (!system_snapshot_get(&snapshot))
            {
                protocol_send_error(8);
                break;
            }

            fill_diagnostics_payload(&response, &snapshot);
            protocol_send_frame(&response);
            break;
        }

        default:
            ESP_LOGW(TAG, "Unknown message type: 0x%02X", frame->type);
            protocol_send_error(4);
            break;
    }
}
