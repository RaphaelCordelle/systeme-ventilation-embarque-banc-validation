#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    float temperature;
    float humidity;
    float pressure_hpa;

    float distance_cm;

    bool sensor_valid;
    bool distance_valid;
    bool obstacle;

    bool position_contradictory;
    bool limit_open;
    bool limit_closed;

    uint32_t timestamp_ms;

} sensor_snapshot_t;


typedef enum
{
    ACTUATOR_STOP = 0,
    ACTUATOR_OPEN,
    ACTUATOR_CLOSE

} actuator_command_t;


typedef enum
{
    USER_COMMAND_NONE = 0,
    USER_COMMAND_OPEN,
    USER_COMMAND_CLOSE,
    USER_COMMAND_AUTO_ON,
    USER_COMMAND_AUTO_OFF

} user_command_t;

#endif
