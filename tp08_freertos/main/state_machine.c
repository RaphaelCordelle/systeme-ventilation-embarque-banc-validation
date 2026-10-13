#include "state_machine.h"

static system_state_t current_state =
    STATE_UNKNOWN;


void state_machine_init(void)
{
    current_state =
        STATE_UNKNOWN;
}


system_state_t state_machine_get_state(void)
{
    return current_state;
}


void state_machine_handle_event(
    system_event_t event
)
{
    if (
        event == EVENT_TIMEOUT ||
        event == EVENT_CONTRADICTION
    )
    {
        current_state =
            STATE_SAFE;

        return;
    }


    switch (current_state)
    {
        case STATE_UNKNOWN:

            if (
                event ==
                EVENT_LIMIT_CLOSED
            )
            {
                current_state =
                    STATE_CLOSED;
            }

            else if (
                event ==
                EVENT_LIMIT_OPEN
            )
            {
                current_state =
                    STATE_OPEN;
            }

            break;


        case STATE_CLOSED:

            if (
                event ==
                EVENT_CMD_OPEN
            )
            {
                current_state =
                    STATE_OPENING;
            }

            break;


        case STATE_OPENING:

            if (
                event ==
                EVENT_LIMIT_OPEN
            )
            {
                current_state =
                    STATE_OPEN;
            }

            break;


        case STATE_OPEN:

            if (
                event ==
                EVENT_CMD_CLOSE
            )
            {
                current_state =
                    STATE_CLOSING;
            }

            break;


        case STATE_CLOSING:

            if (
                event ==
                EVENT_LIMIT_CLOSED
            )
            {
                current_state =
                    STATE_CLOSED;
            }

            break;


        case STATE_SAFE:

            break;


        default:

            current_state =
                STATE_SAFE;

            break;
    }
}


const char *state_machine_state_to_string(
    system_state_t state
)
{
    switch (state)
    {
        case STATE_UNKNOWN:
            return "UNKNOWN";

        case STATE_CLOSED:
            return "CLOSED";

        case STATE_OPENING:
            return "OPENING";

        case STATE_OPEN:
            return "OPEN";

        case STATE_CLOSING:
            return "CLOSING";

        case STATE_SAFE:
            return "SAFE";

        default:
            return "INVALID";
    }
}
