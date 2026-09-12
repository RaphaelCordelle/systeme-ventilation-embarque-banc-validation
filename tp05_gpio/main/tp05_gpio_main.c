#include <stdio.h>
#include <stdbool.h>

#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TRIG_GPIO GPIO_NUM_21
#define ECHO_GPIO GPIO_NUM_22

#define TIMEOUT_US 30000

#define MIN_DISTANCE_CM 2.0f
#define MAX_DISTANCE_CM 400.0f

typedef struct
{
    float distance_cm;
    bool valid;
    int64_t timestamp_us;
} DistanceMeasurement;


// --------------------------------------------------
// Envoie l'impulsion TRIG au HC-SR04
// --------------------------------------------------

void trigger_sensor(void)
{
    gpio_set_level(TRIG_GPIO, 0);
    esp_rom_delay_us(2);

    gpio_set_level(TRIG_GPIO, 1);
    esp_rom_delay_us(10);

    gpio_set_level(TRIG_GPIO, 0);
}


// --------------------------------------------------
// Effectue une mesure complète
// --------------------------------------------------

DistanceMeasurement measure_distance(void)
{
    DistanceMeasurement measurement = {
        .distance_cm = 0.0f,
        .valid = false,
        .timestamp_us = esp_timer_get_time()
    };

    // Déclenche le HC-SR04
    trigger_sensor();

    // --------------------------------------
    // Attente du début de ECHO
    // --------------------------------------

    int64_t wait_start = esp_timer_get_time();

    while (gpio_get_level(ECHO_GPIO) == 0)
    {
        if (esp_timer_get_time() - wait_start > TIMEOUT_US)
        {
            printf("ERREUR : aucun ECHO recu\n");
            return measurement;
        }
    }

    // ECHO vient de passer à 1
    int64_t echo_start = esp_timer_get_time();

    // --------------------------------------
    // Attente de la fin de ECHO
    // --------------------------------------

    while (gpio_get_level(ECHO_GPIO) == 1)
    {
        if (esp_timer_get_time() - echo_start > TIMEOUT_US)
        {
            printf("ERREUR : ECHO bloque a 1\n");
            return measurement;
        }
    }

    // ECHO vient de repasser à 0
    int64_t echo_end = esp_timer_get_time();

    // --------------------------------------
    // Calcul de la durée
    // --------------------------------------

    int64_t echo_duration = echo_end - echo_start;

    // --------------------------------------
    // Conversion durée -> distance
    // --------------------------------------

    float distance_cm =
        echo_duration * 0.0343f / 2.0f;

    measurement.distance_cm = distance_cm;
    measurement.timestamp_us = esp_timer_get_time();

    // --------------------------------------
    // Validation de la mesure
    // --------------------------------------

    if (distance_cm >= MIN_DISTANCE_CM &&
        distance_cm <= MAX_DISTANCE_CM)
    {
        measurement.valid = true;
    }

    return measurement;
}


// --------------------------------------------------
// Programme principal
// --------------------------------------------------

void app_main(void)
{
    // Configuration de TRIG en sortie
    gpio_config_t trig_config = {
        .pin_bit_mask = (1ULL << TRIG_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&trig_config);


    // Configuration de ECHO en entrée
    gpio_config_t echo_config = {
        .pin_bit_mask = (1ULL << ECHO_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&echo_config);


    // État initial propre de TRIG
    gpio_set_level(TRIG_GPIO, 0);


    while (1)
    {
        DistanceMeasurement measurement = measure_distance();

        if (measurement.valid)
        {
            printf(
                "Distance = %.1f cm | timestamp = %lld us\n",
                measurement.distance_cm,
                measurement.timestamp_us
            );
        }
        else
        {
            printf(
                "MESURE INVALIDE | timestamp = %lld us\n",
                measurement.timestamp_us
            );
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}