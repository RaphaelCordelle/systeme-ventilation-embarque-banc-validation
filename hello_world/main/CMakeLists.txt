idf_component_register(
    SRCS "hello_world_main.c"
    PRIV_REQUIRES esp_driver_gpio nvs_flash
    INCLUDE_DIRS ""
)
