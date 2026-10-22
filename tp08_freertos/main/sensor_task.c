#include "sensor_task.h"

#include "app_types.h"

#include "esp_log.h"
#include "esp_timer.h"

#include "freertos/task.h"

static const char *TAG =
    "SENSOR_TASK";


static QueueHandle_t queue_sensor;


static sensor_snapshot_t sensor_read_snapshot(void)
{
    sensor_snapshot_t snapshot =
        {0};

    /* Snapshot factice pour tester les echanges entre taches du TP08. */

    snapshot.temperature = 0.0f;
    snapshot.humidity = 0.0f;
    snapshot.distance_cm = 0.0f;

    snapshot.sensor_valid = false;

    snapshot.obstacle = false;

    snapshot.limit_open = false;
    snapshot.limit_closed = false;

    snapshot.timestamp_ms =
        (uint32_t)
        (
            esp_timer_get_time()
            / 1000
        );

    return snapshot;
}


static void sensor_task(
    void *parameter
)
{
    while (1)
    {
        sensor_snapshot_t snapshot =
            sensor_read_snapshot();

        xQueueOverwrite(
            queue_sensor,
            &snapshot
        );

        ESP_LOGD(
            TAG,
            "Snapshot sent"
        );

        vTaskDelay(
            pdMS_TO_TICKS(100)
        );
    }
}


void sensor_task_start(
    QueueHandle_t sensor_queue
)
{
    queue_sensor =
        sensor_queue;

    xTaskCreate(
        sensor_task,
        "SensorTask",
        4096,
        NULL,
        5,
        NULL
    );
}
