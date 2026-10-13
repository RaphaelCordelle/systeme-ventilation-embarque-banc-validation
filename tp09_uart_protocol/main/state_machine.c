#include "state_machine.h"

static system_state_t current_state = STATE_UNKNOWN;

void state_machine_init(void)
{
    current_state = STATE_UNKNOWN;
}

system_state_t state_machine_get_state(void)
{
    return current_state;
}
