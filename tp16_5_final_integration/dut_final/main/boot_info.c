#include "boot_info.h"

#include "nvs.h"
#include "nvs_flash.h"

uint32_t boot_info_init_and_increment(void)
{
    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        if (nvs_flash_erase() != ESP_OK || nvs_flash_init() != ESP_OK)
        {
            return 0;
        }
    }
    else if (err != ESP_OK)
    {
        return 0;
    }

    nvs_handle_t handle;
    uint32_t boot_count = 0;

    if (nvs_open("system", NVS_READWRITE, &handle) != ESP_OK)
    {
        return 0;
    }

    esp_err_t get_result = nvs_get_u32(handle, "boot_count", &boot_count);
    if (get_result != ESP_OK && get_result != ESP_ERR_NVS_NOT_FOUND)
    {
        nvs_close(handle);
        return 0;
    }

    boot_count++;

    if (nvs_set_u32(handle, "boot_count", boot_count) == ESP_OK)
    {
        nvs_commit(handle);
    }

    nvs_close(handle);
    return boot_count;
}
