idf_component_register(
    SRCS
        "tp03_i2c_main.c"
        "bme280.c"

    PRIV_REQUIRES
        esp_driver_i2c

    INCLUDE_DIRS
        "."
)