#include "sensor_task.h"

#include "app_types.h"

#include "telemetry_source.h"
#include "hc_sr04.h"
#include "endstops.h"

#include "esp_log.h"
#include "esp_timer.h"

#include "freertos/task.h"
#define OBSTACLE_THRESHOLD_CM 15.0f

static const char *TAG =
    "SENSOR_TASK";

static QueueHandle_t queue_sensor;


static sensor_snapshot_t sensor_read_snapshot(void)
{
    sensor_snapshot_t snapshot = {0};

    dut_telemetry_t telemetry = {0};

    if (telemetry_source_read(&telemetry) &&
        telemetry.valid)
    {
        snapshot.temperature =
            telemetry.temperature;

        snapshot.humidity =
            telemetry.humidity;

        snapshot.sensor_valid =
            true;
    }
    else
    {
        snapshot.sensor_valid =
            false;
    }


    DistanceMeasurement distance =
        hc_sr04_measure();

    if (distance.valid)
    {
        snapshot.distance_cm =
            distance.distance_cm;
    }
    else
    {
        snapshot.distance_cm =
            0.0f;
    }


    PositionFeedback position =
        endstops_get_position();

    snapshot.limit_open =
        position == POSITION_OPEN;

    snapshot.limit_closed =
        position == POSITION_CLOSED;

    snapshot.position_contradictory =
        position == POSITION_CONTRADICTORY;

    if (distance.valid && distance.distance_cm <= OBSTACLE_THRESHOLD_CM)
        {
            snapshot.obstacle = true;
        }
        else
        {
            snapshot.obstacle = false;
        }


    snapshot.timestamp_ms =
        (uint32_t)(
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

        ESP_LOGI(
            TAG,
            "T=%.2f H=%.2f D=%.2f OPEN=%d CLOSED=%d CONTRADICTION=%d",
            snapshot.temperature,
            snapshot.humidity,
            snapshot.distance_cm,
            snapshot.limit_open,
            snapshot.limit_closed,
            snapshot.position_contradictory
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