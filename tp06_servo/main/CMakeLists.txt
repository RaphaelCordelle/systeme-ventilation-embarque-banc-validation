idf_component_register(
    SRCS "tp06_servo_main.c"
    PRIV_REQUIRES esp_driver_gpio esp_driver_ledc
    INCLUDE_DIRS ""
)