#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_task_wdt.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "actuator_task.h"
#include "app_config.h"
#include "app_types.h"
#include "boot_info.h"
#include "control_task.h"
#include "dut_version.h"
#include "endstops.h"
#include "hc_sr04.h"
#include "indicators.h"
#include "parser.h"
#include "protocol.h"
#include "sensor_task.h"
#include "servo.h"
#include "state_machine.h"
#include "system_snapshot.h"
#include "uart_link.h"

static const char *TAG = "DUT_FINAL";

static QueueHandle_t sensor_queue;
static QueueHandle_t actuator_queue;
static QueueHandle_t command_queue;

static void init_watchdog(void)
{
    esp_task_wdt_config_t config = {
        .timeout_ms = 5000,
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
        .trigger_panic = true
    };

    esp_err_t result = esp_task_wdt_reconfigure(&config);
    if (result == ESP_ERR_INVALID_STATE)
    {
        result = esp_task_wdt_init(&config);
    }

    if (result != ESP_OK)
    {
        ESP_LOGW(TAG, "Watchdog configuration failed: %s", esp_err_to_name(result));
    }
}

static void uart_task(void *parameter)
{
    (void)parameter;

    parser_t parser;
    parser_init(&parser);

    uint8_t buffer[64];
    protocol_frame_t frame;

    while (1)
    {
        int received = uart_link_read(buffer, sizeof(buffer));

        if (received > 0)
        {
            for (int i = 0; i < received; i++)
            {
                if (parser_process_byte(&parser, buffer[i], &frame))
                {
                    protocol_handle_frame(&frame);
                }
            }
        }

        parser_check_timeout(&parser);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void)
{
    indicators_init();
    indicators_apply(STATE_UNKNOWN, false);

    uint32_t boot_count = boot_info_init_and_increment();

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "%s firmware v%s", DUT_NAME, DUT_VERSION_STRING);
    ESP_LOGI(TAG, "Boot count: %" PRIu32, boot_count);
    ESP_LOGI(
        TAG,
        "Pins | BME SDA=%d SCL=%d | HC TRIG=%d ECHO=%d | SERVO=%d | LIMITS C=%d O=%d",
        DUT_BME_SDA_GPIO,
        DUT_BME_SCL_GPIO,
        HC_SR04_TRIG_GPIO,
        HC_SR04_ECHO_GPIO,
        SERVO_GPIO,
        CLOSED_LIMIT_GPIO,
        OPEN_LIMIT_GPIO
    );
    ESP_LOGI(
        TAG,
        "AUTO | open T>=%.1fC or H>=%.1f%% | close T<=%.1fC and H<=%.1f%%",
        AUTO_OPEN_TEMP_C,
        AUTO_OPEN_HUMIDITY_PCT,
        AUTO_CLOSE_TEMP_C,
        AUTO_CLOSE_HUMIDITY_PCT
    );
    ESP_LOGI(TAG, "========================================");

    endstops_init();
    hc_sr04_init();
    servo_init();
    state_machine_init();

    if (!system_snapshot_init())
    {
        ESP_LOGE(TAG, "System snapshot mutex creation failed");
        indicators_apply(STATE_SAFE, true);
        return;
    }

    sensor_queue = xQueueCreate(1, sizeof(sensor_snapshot_t));
    actuator_queue = xQueueCreate(1, sizeof(actuator_command_t));
    command_queue = xQueueCreate(4, sizeof(user_command_t));

    if (sensor_queue == NULL || actuator_queue == NULL || command_queue == NULL)
    {
        ESP_LOGE(TAG, "Queue creation failed");
        indicators_apply(STATE_SAFE, true);
        return;
    }

    uart_link_init();
    protocol_set_command_queue(command_queue);
    init_watchdog();

    sensor_task_start(sensor_queue);
    actuator_task_start(actuator_queue);
    control_task_start(sensor_queue, actuator_queue, command_queue);

    if (
        xTaskCreate(
            uart_task,
            "UartTask",
            4096,
            NULL,
            5,
            NULL
        ) != pdPASS
    )
    {
        ESP_LOGE(TAG, "UART task creation failed");
        indicators_apply(STATE_SAFE, true);
        return;
    }

    ESP_LOGI(TAG, "DUT ready - waiting for position feedback / automatic control");

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
