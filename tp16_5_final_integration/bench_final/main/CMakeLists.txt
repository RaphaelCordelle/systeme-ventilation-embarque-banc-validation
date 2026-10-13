idf_component_register(
    SRCS
        "main.c"
        "bench_uart.c"
        "bench_protocol.c"
        "bench_sensor.c"
        "bench_compare.c"
        "bench_timing.c"
        "bench_smoke_test.c"
        "bme280.c"

    INCLUDE_DIRS
        "."

    PRIV_REQUIRES
        esp_driver_i2c
        esp_driver_uart
        esp_driver_gpio
        esp_timer
)