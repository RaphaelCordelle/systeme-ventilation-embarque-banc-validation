#include <stdio.h>

#include "driver/gpio.h"
#include "esp_rom_sys.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#define TRIG_GPIO GPIO_NUM_21

void app_main(void)
{
    gpio_config_t trig_config = {
        .pin_bit_mask = (1ULL << TRIG_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&trig_config);

    while (1)
    {
        gpio_set_level(TRIG_GPIO, 0);
        esp_rom_delay_us(2);

        gpio_set_level(TRIG_GPIO, 1);
        esp_rom_delay_us(10);

        gpio_set_level(TRIG_GPIO, 0);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}