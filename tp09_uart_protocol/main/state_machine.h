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

void state_machine_init(void);
system_state_t state_machine_get_state(void);

#endif
