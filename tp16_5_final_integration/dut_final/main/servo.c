#include "servo.h"

#include <stdint.h>

#include "app_config.h"
#include "driver/ledc.h"

static void servo_set_pulse(float pulse_ms)
{
    uint32_t duty = (uint32_t)((pulse_ms / 20.0f) * 65535.0f);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, SERVO_CHANNEL, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, SERVO_CHANNEL);
}

void servo_init(void)
{
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_16_BIT,
        .timer_num = SERVO_TIMER,
        .freq_hz = 50,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_config);

    ledc_channel_config_t channel_config = {
        .gpio_num = SERVO_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = SERVO_CHANNEL,
        .timer_sel = SERVO_TIMER,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&channel_config);
}

void servo_open(void)
{
    servo_set_pulse(SERVO_OPEN_PULSE_MS);
}

void servo_close(void)
{
    servo_set_pulse(SERVO_CLOSE_PULSE_MS);
}

void servo_stop(void)
{
    ledc_stop(LEDC_LOW_SPEED_MODE, SERVO_CHANNEL, 0);
}
