#include <stdbool.h>
#include <stdint.h>

#include "esp_log.h"
#include "esp_err.h"
#include "esp_task_wdt.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "app_types.h"

#include "state_machine.h"
#include "endstops.h"
#include "hc_sr04.h"
#include "servo.h"

#include "sensor_task.h"
#include "control_task.h"
#include "actuator_task.h"

#include "uart_link.h"
#include "parser.h"
#include "protocol.h"


static const char *TAG = "DUT_INTEGRATED";


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
        esp_task_wdt_reconfigure(
            &config
        );

    if (result == ESP_ERR_INVALID_STATE)
    {
        result =
            esp_task_wdt_init(
                &config
            );
    }

    if (result != ESP_OK)
    {
        ESP_LOGW(
            TAG,
            "Watchdog configuration failed: %s",
            esp_err_to_name(result)
        );
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
        int received =
            uart_link_read(
                buffer,
                sizeof(buffer)
            );

        if (received > 0)
        {
            for (int i = 0; i < received; i++)
            {
                bool complete =
                    parser_process_byte(
                        &parser,
                        buffer[i],
                        &frame
                    );

                if (complete)
                {
                    protocol_handle_frame(
                        &frame
                    );
                }
            }
        }

        parser_check_timeout(
            &parser
        );

        vTaskDelay(
            pdMS_TO_TICKS(10)
        );
    }
}


void app_main(void)
{
    ESP_LOGI(
        TAG,
        "DUT integrated starting"
    );


    /*
     * Initialisation des drivers materiels.
     */

    endstops_init();

    hc_sr04_init();

    servo_init();


    /*
     * Initialisation de la machine a etats.
     */

    state_machine_init();


    /*
     * Creation des queues FreeRTOS.
     */

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


    /*
     * Initialisation UART BENCH <-> DUT.
     */

    uart_link_init();


    /*
     * Donne la command_queue au protocole.
     */

    protocol_set_command_queue(
        command_queue
    );


    /*
     * Initialisation / reconfiguration du watchdog.
     */

    init_watchdog();


    /*
     * Demarrage des taches principales.
     */

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


    /*
     * Demarrage de la tache UART.
     */

    BaseType_t uart_task_created =
        xTaskCreate(
            uart_task,
            "UartTask",
            4096,
            NULL,
            5,
            NULL
        );

    if (uart_task_created != pdPASS)
    {
        ESP_LOGE(
            TAG,
            "UART task creation failed"
        );

        return;
    }


    ESP_LOGI(
        TAG,
        "DUT integrated ready"
    );


    while (1)
    {
        vTaskDelay(
            pdMS_TO_TICKS(1000)
        );
    }
}