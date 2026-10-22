#include "actuator_task.h"

#include "app_types.h"

#include "esp_log.h"

#include "freertos/task.h"

static const char *TAG =
    "ACTUATOR_TASK";


static QueueHandle_t queue_actuator;


static void actuator_stop(void)
{
    ESP_LOGI(
        TAG,
        "STOP"
    );

    /* TP08 : arret simule, sans commande PWM. */
}


static void actuator_open(void)
{
    ESP_LOGI(
        TAG,
        "OPEN"
    );

    /* TP08 : ouverture simulee, observee dans les logs. */
}


static void actuator_close(void)
{
    ESP_LOGI(
        TAG,
        "CLOSE"
    );

    /* TP08 : fermeture simulee, observee dans les logs. */
}


static void actuator_task(
    void *parameter
)
{
    actuator_command_t command;


    while (1)
    {
        if (
            xQueueReceive(
                queue_actuator,
                &command,
                portMAX_DELAY
            )
            == pdTRUE
        )
        {
            switch (command)
            {
                case ACTUATOR_OPEN:

                    actuator_open();

                    break;


                case ACTUATOR_CLOSE:

                    actuator_close();

                    break;


                case ACTUATOR_STOP:

                default:

                    actuator_stop();

                    break;
            }
        }
    }
}


void actuator_task_start(
    QueueHandle_t actuator_queue
)
{
    queue_actuator =
        actuator_queue;


    xTaskCreate(
        actuator_task,
        "ActuatorTask",
        4096,
        NULL,
        5,
        NULL
    );
}
