#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "nvs.h"

#define LED_GPIO GPIO_NUM_18
#define FW_VERSION "1.0.0"

static const char *TAG = "TP02";

void app_main(void)
{
    // ----------------------------
    // Initialisation de la NVS
    // ----------------------------

    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // ----------------------------
    // Compteur de démarrages
    // ----------------------------

    nvs_handle_t nvs_handle;

    uint32_t boot_count = 0;

    if (nvs_open("storage", NVS_READWRITE, &nvs_handle) == ESP_OK)
    {
        nvs_get_u32(nvs_handle, "boot_count", &boot_count);

        boot_count++;

        nvs_set_u32(nvs_handle, "boot_count", boot_count);

        nvs_commit(nvs_handle);

        nvs_close(nvs_handle);
    }

    // ----------------------------
    // Informations au démarrage
    // ----------------------------

    ESP_LOGI(TAG, "Firmware version: %s", FW_VERSION);
    ESP_LOGI(TAG, "Boot count: %" PRIu32, boot_count);

    // ----------------------------
    // Configuration de la LED
    // ----------------------------

    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    // ----------------------------
    // Boucle principale
    // ----------------------------

    while (1)
    {
        ESP_LOGI(TAG, "LED ON");
        gpio_set_level(LED_GPIO, 1);

        vTaskDelay(pdMS_TO_TICKS(1000));

        ESP_LOGI(TAG, "LED OFF");
        gpio_set_level(LED_GPIO, 0);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}