#include "control_task.h"

#include "app_config.h"
#include "app_types.h"
#include "indicators.h"
#include "state_machine.h"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "CONTROL";

static QueueHandle_t queue_sensor;
static QueueHandle_t queue_actuator;
static QueueHandle_t queue_command;

static volatile bool auto_enabled = true;

/* Several consecutive BME errors are required before latching SAFE. */
#define SENSOR_FAULT_CONSECUTIVE_LIMIT 5

bool control_task_auto_enabled(void)
{
    return auto_enabled;
}

static void send_actuator_command(actuator_command_t command)
{
    xQueueOverwrite(queue_actuator, &command);
}

static void start_opening(bool *movement_active, TickType_t *movement_start)
{
    system_state_t before = state_machine_get_state();
    state_machine_handle_event(EVENT_CMD_OPEN);

    if (before != STATE_OPENING && state_machine_get_state() == STATE_OPENING)
    {
        ESP_LOGI(TAG, "ACTUATION OPEN");
        send_actuator_command(ACTUATOR_OPEN);
        *movement_start = xTaskGetTickCount();
        *movement_active = true;
    }
}

static void start_closing(bool *movement_active, TickType_t *movement_start)
{
    system_state_t before = state_machine_get_state();
    state_machine_handle_event(EVENT_CMD_CLOSE);

    if (before != STATE_CLOSING && state_machine_get_state() == STATE_CLOSING)
    {
        ESP_LOGI(TAG, "ACTUATION CLOSE");
        send_actuator_command(ACTUATOR_CLOSE);
        *movement_start = xTaskGetTickCount();
        *movement_active = true;
    }
}

static void enter_safe(system_event_t reason, bool *movement_active)
{
    if (state_machine_get_state() != STATE_SAFE)
    {
        ESP_LOGE(TAG, "SAFE reason=%s", state_machine_event_to_string(reason));
        state_machine_handle_event(reason);
    }

    send_actuator_command(ACTUATOR_STOP);
    *movement_active = false;
}

static void process_manual_command(
    user_command_t command,
    bool *movement_active,
    TickType_t *movement_start
)
{
    switch (command)
    {
        case USER_COMMAND_AUTO_ON:
            auto_enabled = true;
            ESP_LOGI(TAG, "CONTROL_MODE=AUTO");
            break;

        case USER_COMMAND_AUTO_OFF:
            auto_enabled = false;
            ESP_LOGI(TAG, "CONTROL_MODE=MANUAL");
            break;

        case USER_COMMAND_OPEN:
            /* A manual actuator order intentionally overrides automatic logic. */
            auto_enabled = false;
            ESP_LOGI(TAG, "MANUAL OPEN -> AUTO disabled");
            start_opening(movement_active, movement_start);
            break;

        case USER_COMMAND_CLOSE:
            auto_enabled = false;
            ESP_LOGI(TAG, "MANUAL CLOSE -> AUTO disabled");
            start_closing(movement_active, movement_start);
            break;

        case USER_COMMAND_NONE:
        default:
            break;
    }
}

static void process_auto_control(
    const sensor_snapshot_t *snapshot,
    bool *movement_active,
    TickType_t *movement_start
)
{
    if (!auto_enabled || !snapshot->sensor_valid)
    {
        return;
    }

    system_state_t state = state_machine_get_state();

    bool open_request =
        snapshot->temperature >= AUTO_OPEN_TEMP_C ||
        snapshot->humidity >= AUTO_OPEN_HUMIDITY_PCT;

    bool close_request =
        snapshot->temperature <= AUTO_CLOSE_TEMP_C &&
        snapshot->humidity <= AUTO_CLOSE_HUMIDITY_PCT;

    if (state == STATE_CLOSED && open_request)
    {
        ESP_LOGI(
            TAG,
            "AUTO OPEN | T=%.2fC H=%.2f%% thresholds[%.1fC/%.1f%%]",
            snapshot->temperature,
            snapshot->humidity,
            AUTO_OPEN_TEMP_C,
            AUTO_OPEN_HUMIDITY_PCT
        );

        start_opening(movement_active, movement_start);
    }
    else if (state == STATE_OPEN && close_request)
    {
        ESP_LOGI(
            TAG,
            "AUTO CLOSE | T=%.2fC H=%.2f%% thresholds[%.1fC/%.1f%%]",
            snapshot->temperature,
            snapshot->humidity,
            AUTO_CLOSE_TEMP_C,
            AUTO_CLOSE_HUMIDITY_PCT
        );

        start_closing(movement_active, movement_start);
    }
}

static void control_task(void *parameter)
{
    (void)parameter;

    sensor_snapshot_t snapshot;
    user_command_t command;

    bool movement_active = false;
    TickType_t movement_start = 0;
    unsigned sensor_fault_count = 0;

    ESP_LOGI(
        TAG,
        "AUTO enabled | open if T>=%.1fC or H>=%.1f%% | close if T<=%.1fC and H<=%.1f%%",
        AUTO_OPEN_TEMP_C,
        AUTO_OPEN_HUMIDITY_PCT,
        AUTO_CLOSE_TEMP_C,
        AUTO_CLOSE_HUMIDITY_PCT
    );

    while (1)
    {
        if (xQueueReceive(queue_sensor, &snapshot, portMAX_DELAY) != pdTRUE)
        {
            continue;
        }

        system_state_t state = state_machine_get_state();

        if (!snapshot.sensor_valid)
        {
            if (sensor_fault_count < SENSOR_FAULT_CONSECUTIVE_LIMIT)
            {
                sensor_fault_count++;
            }
        }
        else
        {
            sensor_fault_count = 0;
        }

        bool sensor_fault = sensor_fault_count >= SENSOR_FAULT_CONSECUTIVE_LIMIT;

        if (sensor_fault)
        {
            enter_safe(EVENT_SENSOR_FAULT, &movement_active);
        }
        else if (snapshot.position_contradictory)
        {
            enter_safe(EVENT_CONTRADICTION, &movement_active);
        }
        else
        {
            state = state_machine_get_state();

            /* Position feedback only generates an event if it changes/finishes state. */
            if (snapshot.limit_open && state != STATE_OPEN)
            {
                if (state == STATE_OPENING || state == STATE_UNKNOWN)
                {
                    state_machine_handle_event(EVENT_LIMIT_OPEN);

                    if (state_machine_get_state() == STATE_OPEN)
                    {
                        send_actuator_command(ACTUATOR_STOP);
                        movement_active = false;
                        ESP_LOGI(TAG, "POSITION CONFIRMED OPEN");
                    }
                }
                else if (state == STATE_CLOSED)
                {
                    enter_safe(EVENT_LIMIT_OPEN, &movement_active);
                }
            }
            else if (snapshot.limit_closed && state != STATE_CLOSED)
            {
                if (state == STATE_CLOSING || state == STATE_UNKNOWN)
                {
                    state_machine_handle_event(EVENT_LIMIT_CLOSED);

                    if (state_machine_get_state() == STATE_CLOSED)
                    {
                        send_actuator_command(ACTUATOR_STOP);
                        movement_active = false;
                        ESP_LOGI(TAG, "POSITION CONFIRMED CLOSED");
                    }
                }
                else if (state == STATE_OPEN)
                {
                    enter_safe(EVENT_LIMIT_CLOSED, &movement_active);
                }
            }

            /* Obstacle safety: while closing, stop the closure and reopen. */
            if (
                snapshot.obstacle &&
                state_machine_get_state() == STATE_CLOSING
            )
            {
                ESP_LOGW(
                    TAG,
                    "OBSTACLE %.1fcm <= %.1fcm -> REOPEN",
                    snapshot.distance_cm,
                    OBSTACLE_THRESHOLD_CM
                );

                state_machine_handle_event(EVENT_OBSTACLE);
                send_actuator_command(ACTUATOR_OPEN);
                movement_start = xTaskGetTickCount();
                movement_active = true;
            }
        }

        /* Drain commands so a short burst from the bench does not leave stale orders. */
        while (xQueueReceive(queue_command, &command, 0) == pdTRUE)
        {
            if (state_machine_get_state() != STATE_SAFE)
            {
                process_manual_command(command, &movement_active, &movement_start);
            }
            else if (command == USER_COMMAND_AUTO_OFF)
            {
                auto_enabled = false;
            }
        }

        if (state_machine_get_state() != STATE_SAFE)
        {
            process_auto_control(&snapshot, &movement_active, &movement_start);
        }

        if (movement_active)
        {
            TickType_t elapsed = xTaskGetTickCount() - movement_start;

            if (elapsed >= pdMS_TO_TICKS(MOVEMENT_TIMEOUT_MS))
            {
                enter_safe(EVENT_TIMEOUT, &movement_active);
            }
        }

        indicators_apply(state_machine_get_state(), sensor_fault);
    }
}

void control_task_start(
    QueueHandle_t sensor_queue,
    QueueHandle_t actuator_queue,
    QueueHandle_t command_queue
)
{
    queue_sensor = sensor_queue;
    queue_actuator = actuator_queue;
    queue_command = command_queue;

    xTaskCreate(
        control_task,
        "ControlTask",
        4096,
        NULL,
        6,
        NULL
    );
}
