idf_component_register(
    SRCS "tp05_gpio_main.c"
    PRIV_REQUIRES esp_driver_gpio esp_timer
    INCLUDE_DIRS ""
)