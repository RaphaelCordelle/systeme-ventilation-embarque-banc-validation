idf_component_register(
    SRCS
        "main.c"

        "bme280.c"
        "telemetry_source.c"

        "hc_sr04.c"
        "endstops.c"
        "servo.c"

        "state_machine.c"

        "sensor_task.c"
        "control_task.c"
        "actuator_task.c"

        "uart_link.c"
        "parser.c"
        "protocol.c"

    INCLUDE_DIRS
        "."

    PRIV_REQUIRES
        esp_driver_i2c
        esp_driver_uart
        esp_driver_gpio
        esp_driver_ledc
        esp_timer
)