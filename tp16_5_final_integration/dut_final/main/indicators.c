#include "indicators.h"

#include "app_config.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

static void set_leds(int red, int yellow, int green)
{
    gpio_set_level(RED_LED_GPIO, red);
    gpio_set_level(YELLOW_LED_GPIO, yellow);
    gpio_set_level(GREEN_LED_GPIO, green);
}

void indicators_buzzer_on(void)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL, 512);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL);
}

void indicators_buzzer_off(void)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL);
}

void indicators_init(void)
{
    gpio_config_t led_config = {
        .pin_bit_mask =
            (1ULL << RED_LED_GPIO) |
            (1ULL << YELLOW_LED_GPIO) |
            (1ULL << GREEN_LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&led_config);

    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = BUZZER_TIMER,
        .freq_hz = BUZZER_FREQUENCY_HZ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_config);

    ledc_channel_config_t channel_config = {
        .gpio_num = BUZZER_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = BUZZER_CHANNEL,
        .timer_sel = BUZZER_TIMER,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&channel_config);

    set_leds(0, 1, 0);
    indicators_buzzer_off();
}

void indicators_apply(system_state_t state, bool sensor_fault)
{
    if (sensor_fault || state == STATE_SAFE)
    {
        /* SAFE / defaut : rouge + jaune + buzzer. */
        set_leds(1, 1, 0);
        indicators_buzzer_on();
        return;
    }

    indicators_buzzer_off();

    switch (state)
    {
        case STATE_CLOSED:
            /* Semantique conservee du TP04 : rouge = volet ferme. */
            set_leds(1, 0, 0);
            break;

        case STATE_OPEN:
            /* Vert = volet ouvert. */
            set_leds(0, 0, 1);
            break;

        case STATE_OPENING:
        case STATE_CLOSING:
        case STATE_UNKNOWN:
        default:
            /* Jaune = mouvement / position intermediaire / initialisation. */
            set_leds(0, 1, 0);
            break;
    }
}
