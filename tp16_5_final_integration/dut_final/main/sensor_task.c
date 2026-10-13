#include "sensor_task.h"

#include "app_config.h"
#include "app_types.h"
#include "endstops.h"
#include "hc_sr04.h"
#include "system_snapshot.h"
#include "telemetry_source.h"

#include "esp_log.h"
#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SENSOR";
static QueueHandle_t queue_sensor;

static sensor_snapshot_t sensor_read_snapshot(void)
{
    sensor_snapshot_t snapshot = {0};
    dut_telemetry_t telemetry = {0};

    if (telemetry_source_read(&telemetry) && telemetry.valid)
    {
        snapshot.temperature = telemetry.temperature;
        snapshot.humidity = telemetry.humidity;
        snapshot.pressure_hpa = telemetry.pressure_hpa;
        snapshot.sensor_valid = true;
    }

    DistanceMeasurement distance = hc_sr04_measure();
    snapshot.distance_valid = distance.valid;
    snapshot.distance_cm = distance.valid ? distance.distance_cm : 0.0f;
    snapshot.obstacle =
        distance.valid &&
        distance.distance_cm <= OBSTACLE_THRESHOLD_CM;

    PositionFeedback position = endstops_get_position();
    snapshot.limit_open = position == POSITION_OPEN;
    snapshot.limit_closed = position == POSITION_CLOSED;
    snapshot.position_contradictory = position == POSITION_CONTRADICTORY;

    snapshot.timestamp_ms = (uint32_t)(esp_timer_get_time() / 1000);
    return snapshot;
}

static void log_snapshot(const sensor_snapshot_t *snapshot)
{
    if (!snapshot->sensor_valid)
    {
        ESP_LOGW(
            TAG,
            "SNAPSHOT BME=INVALID D=%s%.1fcm obstacle=%d limits[O=%d C=%d X=%d]",
            snapshot->distance_valid ? "" : "INVALID/",
            snapshot->distance_cm,
            snapshot->obstacle,
            snapshot->limit_open,
            snapshot->limit_closed,
            snapshot->position_contradictory
        );
        return;
    }

    ESP_LOGI(
        TAG,
        "SNAPSHOT T=%.2fC H=%.2f%% P=%.2fhPa D=%s%.1fcm obstacle=%d limits[O=%d C=%d X=%d]",
        snapshot->temperature,
        snapshot->humidity,
        snapshot->pressure_hpa,
        snapshot->distance_valid ? "" : "INVALID/",
        snapshot->distance_cm,
        snapshot->obstacle,
        snapshot->limit_open,
        snapshot->limit_closed,
        snapshot->position_contradictory
    );
}

static void sensor_task(void *parameter)
{
    (void)parameter;

    uint32_t last_log_ms = 0;

    while (1)
    {
        sensor_snapshot_t snapshot = sensor_read_snapshot();

        system_snapshot_update(&snapshot);
        xQueueOverwrite(queue_sensor, &snapshot);

        if (
            last_log_ms == 0 ||
            snapshot.timestamp_ms - last_log_ms >= SENSOR_LOG_PERIOD_MS
        )
        {
            log_snapshot(&snapshot);
            last_log_ms = snapshot.timestamp_ms;
        }

        vTaskDelay(pdMS_TO_TICKS(SENSOR_TASK_PERIOD_MS));
    }
}

void sensor_task_start(QueueHandle_t sensor_queue)
{
    queue_sensor = sensor_queue;

    xTaskCreate(
        sensor_task,
        "SensorTask",
        4096,
        NULL,
        5,
        NULL
    );
}
