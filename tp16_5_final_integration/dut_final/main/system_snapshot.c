#include "system_snapshot.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static SemaphoreHandle_t snapshot_mutex = NULL;
static sensor_snapshot_t latest_snapshot;
static bool has_snapshot = false;

bool system_snapshot_init(void)
{
    snapshot_mutex = xSemaphoreCreateMutex();
    memset(&latest_snapshot, 0, sizeof(latest_snapshot));
    has_snapshot = false;
    return snapshot_mutex != NULL;
}

void system_snapshot_update(const sensor_snapshot_t *snapshot)
{
    if (snapshot == NULL || snapshot_mutex == NULL)
    {
        return;
    }

    if (xSemaphoreTake(snapshot_mutex, pdMS_TO_TICKS(20)) == pdTRUE)
    {
        latest_snapshot = *snapshot;
        has_snapshot = true;
        xSemaphoreGive(snapshot_mutex);
    }
}

bool system_snapshot_get(sensor_snapshot_t *snapshot)
{
    if (snapshot == NULL || snapshot_mutex == NULL)
    {
        return false;
    }

    bool result = false;

    if (xSemaphoreTake(snapshot_mutex, pdMS_TO_TICKS(20)) == pdTRUE)
    {
        if (has_snapshot)
        {
            *snapshot = latest_snapshot;
            result = true;
        }

        xSemaphoreGive(snapshot_mutex);
    }

    return result;
}
