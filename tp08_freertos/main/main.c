#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "esp_task_wdt.h"

#include "app_types.h"

#include "state_machine.h"

#include "sensor_task.h"
#include "control_task.h"
#include "actuator_task.h"


static const char *TAG =
    "TP08";


static QueueHandle_t sensor_queue;
static QueueHandle_t actuator_queue;
static QueueHandle_t command_queue;


static void init_watchdog(void)
{
    esp_task_wdt_config_t config = {
        .timeout_ms = 5000,
        .idle_core_mask =
            (1 << portNUM_PROCESSORS) - 1,
        .trigger_panic = true
    };

    esp_err_t result =
        esp_task_wdt_init(
            &config
        );

    if (
        result ==
        ESP_ERR_INVALID_STATE
    )
    {
        esp_task_wdt_reconfigure(
            &config
        );
    }
}


void app_main(void)
{
    ESP_LOGI(
        TAG,
        "TP08 FreeRTOS starting"
    );


    state_machine_init();


    sensor_queue =
        xQueueCreate(
            1,
            sizeof(sensor_snapshot_t)
        );


    actuator_queue =
        xQueueCreate(
            1,
            sizeof(actuator_command_t)
        );


    command_queue =
        xQueueCreate(
            4,
            sizeof(user_command_t)
        );


    if (
        sensor_queue == NULL ||
        actuator_queue == NULL ||
        command_queue == NULL
    )
    {
        ESP_LOGE(
            TAG,
            "Queue creation failed"
        );

        return;
    }


    init_watchdog();


    sensor_task_start(
        sensor_queue
    );


    actuator_task_start(
        actuator_queue
    );


    control_task_start(
        sensor_queue,
        actuator_queue,
        command_queue
    );


    ESP_LOGI(
        TAG,
        "DUT FreeRTOS ready"
    );


    while (1)
    {
        vTaskDelay(
            pdMS_TO_TICKS(1000)
        );
    }
}
