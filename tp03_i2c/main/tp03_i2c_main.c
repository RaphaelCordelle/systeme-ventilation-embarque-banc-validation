#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_rom_sys.h"

#include "bme280.h"


#define I2C_SDA_GPIO GPIO_NUM_21
#define I2C_SCL_GPIO GPIO_NUM_22

#define BME280_ADDR 0x76


static const char *TAG = "TP03";


/* =========================================================
   Fonction de LECTURE demandée par le driver Bosch
   ========================================================= */
static int8_t bme280_i2c_read(
    uint8_t reg_addr,
    uint8_t *reg_data,
    uint32_t len,
    void *intf_ptr)
{
    i2c_master_dev_handle_t sensor_handle =
        (i2c_master_dev_handle_t)intf_ptr;

    esp_err_t result = i2c_master_transmit_receive(
        sensor_handle,
        &reg_addr,
        1,
        reg_data,
        len,
        100
    );

    if (result == ESP_OK)
    {
        return 0;
    }

    return -1;
}


/* =========================================================
   Fonction d'ECRITURE demandée par le driver Bosch
   ========================================================= */
static int8_t bme280_i2c_write(
    uint8_t reg_addr,
    const uint8_t *reg_data,
    uint32_t len,
    void *intf_ptr)
{
    i2c_master_dev_handle_t sensor_handle =
        (i2c_master_dev_handle_t)intf_ptr;

    uint8_t buffer[32];

    if (len > 31)
    {
        return -1;
    }

    buffer[0] = reg_addr;
    memcpy(&buffer[1], reg_data, len);

    esp_err_t result = i2c_master_transmit(
        sensor_handle,
        buffer,
        len + 1,
        100
    );

    if (result == ESP_OK)
    {
        return 0;
    }

    return -1;
}


/* =========================================================
   Fonction d'attente demandée par Bosch
   ========================================================= */
static void bme280_delay_us(
    uint32_t period,
    void *intf_ptr)
{
    (void)intf_ptr;

    esp_rom_delay_us(period);
}


/* =========================================================
   PROGRAMME PRINCIPAL
   ========================================================= */
void app_main(void)
{
    /* ---------- 1. Création du bus I2C ---------- */

    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,

        .scl_io_num = I2C_SCL_GPIO,
        .sda_io_num = I2C_SDA_GPIO,

        .glitch_ignore_cnt = 7,

        .flags.enable_internal_pullup = true,
    };

    i2c_master_bus_handle_t bus_handle;

    ESP_ERROR_CHECK(
        i2c_new_master_bus(
            &bus_config,
            &bus_handle
        )
    );


    /* ---------- 2. Ajout du BME280 ---------- */

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BME280_ADDR,
        .scl_speed_hz = 100000,
    };

    i2c_master_dev_handle_t sensor_handle;

    ESP_ERROR_CHECK(
        i2c_master_bus_add_device(
            bus_handle,
            &dev_config,
            &sensor_handle
        )
    );


    /* ---------- 3. Connexion entre Bosch et ESP32 ---------- */

    struct bme280_dev dev = {0};

    dev.intf = BME280_I2C_INTF;

    dev.read = bme280_i2c_read;
    dev.write = bme280_i2c_write;
    dev.delay_us = bme280_delay_us;

    dev.intf_ptr = sensor_handle;


    /* ---------- 4. Initialisation du BME280 ---------- */

    int8_t result = bme280_init(&dev);

    if (result != BME280_OK)
    {
        ESP_LOGE(
            TAG,
            "Erreur initialisation BME280 : %d",
            result
        );

        return;
    }

    ESP_LOGI(
        TAG,
        "BME280 initialise - Chip ID = 0x%02X",
        dev.chip_id
    );


    /* ---------- 5. Configuration des mesures ---------- */

    struct bme280_settings settings = {0};

    settings.osr_h = BME280_OVERSAMPLING_1X;
    settings.osr_p = BME280_OVERSAMPLING_1X;
    settings.osr_t = BME280_OVERSAMPLING_1X;

    result = bme280_set_sensor_settings(
        BME280_SEL_OSR_HUM |
        BME280_SEL_OSR_PRESS |
        BME280_SEL_OSR_TEMP,

        &settings,
        &dev
    );

    if (result != BME280_OK)
    {
        ESP_LOGE(TAG, "Erreur configuration BME280");
        return;
    }


    /* ---------- 6. Mode normal ---------- */

    result = bme280_set_sensor_mode(
        BME280_POWERMODE_NORMAL,
        &dev
    );

    if (result != BME280_OK)
    {
        ESP_LOGE(TAG, "Erreur activation BME280");
        return;
    }


    ESP_LOGI(TAG, "Acquisition demarree");


    /* ---------- 7. Lecture permanente ---------- */

    while (1)
    {
        struct bme280_data data;

        result = bme280_get_sensor_data(
            BME280_ALL,
            &data,
            &dev
        );

        if (result == BME280_OK)
        {
            ESP_LOGI(
                TAG,
                "T = %.2f C | P = %.2f hPa | H = %.2f %%",
                data.temperature,
                data.pressure / 100.0,
                data.humidity
            );
        }
        else
        {
            ESP_LOGE(
                TAG,
                "Erreur lecture BME280 : %d",
                result
            );
        }

        vTaskDelay(
            pdMS_TO_TICKS(1000)
        );
    }
}