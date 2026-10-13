#include "fault_injection.h"


static fault_injection_config_t config = {
    .active_fault = FAULT_NONE,
    .enabled = false
};


void fault_injection_init(void)
{
    config.active_fault =
        FAULT_NONE;

    config.enabled =
        false;
}


void fault_injection_enable(
    fault_type_t fault
)
{
    config.active_fault =
        fault;

    config.enabled =
        true;
}


void fault_injection_disable(void)
{
    config.active_fault =
        FAULT_NONE;

    config.enabled =
        false;
}


bool fault_injection_is_enabled(
    fault_type_t fault
)
{
    return
        config.enabled &&
        config.active_fault == fault;
}


fault_type_t fault_injection_get_active(void)
{
    if (!config.enabled)
    {
        return FAULT_NONE;
    }

    return config.active_fault;
}


float fault_injection_apply_distance(
    float distance_cm
)
{
    if (
        fault_injection_is_enabled(
            FAULT_INVALID_DISTANCE
        )
    )
    {
        return -1.0f;
    }

    return distance_cm;
}