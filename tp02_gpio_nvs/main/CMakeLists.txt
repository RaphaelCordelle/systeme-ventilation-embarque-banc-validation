idf_component_register(
    SRCS "tp02_gpio_nvs_main.c"
    PRIV_REQUIRES esp_driver_gpio nvs_flash
    INCLUDE_DIRS ""
)
