#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stdint.h>

#include "protocol_types.h"

typedef enum
{
    PARSER_WAIT_SOF = 0,
    PARSER_READ_VERSION,
    PARSER_READ_TYPE,
    PARSER_READ_LENGTH,
    PARSER_READ_PAYLOAD,
    PARSER_READ_CHECK

} parser_state_t;

typedef struct
{
    parser_state_t state;

    protocol_frame_t frame;

    uint8_t payload_index;

    uint32_t last_byte_time;

} parser_t;

void parser_init(
    parser_t *parser
);

bool parser_process_byte(
    parser_t *parser,
    uint8_t byte,
    protocol_frame_t *frame
);

void parser_check_timeout(
    parser_t *parser
);

#endif
