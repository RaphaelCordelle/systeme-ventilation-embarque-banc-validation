#include <stdio.h>

#include "driver/gpio.h"
#include "driver/ledc.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SERVO_GPIO 13


void app_main(void)
{
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_16_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 50,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ledc_timer_config(&timer_config);

    ledc_channel_config_t channel_config = {
        .gpio_num = SERVO_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };

    ledc_channel_config(&channel_config);

    // Impulsion d'environ 1,5 ms sur une période de 20 ms, 65535 est la valeur max sur 16 bits
    uint32_t duty = (uint32_t)((1.5f / 20.0f) * 65535.0f);

    ledc_set_duty(
        LEDC_LOW_SPEED_MODE,
        LEDC_CHANNEL_0,
        duty
    );

    ledc_update_duty(
        LEDC_LOW_SPEED_MODE,
        LEDC_CHANNEL_0
    );

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}