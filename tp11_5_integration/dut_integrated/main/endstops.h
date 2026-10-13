#pragma once

#include "driver/gpio.h"

typedef enum
{
    POSITION_OPEN,
    POSITION_CLOSED,
    POSITION_BETWEEN,
    POSITION_CONTRADICTORY
} PositionFeedback;

void endstops_init(void);

int endstops_read_debounced(gpio_num_t gpio);

PositionFeedback endstops_get_position(void);