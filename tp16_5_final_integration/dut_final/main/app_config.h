#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "driver/gpio.h"
#include "driver/ledc.h"

/*
 * TP16.5 - configuration materielle et seuils de demonstration.
 *
 * Tous les nombres de broches et seuils applicatifs sont centralises ici afin
 * d'eviter les "magic numbers" disperses dans le firmware.
 */

/* DUT BME280 */
#define DUT_BME_SDA_GPIO       GPIO_NUM_32
#define DUT_BME_SCL_GPIO       GPIO_NUM_33
#define DUT_BME_I2C_ADDRESS    0x76

/* HC-SR04 - ECHO doit rester adapte 5 V -> 3,3 V */
#define HC_SR04_TRIG_GPIO      GPIO_NUM_21
#define HC_SR04_ECHO_GPIO      GPIO_NUM_22
#define HC_SR04_TIMEOUT_US     30000
#define HC_SR04_MIN_CM         2.0f
#define HC_SR04_MAX_CM         400.0f
#define OBSTACLE_THRESHOLD_CM  15.0f

/* Retour de position */
#define CLOSED_LIMIT_GPIO      GPIO_NUM_18
#define OPEN_LIMIT_GPIO        GPIO_NUM_19
#define ENDSTOP_DEBOUNCE_MS    10

/* Servo SG90 - alimentation 5 V separee, masse commune */
#define SERVO_GPIO             GPIO_NUM_13
#define SERVO_TIMER            LEDC_TIMER_1
#define SERVO_CHANNEL          LEDC_CHANNEL_1
#define SERVO_OPEN_PULSE_MS    0.9f
#define SERVO_CLOSE_PULSE_MS   2.5f
#define MOVEMENT_TIMEOUT_MS    3000

/* Indicateurs du TP04 */
#define RED_LED_GPIO           GPIO_NUM_25
#define YELLOW_LED_GPIO        GPIO_NUM_26
#define GREEN_LED_GPIO         GPIO_NUM_27
#define BUZZER_GPIO            GPIO_NUM_23
#define BUZZER_TIMER           LEDC_TIMER_0
#define BUZZER_CHANNEL         LEDC_CHANNEL_0
#define BUZZER_FREQUENCY_HZ    2000

/* UART BENCH <-> DUT */
#define DUT_UART_TX_GPIO       GPIO_NUM_17
#define DUT_UART_RX_GPIO       GPIO_NUM_16
#define DUT_UART_BAUDRATE      115200

/* Cadence d'acquisition et cadence des logs de synthese */
#define SENSOR_TASK_PERIOD_MS  100
#define SENSOR_LOG_PERIOD_MS   1000

/*
 * Regulation automatique avec hysteresis.
 * Ces valeurs sont volontairement faciles a modifier pour la video de demo.
 *
 * Ouverture : T >= 26 C OU H >= 70 %
 * Fermeture : T <= 25 C ET H <= 65 %
 */
#define AUTO_OPEN_TEMP_C       26.0f
#define AUTO_CLOSE_TEMP_C      25.0f
#define AUTO_OPEN_HUMIDITY_PCT 70.0f
#define AUTO_CLOSE_HUMIDITY_PCT 65.0f

#endif
