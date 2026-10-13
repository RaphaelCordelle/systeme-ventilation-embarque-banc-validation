idf_component_register(
    SRCS
        "main.c"
        "uart_link.c"
        "protocol.c"
        "parser.c"
        "state_machine.c"
        "telemetry_source.c"
        "bme280.c"

    INCLUDE_DIRS
        "."

    PRIV_REQUIRES
        esp_driver_i2c
        esp_driver_uart
)
