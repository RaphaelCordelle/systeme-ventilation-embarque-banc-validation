#ifndef ACTUATOR_TASK_H
#define ACTUATOR_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

void actuator_task_start(
    QueueHandle_t actuator_queue
);

#endif
