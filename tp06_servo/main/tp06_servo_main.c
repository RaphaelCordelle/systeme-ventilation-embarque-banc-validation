#include <stdio.h>
#include <stdbool.h>

#include "driver/gpio.h"
#include "driver/ledc.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SERVO_GPIO 13
#define CLOSED_GPIO 18
#define OPEN_GPIO 19

#define SERVO_OPEN_PULSE_MS  2.0f
#define SERVO_CLOSE_PULSE_MS 1.0f

#define MOVEMENT_TIMEOUT_MS 3000

void servo_set_pulse(float pulse_ms)
{
    uint32_t duty =
        (uint32_t)((pulse_ms / 20.0f) * 65535.0f);

    ledc_set_duty(
        LEDC_LOW_SPEED_MODE,
        LEDC_CHANNEL_0,
        duty
    );

    ledc_update_duty(
        LEDC_LOW_SPEED_MODE,
        LEDC_CHANNEL_0
    );
}

void servo_stop(void)
{
    ledc_stop(
        LEDC_LOW_SPEED_MODE,
        LEDC_CHANNEL_0,
        0
    );

    printf("PWM SERVO COUPE\n");
}

bool switch_pressed(int gpio)
{
    return gpio_get_level(gpio) == 0;
}

bool switch_pressed_debounced(int gpio)
{
    int first_read = gpio_get_level(gpio);

    vTaskDelay(pdMS_TO_TICKS(20));

    int second_read = gpio_get_level(gpio);

    if (first_read == second_read)
    {
        return second_read == 0;
    }

    return false;
}

bool limits_contradictory(void)
{
    bool closed = switch_pressed(CLOSED_GPIO);
    bool open = switch_pressed(OPEN_GPIO);

    return closed && open;
}

bool servo_open(void)
{
    printf("\n--- OUVERTURE ---\n");

    if (limits_contradictory())
    {
        printf("ERREUR : fins de course contradictoires\n");
        servo_stop();
        return false;
    }

    servo_set_pulse(SERVO_OPEN_PULSE_MS);

    TickType_t start = xTaskGetTickCount();

    while (1)
    {
        if (limits_contradictory())
        {
            printf("ERREUR : fins de course contradictoires\n");
            servo_stop();
            return false;
        }

        if (switch_pressed_debounced(OPEN_GPIO))
        {
            printf("OPEN detecte\n");
            servo_stop();
            return true;
        }

        if ((xTaskGetTickCount() - start) >
            pdMS_TO_TICKS(MOVEMENT_TIMEOUT_MS))
        {
            printf("ERREUR : TIMEOUT OUVERTURE\n");
            servo_stop();
            return false;
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

bool servo_close(void)
{
    printf("\n--- FERMETURE ---\n");

    if (limits_contradictory())
    {
        printf("ERREUR : fins de course contradictoires\n");
        servo_stop();
        return false;
    }

    servo_set_pulse(SERVO_CLOSE_PULSE_MS);

    TickType_t start = xTaskGetTickCount();

    while (1)
    {
        if (limits_contradictory())
        {
            printf("ERREUR : fins de course contradictoires\n");
            servo_stop();
            return false;
        }

        if (switch_pressed_debounced(CLOSED_GPIO))
        {
            printf("CLOSED detecte\n");
            servo_stop();
            return true;
        }

        if ((xTaskGetTickCount() - start) >
            pdMS_TO_TICKS(MOVEMENT_TIMEOUT_MS))
        {
            printf("ERREUR : TIMEOUT FERMETURE\n");
            servo_stop();
            return false;
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

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

    gpio_config_t limit_config = {
        .pin_bit_mask =
            (1ULL << CLOSED_GPIO) |
            (1ULL << OPEN_GPIO),

        .mode = GPIO_MODE_INPUT,

        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,

        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&limit_config);

    printf("TP06 PRET\n");

    while (1)
    {
        servo_open();

        vTaskDelay(pdMS_TO_TICKS(2000));

        servo_close();

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}