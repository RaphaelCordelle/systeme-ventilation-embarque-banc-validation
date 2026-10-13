#include "endstops.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define CLOSED_LIMIT GPIO_NUM_18
#define OPEN_LIMIT   GPIO_NUM_19

void endstops_init(void)
{
    gpio_config_t limit_config = {
        .pin_bit_mask =
            (1ULL << CLOSED_LIMIT) |
            (1ULL << OPEN_LIMIT),

        .mode = GPIO_MODE_INPUT,

        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,

        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&limit_config);
}

int endstops_read_debounced(gpio_num_t gpio)
{
    int first_state = gpio_get_level(gpio);

    vTaskDelay(pdMS_TO_TICKS(20));

    int second_state = gpio_get_level(gpio);

    if (first_state == second_state)
    {
        return first_state;
    }

    return -1;
}

PositionFeedback endstops_get_position(void)
{
    int closed_state =
        endstops_read_debounced(CLOSED_LIMIT);

    int open_state =
        endstops_read_debounced(OPEN_LIMIT);

    if (closed_state == -1 || open_state == -1)
    {
        return POSITION_CONTRADICTORY;
    }

    if (closed_state == 0 && open_state == 1)
    {
        return POSITION_CLOSED;
    }

    if (closed_state == 1 && open_state == 0)
    {
        return POSITION_OPEN;
    }

    if (closed_state == 1 && open_state == 1)
    {
        return POSITION_BETWEEN;
    }

    return POSITION_CONTRADICTORY;
}