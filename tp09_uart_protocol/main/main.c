#include <stdbool.h>
#include <stdint.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "parser.h"
#include "protocol.h"
#include "state_machine.h"
#include "uart_link.h"

static const char *TAG =
    "TP09";

void app_main(void)
{
    ESP_LOGI(
        TAG,
        "TP09 UART protocol starting"
    );

    state_machine_init();

    uart_link_init();

    parser_t parser;

    parser_init(
        &parser
    );

    uint8_t buffer[64];

    protocol_frame_t frame;

    while (1)
    {
        int received =
            uart_link_read(
                buffer,
                sizeof(buffer)
            );

        if (received > 0)
        {
            for (
                int i = 0;
                i < received;
                i++
            )
            {
                bool complete =
                    parser_process_byte(
                        &parser,
                        buffer[i],
                        &frame
                    );

                if (complete)
                {
                    protocol_handle_frame(
                        &frame
                    );
                }
            }
        }

        parser_check_timeout(
            &parser
        );

        vTaskDelay(
            pdMS_TO_TICKS(10)
        );
    }
}
