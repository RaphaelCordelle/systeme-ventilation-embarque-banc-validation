#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

#include "esp_log.h"


#define CLOSED_LIMIT GPIO_NUM_18
#define OPEN_LIMIT   GPIO_NUM_19

#define RED_LED      GPIO_NUM_25
#define YELLOW_LED   GPIO_NUM_26
#define GREEN_LED    GPIO_NUM_27

#define BUZZER_GPIO  GPIO_NUM_23

static const char *TAG = "TP04";


typedef enum
{
    POSITION_OPEN,
    POSITION_CLOSED,
    POSITION_BETWEEN,
    POSITION_CONTRADICTORY
} PositionFeedback;


int read_debounced_gpio(gpio_num_t gpio)
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


PositionFeedback get_position(int closed_state, int open_state)
{
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


void buzzer_on(void)
{
    ledc_set_duty(
        LEDC_LOW_SPEED_MODE,
        LEDC_CHANNEL_0,
        512
    );

    ledc_update_duty(
        LEDC_LOW_SPEED_MODE,
        LEDC_CHANNEL_0
    );
}


void buzzer_off(void)
{
    ledc_set_duty(
        LEDC_LOW_SPEED_MODE,
        LEDC_CHANNEL_0,
        0
    );

    ledc_update_duty(
        LEDC_LOW_SPEED_MODE,
        LEDC_CHANNEL_0
    );
}


void app_main(void)
{
    // -------------------------
    // FINS DE COURSE
    // -------------------------

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


    // -------------------------
    // LEDs
    // -------------------------

    gpio_config_t led_config = {

        .pin_bit_mask =
            (1ULL << RED_LED) |
            (1ULL << YELLOW_LED) |
            (1ULL << GREEN_LED),

        .mode = GPIO_MODE_OUTPUT,

        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,

        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&led_config);


    // -------------------------
    // TIMER PWM DU BUZZER
    // -------------------------

    ledc_timer_config_t timer_config = {

        .speed_mode = LEDC_LOW_SPEED_MODE,

        .duty_resolution = LEDC_TIMER_10_BIT,

        .timer_num = LEDC_TIMER_0,

        .freq_hz = 2000,

        .clk_cfg = LEDC_AUTO_CLK
    };

    ledc_timer_config(&timer_config);


    // -------------------------
    // CANAL PWM DU BUZZER
    // -------------------------

    ledc_channel_config_t channel_config = {

        .gpio_num = BUZZER_GPIO,

        .speed_mode = LEDC_LOW_SPEED_MODE,

        .channel = LEDC_CHANNEL_0,

        .timer_sel = LEDC_TIMER_0,

        .duty = 0,

        .hpoint = 0
    };

    ledc_channel_config(&channel_config);


    // Tout éteint au démarrage
    gpio_set_level(RED_LED, 0);
    gpio_set_level(YELLOW_LED, 0);
    gpio_set_level(GREEN_LED, 0);

    buzzer_off();


    while (1)
    {
        int closed_state =
            read_debounced_gpio(CLOSED_LIMIT);

        int open_state =
            read_debounced_gpio(OPEN_LIMIT);


        if (closed_state == -1 || open_state == -1)
        {
            ESP_LOGW(TAG, "Etat instable detecte");

            vTaskDelay(pdMS_TO_TICKS(10));

            continue;
        }


        PositionFeedback position =
            get_position(closed_state, open_state);


        // On éteint les LEDs
        gpio_set_level(RED_LED, 0);
        gpio_set_level(YELLOW_LED, 0);
        gpio_set_level(GREEN_LED, 0);


        if (position == POSITION_CLOSED)
        {
            ESP_LOGI(TAG, "Position : CLOSED");

            gpio_set_level(RED_LED, 1);

            buzzer_off();
        }


        else if (position == POSITION_OPEN)
        {
            ESP_LOGI(TAG, "Position : OPEN");

            gpio_set_level(GREEN_LED, 1);

            buzzer_off();
        }


        else if (position == POSITION_BETWEEN)
        {
            ESP_LOGI(TAG, "Position : BETWEEN");

            gpio_set_level(YELLOW_LED, 1);

            buzzer_off();
        }


        else
        {
            ESP_LOGE(
                TAG,
                "ANOMALIE : CONTRADICTORY"
            );

            gpio_set_level(RED_LED, 1);
            gpio_set_level(YELLOW_LED, 1);

            buzzer_on();
        }


        vTaskDelay(pdMS_TO_TICKS(500));
    }
}