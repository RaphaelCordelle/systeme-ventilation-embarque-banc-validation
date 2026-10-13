#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#include "protocol_types.h"

uint8_t protocol_compute_check(
    const protocol_frame_t *frame
);

void protocol_handle_frame(
    const protocol_frame_t *frame
);

#endif
