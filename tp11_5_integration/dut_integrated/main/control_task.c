#include "control_task.h"

#include "app_types.h"
#include "state_machine.h"

#include "esp_log.h"

#include "freertos/task.h"

static const char *TAG =
    "CONTROL_TASK";


static QueueHandle_t queue_sensor;
static QueueHandle_t queue_actuator;
static QueueHandle_t queue_command;


#define MOVEMENT_TIMEOUT_MS 3000


static void send_actuator_command(
    actuator_command_t command
)
{
    xQueueOverwrite(
        queue_actuator,
        &command
    );
}


static void control_task(
    void *parameter
)
{
    sensor_snapshot_t snapshot;

    user_command_t command;

    bool movement_active =
        false;

    TickType_t movement_start =
        0;


    while (1)
    {
        if (
            xQueueReceive(
                queue_sensor,
                &snapshot,
                portMAX_DELAY
            )
            == pdTRUE
        )
        {
            if (
                snapshot.position_contradictory
            )
            {
                ESP_LOGE(
                    TAG,
                    "End-stop contradiction detected"
                );

                state_machine_handle_event(
                    EVENT_CONTRADICTION
                );

                send_actuator_command(
                    ACTUATOR_STOP
                );

                movement_active =
                    false;
            }
            else if (
                snapshot.limit_open
            )
            {
                state_machine_handle_event(
                    EVENT_LIMIT_OPEN
                );

                send_actuator_command(
                    ACTUATOR_STOP
                );

                movement_active =
                    false;
            }
            else if (
                snapshot.limit_closed
            )
            {
                state_machine_handle_event(
                    EVENT_LIMIT_CLOSED
                );

                send_actuator_command(
                    ACTUATOR_STOP
                );

                movement_active =
                    false;
            }
            if (snapshot.obstacle)
            {
                send_actuator_command(
                    ACTUATOR_STOP
                );

                movement_active =
                    false;

                ESP_LOGW(
                    TAG,
                    "Obstacle detected"
                );
            }


            if (
                xQueueReceive(
                    queue_command,
                    &command,
                    0
                )
                == pdTRUE
            )
            {
                if (
                    command ==
                    USER_COMMAND_OPEN
                )
                {
                    state_machine_handle_event(
                        EVENT_CMD_OPEN
                    );

                    if (
                        state_machine_get_state() ==
                        STATE_OPENING
                    )
                    {
                        send_actuator_command(
                            ACTUATOR_OPEN
                        );

                        movement_start =
                            xTaskGetTickCount();

                        movement_active =
                            true;
                    }
                    else
                    {
                        send_actuator_command(
                            ACTUATOR_STOP
                        );

                        movement_active =
                            false;
                    }
                }


                else if (
                    command ==
                    USER_COMMAND_CLOSE
                )
                {
                    state_machine_handle_event(
                        EVENT_CMD_CLOSE
                    );

                    if (
                        state_machine_get_state() ==
                        STATE_CLOSING
                    )
                    {
                        send_actuator_command(
                            ACTUATOR_CLOSE
                        );

                        movement_start =
                            xTaskGetTickCount();

                        movement_active =
                            true;
                    }
                    else
                    {
                        send_actuator_command(
                            ACTUATOR_STOP
                        );

                        movement_active =
                            false;
                    }
                }
            }


            if (movement_active)
            {
                TickType_t now =
                    xTaskGetTickCount();

                TickType_t elapsed =
                    now -
                    movement_start;

                if (
                    elapsed >=
                    pdMS_TO_TICKS(
                        MOVEMENT_TIMEOUT_MS
                    )
                )
                {
                    state_machine_handle_event(
                        EVENT_TIMEOUT
                    );

                    send_actuator_command(
                        ACTUATOR_STOP
                    );

                    movement_active =
                        false;

                    ESP_LOGE(
                        TAG,
                        "Movement timeout"
                    );
                }
            }


            ESP_LOGI(
                TAG,
                "State: %s",
                state_machine_state_to_string(
                    state_machine_get_state()
                )
            );
        }
    }
}


void control_task_start(
    QueueHandle_t sensor_queue,
    QueueHandle_t actuator_queue,
    QueueHandle_t command_queue
)
{
    queue_sensor =
        sensor_queue;

    queue_actuator =
        actuator_queue;

    queue_command =
        command_queue;


    xTaskCreate(
        control_task,
        "ControlTask",
        4096,
        NULL,
        6,
        NULL
    );
}
