#include "telemetry_source.h"

#include <string.h>

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_rom_sys.h"

#include "bme280.h"
#include "app_config.h"

static const char *TAG = "TELEMETRY";

static i2c_master_bus_handle_t bus_handle;
static i2c_master_dev_handle_t sensor_handle;
static struct bme280_dev sensor;
static bool sensor_ready;

static int8_t bme280_i2c_read(
    uint8_t reg_addr,
    uint8_t *reg_data,
    uint32_t len,
    void *intf_ptr
)
{
    i2c_master_dev_handle_t handle =
        (i2c_master_dev_handle_t)intf_ptr;

    return i2c_master_transmit_receive(
        handle,
        &reg_addr,
        1,
        reg_data,
        len,
        100
    ) == ESP_OK ? BME280_OK : BME280_E_COMM_FAIL;
}

static int8_t bme280_i2c_write(
    uint8_t reg_addr,
    const uint8_t *reg_data,
    uint32_t len,
    void *intf_ptr
)
{
    if (len > 31U)
    {
        return BME280_E_INVALID_LEN;
    }

    i2c_master_dev_handle_t handle =
        (i2c_master_dev_handle_t)intf_ptr;

    uint8_t buffer[32];
    buffer[0] = reg_addr;
    memcpy(&buffer[1], reg_data, len);

    return i2c_master_transmit(
        handle,
        buffer,
        len + 1U,
        100
    ) == ESP_OK ? BME280_OK : BME280_E_COMM_FAIL;
}

static void bme280_delay_us(
    uint32_t period,
    void *intf_ptr
)
{
    (void)intf_ptr;
    esp_rom_delay_us(period);
}

static bool telemetry_source_init(void)
{
    if (sensor_ready)
    {
        return true;
    }

    if (bus_handle == NULL)
    {
        i2c_master_bus_config_t bus_config = {
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .i2c_port = I2C_NUM_0,
            .scl_io_num = DUT_BME_SCL_GPIO,
            .sda_io_num = DUT_BME_SDA_GPIO,
            .glitch_ignore_cnt = 7,
            .flags.enable_internal_pullup = true,
        };

        esp_err_t result = i2c_new_master_bus(
            &bus_config,
            &bus_handle
        );

        if (result != ESP_OK)
        {
            ESP_LOGE(TAG, "I2C bus initialization failed: %s", esp_err_to_name(result));
            return false;
        }
    }

    if (sensor_handle == NULL)
    {
        i2c_device_config_t device_config = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = DUT_BME_I2C_ADDRESS,
            .scl_speed_hz = 100000,
        };

        esp_err_t result = i2c_master_bus_add_device(
            bus_handle,
            &device_config,
            &sensor_handle
        );

        if (result != ESP_OK)
        {
            ESP_LOGE(TAG, "BME280 registration failed: %s", esp_err_to_name(result));
            return false;
        }
    }

    sensor = (struct bme280_dev){0};
    sensor.intf = BME280_I2C_INTF;
    sensor.read = bme280_i2c_read;
    sensor.write = bme280_i2c_write;
    sensor.delay_us = bme280_delay_us;
    sensor.intf_ptr = sensor_handle;

    int8_t result = bme280_init(&sensor);

    if (result != BME280_OK)
    {
        ESP_LOGE(TAG, "BME280 initialization failed: %d", result);
        return false;
    }

    struct bme280_settings settings = {0};
    settings.osr_h = BME280_OVERSAMPLING_1X;
    settings.osr_p = BME280_OVERSAMPLING_1X;
    settings.osr_t = BME280_OVERSAMPLING_1X;

    result = bme280_set_sensor_settings(
        BME280_SEL_OSR_HUM |
        BME280_SEL_OSR_PRESS |
        BME280_SEL_OSR_TEMP,
        &settings,
        &sensor
    );

    if (result != BME280_OK)
    {
        ESP_LOGE(TAG, "BME280 configuration failed: %d", result);
        return false;
    }

    result = bme280_set_sensor_mode(
        BME280_POWERMODE_NORMAL,
        &sensor
    );

    if (result != BME280_OK)
    {
        ESP_LOGE(TAG, "BME280 start failed: %d", result);
        return false;
    }

    sensor_ready = true;
    ESP_LOGI(TAG, "BME280 ready - Chip ID=0x%02X", sensor.chip_id);
    return true;
}

bool telemetry_source_read(dut_telemetry_t *telemetry)
{
    if (telemetry == NULL)
    {
        return false;
    }

    telemetry->valid = false;

    if (!telemetry_source_init())
    {
        return false;
    }

    struct bme280_data data = {0};
    int8_t result = bme280_get_sensor_data(
        BME280_ALL,
        &data,
        &sensor
    );

    if (result != BME280_OK)
    {
        ESP_LOGE(TAG, "BME280 read failed: %d", result);
        return false;
    }

    telemetry->temperature = (float)data.temperature;
    telemetry->humidity = (float)data.humidity;
    telemetry->pressure_hpa = (float)(data.pressure / 100.0);
    telemetry->valid = true;

    return true;
}
