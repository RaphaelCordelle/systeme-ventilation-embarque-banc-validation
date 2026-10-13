#ifndef INDICATORS_H
#define INDICATORS_H

#include <stdbool.h>
#include "state_machine.h"

void indicators_init(void);
void indicators_apply(system_state_t state, bool sensor_fault);
void indicators_buzzer_on(void);
void indicators_buzzer_off(void);

#endif
