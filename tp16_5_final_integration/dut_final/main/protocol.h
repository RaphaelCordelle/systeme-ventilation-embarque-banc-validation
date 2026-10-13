#ifndef PROTOCOL_H
#define PROTOCOL_H
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdint.h>

#include "protocol_types.h"

uint8_t protocol_compute_check(
    const protocol_frame_t *frame
);

void protocol_send_frame(
    const protocol_frame_t *frame
);

void protocol_handle_frame(
    const protocol_frame_t *frame
);


void protocol_set_command_queue(
    QueueHandle_t command_queue
);


#endif
