#include "state_machine.h"

#include "esp_log.h"

static const char *TAG = "FSM";
static system_state_t current_state = STATE_UNKNOWN;

void state_machine_init(void)
{
    current_state = STATE_UNKNOWN;
    ESP_LOGI(TAG, "INIT -> UNKNOWN");
}

system_state_t state_machine_get_state(void)
{
    return current_state;
}

const char *state_machine_state_to_string(system_state_t state)
{
    switch (state)
    {
        case STATE_UNKNOWN: return "UNKNOWN";
        case STATE_CLOSED: return "CLOSED";
        case STATE_OPENING: return "OPENING";
        case STATE_OPEN: return "OPEN";
        case STATE_CLOSING: return "CLOSING";
        case STATE_SAFE: return "SAFE";
        default: return "INVALID";
    }
}

const char *state_machine_event_to_string(system_event_t event)
{
    switch (event)
    {
        case EVENT_CMD_OPEN: return "CMD_OPEN";
        case EVENT_CMD_CLOSE: return "CMD_CLOSE";
        case EVENT_LIMIT_OPEN: return "LIMIT_OPEN";
        case EVENT_LIMIT_CLOSED: return "LIMIT_CLOSED";
        case EVENT_TIMEOUT: return "TIMEOUT";
        case EVENT_CONTRADICTION: return "CONTRADICTION";
        case EVENT_SENSOR_FAULT: return "SENSOR_FAULT";
        case EVENT_OBSTACLE: return "OBSTACLE";
        default: return "UNKNOWN_EVENT";
    }
}

void state_machine_handle_event(system_event_t event)
{
    system_state_t previous = current_state;

    switch (current_state)
    {
        case STATE_UNKNOWN:
            if (event == EVENT_LIMIT_OPEN)
            {
                current_state = STATE_OPEN;
            }
            else if (event == EVENT_LIMIT_CLOSED)
            {
                current_state = STATE_CLOSED;
            }
            else if (event == EVENT_CMD_OPEN)
            {
                current_state = STATE_OPENING;
            }
            else if (event == EVENT_CMD_CLOSE)
            {
                current_state = STATE_CLOSING;
            }
            else if (event == EVENT_CONTRADICTION ||
                     event == EVENT_SENSOR_FAULT ||
                     event == EVENT_TIMEOUT)
            {
                current_state = STATE_SAFE;
            }
            break;

        case STATE_CLOSED:
            if (event == EVENT_CMD_OPEN)
            {
                current_state = STATE_OPENING;
            }
            else if (event == EVENT_LIMIT_OPEN ||
                     event == EVENT_CONTRADICTION ||
                     event == EVENT_SENSOR_FAULT)
            {
                current_state = STATE_SAFE;
            }
            break;

        case STATE_OPENING:
            if (event == EVENT_LIMIT_OPEN)
            {
                current_state = STATE_OPEN;
            }
            else if (event == EVENT_TIMEOUT ||
                     event == EVENT_CONTRADICTION ||
                     event == EVENT_SENSOR_FAULT)
            {
                current_state = STATE_SAFE;
            }
            /* LIMIT_CLOSED is expected briefly while leaving the closed stop. */
            break;

        case STATE_OPEN:
            if (event == EVENT_CMD_CLOSE)
            {
                current_state = STATE_CLOSING;
            }
            else if (event == EVENT_LIMIT_CLOSED ||
                     event == EVENT_CONTRADICTION ||
                     event == EVENT_SENSOR_FAULT)
            {
                current_state = STATE_SAFE;
            }
            break;

        case STATE_CLOSING:
            if (event == EVENT_LIMIT_CLOSED)
            {
                current_state = STATE_CLOSED;
            }
            else if (event == EVENT_OBSTACLE)
            {
                /* Safe reaction retained for TP16.5: reopen the flap. */
                current_state = STATE_OPENING;
            }
            else if (event == EVENT_TIMEOUT ||
                     event == EVENT_CONTRADICTION ||
                     event == EVENT_SENSOR_FAULT)
            {
                current_state = STATE_SAFE;
            }
            /* LIMIT_OPEN is expected briefly while leaving the open stop. */
            break;

        case STATE_SAFE:
            /* SAFE is latched until reboot. */
            break;
    }

    if (current_state != previous)
    {
        ESP_LOGI(
            TAG,
            "%s --%s--> %s",
            state_machine_state_to_string(previous),
            state_machine_event_to_string(event),
            state_machine_state_to_string(current_state)
        );
    }
}
