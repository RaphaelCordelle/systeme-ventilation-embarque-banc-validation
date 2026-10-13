#include "actuator_task.h"

#include "app_types.h"
#include "servo.h"

#include "esp_log.h"

#include "freertos/task.h"

static const char *TAG = "ACTUATOR_TASK";

static QueueHandle_t queue_actuator;


static void actuator_task(void *parameter)
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

                    ESP_LOGI(
                        TAG,
                        "OPEN"
                    );

                    servo_open();

                    break;


                case ACTUATOR_CLOSE:

                    ESP_LOGI(
                        TAG,
                        "CLOSE"
                    );

                    servo_close();

                    break;


                case ACTUATOR_STOP:

                default:

                    ESP_LOGI(
                        TAG,
                        "STOP"
                    );

                    servo_stop();

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