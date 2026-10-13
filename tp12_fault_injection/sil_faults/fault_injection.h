#ifndef FAULT_INJECTION_H
#define FAULT_INJECTION_H

#include <stdbool.h>

typedef enum
{
    FAULT_NONE = 0,

    FAULT_SENSOR_DISCONNECTED,
    FAULT_SENSOR_FROZEN,
    FAULT_SENSOR_DRIFT,
    FAULT_SENSOR_NOISE,
    FAULT_INVALID_DISTANCE,

    FAULT_SERVO_SLOW,
    FAULT_SERVO_BLOCKED,

    FAULT_ENDSTOP_CONTRADICTION,

    FAULT_UART_LOSS,
    FAULT_UART_DELAY,
    FAULT_UART_CORRUPTED_FRAME,

    FAULT_DUT_REBOOT

} fault_type_t;


typedef struct
{
    fault_type_t active_fault;
    bool enabled;

} fault_injection_config_t;


void fault_injection_init(void);

void fault_injection_enable(
    fault_type_t fault
);

void fault_injection_disable(void);

bool fault_injection_is_enabled(
    fault_type_t fault
);

fault_type_t fault_injection_get_active(void);

float fault_injection_apply_distance(
    float distance_cm
);

#endif