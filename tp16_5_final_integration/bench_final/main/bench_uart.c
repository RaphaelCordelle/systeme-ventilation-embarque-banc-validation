#include "bench_uart.h"

#include "bench_config.h"
#include "freertos/FreeRTOS.h"
#include "driver/uart.h"

#define BENCH_UART UART_NUM_1
#define BENCH_UART_BUFFER_SIZE 256

void bench_uart_init(void)
{
    uart_config_t config = {
        .baud_rate = BENCH_UART_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_driver_install(
        BENCH_UART,
        BENCH_UART_BUFFER_SIZE,
        BENCH_UART_BUFFER_SIZE,
        0,
        NULL,
        0
    );

    uart_param_config(BENCH_UART, &config);
    uart_set_pin(
        BENCH_UART,
        BENCH_UART_TX_GPIO,
        BENCH_UART_RX_GPIO,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE
    );
}

int bench_uart_send(const uint8_t *data, size_t length)
{
    return uart_write_bytes(BENCH_UART, data, length);
}

int bench_uart_receive(uint8_t *buffer, size_t max_length)
{
    return uart_read_bytes(
        BENCH_UART,
        buffer,
        max_length,
        pdMS_TO_TICKS(500)
    );
}
