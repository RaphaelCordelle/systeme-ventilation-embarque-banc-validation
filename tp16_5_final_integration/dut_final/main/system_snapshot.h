#ifndef SYSTEM_SNAPSHOT_H
#define SYSTEM_SNAPSHOT_H

#include <stdbool.h>
#include "app_types.h"

bool system_snapshot_init(void);
void system_snapshot_update(const sensor_snapshot_t *snapshot);
bool system_snapshot_get(sensor_snapshot_t *snapshot);

#endif
