#include <stdio.h>
#include "state_machine.h"

// Etat courant de la machine
static system_state_t current_state = STATE_UNKNOWN;

void state_machine_init(void)
{
    current_state = STATE_UNKNOWN;
    printf("FSM initialisee : %s\n",
           state_machine_state_to_string(current_state));
}

system_state_t state_machine_get_state(void)
{
    return current_state;
}

const char *state_machine_state_to_string(system_state_t state)
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

void state_machine_handle_event(system_event_t event)
{
    printf("Etat actuel : %s\n",
           state_machine_state_to_string(current_state));

    switch (current_state)
    {
        case STATE_UNKNOWN:

            if (event == EVENT_CMD_OPEN)
            {
                current_state = STATE_OPENING;
                printf("Action : demarrer ouverture\n");
            }
            else if (event == EVENT_CMD_CLOSE)
            {
                current_state = STATE_CLOSING;
                printf("Action : demarrer fermeture\n");
            }
            else if (event == EVENT_LIMIT_OPEN)
            {
                current_state = STATE_OPEN;
            }
            else if (event == EVENT_LIMIT_CLOSED)
            {
                current_state = STATE_CLOSED;
            }
            else if (event == EVENT_CONTRADICTION)
            {
                current_state = STATE_SAFE;
                printf("Erreur : contradiction\n");
            }

            break;

        case STATE_CLOSED:

            if (event == EVENT_CMD_OPEN)
            {
                current_state = STATE_OPENING;
                printf("Action : demarrer ouverture\n");
            }
            else if (event == EVENT_CMD_CLOSE)
            {
                printf("Commande ignoree : deja ferme\n");
            }
            else if (event == EVENT_LIMIT_OPEN)
            {
                current_state = STATE_SAFE;
                printf("Erreur : LIMIT_OPEN incoherent\n");
            }
            else if (event == EVENT_CONTRADICTION)
            {
                current_state = STATE_SAFE;
                printf("Erreur : contradiction\n");
            }

            break;

        case STATE_OPENING:

            if (event == EVENT_LIMIT_OPEN)
            {
                current_state = STATE_OPEN;
                printf("Action : arreter servo\n");
            }
            else if (event == EVENT_TIMEOUT)
            {
                current_state = STATE_SAFE;
                printf("Erreur : timeout ouverture\n");
                printf("Action : arreter servo\n");
            }
            else if (event == EVENT_CONTRADICTION)
            {
                current_state = STATE_SAFE;
                printf("Erreur : contradiction\n");
                printf("Action : arreter servo\n");
            }
            else if (event == EVENT_LIMIT_CLOSED)
            {
                current_state = STATE_SAFE;
                printf("Erreur : LIMIT_CLOSED pendant ouverture\n");
            }
            else if (event == EVENT_CMD_OPEN ||
                     event == EVENT_CMD_CLOSE)
            {
                printf("Commande ignoree pendant ouverture\n");
            }

            break;

        case STATE_OPEN:

            if (event == EVENT_CMD_CLOSE)
            {
                current_state = STATE_CLOSING;
                printf("Action : demarrer fermeture\n");
            }
            else if (event == EVENT_CMD_OPEN)
            {
                printf("Commande ignoree : deja ouvert\n");
            }
            else if (event == EVENT_LIMIT_CLOSED)
            {
                current_state = STATE_SAFE;
                printf("Erreur : LIMIT_CLOSED incoherent\n");
            }
            else if (event == EVENT_CONTRADICTION)
            {
                current_state = STATE_SAFE;
                printf("Erreur : contradiction\n");
            }

            break;

        case STATE_CLOSING:

            if (event == EVENT_LIMIT_CLOSED)
            {
                current_state = STATE_CLOSED;
                printf("Action : arreter servo\n");
            }
            else if (event == EVENT_TIMEOUT)
            {
                current_state = STATE_SAFE;
                printf("Erreur : timeout fermeture\n");
                printf("Action : arreter servo\n");
            }
            else if (event == EVENT_CONTRADICTION)
            {
                current_state = STATE_SAFE;
                printf("Erreur : contradiction\n");
                printf("Action : arreter servo\n");
            }
            else if (event == EVENT_LIMIT_OPEN)
            {
                current_state = STATE_SAFE;
                printf("Erreur : LIMIT_OPEN pendant fermeture\n");
            }
            else if (event == EVENT_CMD_OPEN ||
                     event == EVENT_CMD_CLOSE)
            {
                printf("Commande ignoree pendant fermeture\n");
            }

            break;

        case STATE_SAFE:

            printf("Systeme en SAFE : evenement ignore\n");
            break;
    }

    printf("Nouvel etat : %s\n\n",
           state_machine_state_to_string(current_state));
}