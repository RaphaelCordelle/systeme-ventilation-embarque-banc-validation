#include "hc_sr04.h"

#include "app_config.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"

static void trigger_sensor(void)
{
    gpio_set_level(HC_SR04_TRIG_GPIO, 0);
    esp_rom_delay_us(2);
    gpio_set_level(HC_SR04_TRIG_GPIO, 1);
    esp_rom_delay_us(10);
    gpio_set_level(HC_SR04_TRIG_GPIO, 0);
}

void hc_sr04_init(void)
{
    gpio_config_t trig_config = {
        .pin_bit_mask = (1ULL << HC_SR04_TRIG_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&trig_config);

    gpio_config_t echo_config = {
        .pin_bit_mask = (1ULL << HC_SR04_ECHO_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&echo_config);

    gpio_set_level(HC_SR04_TRIG_GPIO, 0);
}

DistanceMeasurement hc_sr04_measure(void)
{
    DistanceMeasurement measurement = {
        .distance_cm = 0.0f,
        .valid = false,
        .timestamp_us = esp_timer_get_time()
    };

    trigger_sensor();

    int64_t wait_start = esp_timer_get_time();
    while (gpio_get_level(HC_SR04_ECHO_GPIO) == 0)
    {
        if (esp_timer_get_time() - wait_start > HC_SR04_TIMEOUT_US)
        {
            return measurement;
        }
    }

    int64_t echo_start = esp_timer_get_time();
    while (gpio_get_level(HC_SR04_ECHO_GPIO) == 1)
    {
        if (esp_timer_get_time() - echo_start > HC_SR04_TIMEOUT_US)
        {
            return measurement;
        }
    }

    int64_t echo_end = esp_timer_get_time();
    float distance_cm = (float)(echo_end - echo_start) * 0.0343f / 2.0f;

    measurement.distance_cm = distance_cm;
    measurement.timestamp_us = esp_timer_get_time();
    measurement.valid =
        distance_cm >= HC_SR04_MIN_CM &&
        distance_cm <= HC_SR04_MAX_CM;

    return measurement;
}
