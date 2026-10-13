#ifndef PROTOCOL_TYPES_H
#define PROTOCOL_TYPES_H

#include <stdint.h>

#define PROTOCOL_SOF 0xAA
#define PROTOCOL_VERSION 0x00
#define PROTOCOL_MAX_PAYLOAD 56

typedef enum
{
    MSG_HELLO = 0x01,
    MSG_GET_STATUS = 0x02,
    MSG_SET_MODE = 0x03,
    MSG_GET_TELEMETRY = 0x04,
    MSG_GET_VERSION = 0x05,

    MSG_RESPONSE = 0x80,
    MSG_ERROR = 0x81

} protocol_message_type_t;

typedef struct
{
    uint8_t sof;
    uint8_t version;
    uint8_t type;
    uint8_t length;

    uint8_t payload[PROTOCOL_MAX_PAYLOAD];

    uint8_t check;

} protocol_frame_t;

#endif
