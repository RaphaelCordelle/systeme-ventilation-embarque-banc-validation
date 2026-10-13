#include "endstops.h"

#include "app_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void endstops_init(void)
{
    gpio_config_t limit_config = {
        .pin_bit_mask =
            (1ULL << CLOSED_LIMIT_GPIO) |
            (1ULL << OPEN_LIMIT_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&limit_config);
}

int endstops_read_debounced(gpio_num_t gpio)
{
    /* 3-sample majority filter: a single bounce cannot flip the logical state. */
    int high_votes = 0;

    for (int i = 0; i < 3; i++)
    {
        high_votes += gpio_get_level(gpio) ? 1 : 0;

        if (i < 2)
        {
            vTaskDelay(pdMS_TO_TICKS(ENDSTOP_DEBOUNCE_MS));
        }
    }

    return high_votes >= 2 ? 1 : 0;
}

PositionFeedback endstops_get_position(void)
{
    int closed_state = endstops_read_debounced(CLOSED_LIMIT_GPIO);
    int open_state = endstops_read_debounced(OPEN_LIMIT_GPIO);

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

    /* Both active at once is physically contradictory. */
    return POSITION_CONTRADICTORY;
}
