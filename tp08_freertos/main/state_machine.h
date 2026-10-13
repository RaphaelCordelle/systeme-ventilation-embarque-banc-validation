#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

typedef enum
{
    STATE_UNKNOWN = 0,
    STATE_CLOSED,
    STATE_OPENING,
    STATE_OPEN,
    STATE_CLOSING,
    STATE_SAFE

} system_state_t;


typedef enum
{
    EVENT_NONE = 0,

    EVENT_CMD_OPEN,
    EVENT_CMD_CLOSE,

    EVENT_LIMIT_OPEN,
    EVENT_LIMIT_CLOSED,

    EVENT_TIMEOUT,
    EVENT_CONTRADICTION

} system_event_t;


void state_machine_init(void);

void state_machine_handle_event(
    system_event_t event
);

system_state_t state_machine_get_state(void);

const char *state_machine_state_to_string(
    system_state_t state
);

#endif
