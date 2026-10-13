#include "parser.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define PARSER_TIMEOUT_MS 500

static void parser_reset(
    parser_t *parser
)
{
    parser->state =
        PARSER_WAIT_SOF;

    parser->payload_index = 0;

    memset(
        &parser->frame,
        0,
        sizeof(parser->frame)
    );
}

void parser_init(
    parser_t *parser
)
{
    parser_reset(parser);

    parser->last_byte_time =
        xTaskGetTickCount();
}

bool parser_process_byte(
    parser_t *parser,
    uint8_t byte,
    protocol_frame_t *frame
)
{
    parser->last_byte_time =
        xTaskGetTickCount();

    switch (parser->state)
    {
        case PARSER_WAIT_SOF:

            if (byte == PROTOCOL_SOF)
            {
                parser->frame.sof = byte;

                parser->state =
                    PARSER_READ_VERSION;
            }

            break;

        case PARSER_READ_VERSION:

            parser->frame.version = byte;

            if (
                byte !=
                PROTOCOL_VERSION
            )
            {
                parser_reset(parser);
                break;
            }

            parser->state =
                PARSER_READ_TYPE;

            break;

        case PARSER_READ_TYPE:

            parser->frame.type = byte;

            parser->state =
                PARSER_READ_LENGTH;

            break;

        case PARSER_READ_LENGTH:

            parser->frame.length = byte;

            if (
                byte >
                PROTOCOL_MAX_PAYLOAD
            )
            {
                parser_reset(parser);
                break;
            }

            parser->payload_index = 0;

            if (byte == 0)
            {
                parser->state =
                    PARSER_READ_CHECK;
            }
            else
            {
                parser->state =
                    PARSER_READ_PAYLOAD;
            }

            break;

        case PARSER_READ_PAYLOAD:

            parser->frame.payload[
                parser->payload_index
            ] = byte;

            parser->payload_index++;

            if (
                parser->payload_index >=
                parser->frame.length
            )
            {
                parser->state =
                    PARSER_READ_CHECK;
            }

            break;

        case PARSER_READ_CHECK:

            parser->frame.check = byte;

            *frame =
                parser->frame;

            parser_reset(parser);

            return true;

        default:

            parser_reset(parser);

            break;
    }

    return false;
}

void parser_check_timeout(
    parser_t *parser
)
{
    if (
        parser->state ==
        PARSER_WAIT_SOF
    )
    {
        return;
    }

    uint32_t now =
        xTaskGetTickCount();

    uint32_t elapsed =
        now -
        parser->last_byte_time;

    if (
        elapsed >=
        pdMS_TO_TICKS(
            PARSER_TIMEOUT_MS
        )
    )
    {
        parser_reset(parser);
    }
}
