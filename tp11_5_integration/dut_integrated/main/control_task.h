#ifndef CONTROL_TASK_H
#define CONTROL_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

void control_task_start(
    QueueHandle_t sensor_queue,
    QueueHandle_t actuator_queue,
    QueueHandle_t command_queue
);

#endif
