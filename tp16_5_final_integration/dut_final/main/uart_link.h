#ifndef UART_LINK_H
#define UART_LINK_H

#include <stddef.h>
#include <stdint.h>

void uart_link_init(void);

int uart_link_read(
    uint8_t *buffer,
    size_t max_length
);

int uart_link_write(
    const uint8_t *data,
    size_t length
);

#endif
