#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

// Etats possibles du système
typedef enum {
    STATE_UNKNOWN,
    STATE_CLOSED,
    STATE_OPENING,
    STATE_OPEN,
    STATE_CLOSING,
    STATE_SAFE
} system_state_t;

// Evénements pouvant être reçus par la machine à états
typedef enum {
    EVENT_CMD_OPEN,
    EVENT_CMD_CLOSE,
    EVENT_LIMIT_OPEN,
    EVENT_LIMIT_CLOSED,
    EVENT_TIMEOUT,
    EVENT_CONTRADICTION
} system_event_t;

// Initialise la machine à états
void state_machine_init(void);

// Traite un événement
void state_machine_handle_event(system_event_t event);

// Retourne l'état actuel
system_state_t state_machine_get_state(void);

// Convertit un état en texte pour les logs
const char *state_machine_state_to_string(system_state_t state);

#endif