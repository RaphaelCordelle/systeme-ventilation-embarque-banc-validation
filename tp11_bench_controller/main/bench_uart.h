#ifndef BENCH_UART_H
#define BENCH_UART_H

#include <stddef.h>
#include <stdint.h>

void bench_uart_init(void);

int bench_uart_send(const uint8_t *data, size_t length);

int bench_uart_receive(uint8_t *buffer, size_t max_length);

#endif