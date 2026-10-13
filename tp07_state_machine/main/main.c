#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "state_machine.h"

void app_main(void)
{
    state_machine_init();

    printf("\n--- TEST 1 : ouverture normale ---\n");

    state_machine_handle_event(EVENT_CMD_OPEN);

    vTaskDelay(pdMS_TO_TICKS(1000));

    state_machine_handle_event(EVENT_LIMIT_OPEN);

    vTaskDelay(pdMS_TO_TICKS(1000));

    printf("\n--- TEST 2 : fermeture normale ---\n");

    state_machine_handle_event(EVENT_CMD_CLOSE);

    vTaskDelay(pdMS_TO_TICKS(1000));

    state_machine_handle_event(EVENT_LIMIT_CLOSED);

    vTaskDelay(pdMS_TO_TICKS(1000));

    printf("\n--- TEST 3 : timeout ---\n");

    state_machine_handle_event(EVENT_CMD_OPEN);

    vTaskDelay(pdMS_TO_TICKS(1000));

    state_machine_handle_event(EVENT_TIMEOUT);

    printf("\n--- TEST 4 : commande inutile ---\n");


    state_machine_handle_event(EVENT_LIMIT_OPEN);

    state_machine_handle_event(EVENT_CMD_OPEN);

    printf("\n--- TEST 5 : incoherence ---\n");


    state_machine_handle_event(EVENT_CMD_OPEN);

    state_machine_handle_event(EVENT_LIMIT_CLOSED);
}
