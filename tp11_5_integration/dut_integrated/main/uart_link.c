#include "uart_link.h"

#include "driver/uart.h"
#include "freertos/FreeRTOS.h"

#define UART_LINK_PORT UART_NUM_1

#define UART_LINK_TX_PIN 17
#define UART_LINK_RX_PIN 16

#define UART_LINK_BAUDRATE 115200

#define UART_LINK_BUFFER_SIZE 256

void uart_link_init(void)
{
    uart_config_t config = {
        .baud_rate = UART_LINK_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_driver_install(
        UART_LINK_PORT,
        UART_LINK_BUFFER_SIZE,
        UART_LINK_BUFFER_SIZE,
        0,
        NULL,
        0
    );

    uart_param_config(
        UART_LINK_PORT,
        &config
    );

    uart_set_pin(
        UART_LINK_PORT,
        UART_LINK_TX_PIN,
        UART_LINK_RX_PIN,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE
    );
}

int uart_link_read(
    uint8_t *buffer,
    size_t max_length
)
{
    return uart_read_bytes(
        UART_LINK_PORT,
        buffer,
        max_length,
        pdMS_TO_TICKS(100)
    );
}

int uart_link_write(
    const uint8_t *data,
    size_t length
)
{
    return uart_write_bytes(
        UART_LINK_PORT,
        data,
        length
    );
}
