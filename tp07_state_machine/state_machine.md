# TP07 — Machine à états

## États

- UNKNOWN : la position du volet n'est pas connue.
- CLOSED : le volet est confirmé fermé par le fin de course CLOSED.
- OPENING : le volet est en cours d'ouverture.
- OPEN : le volet est confirmé ouvert par le fin de course OPEN.
- CLOSING : le volet est en cours de fermeture.
- SAFE : le système est arrêté à cause d'une anomalie.

## Événements

- CMD_OPEN : demande d'ouverture.
- CMD_CLOSE : demande de fermeture.
- LIMIT_OPEN : fin de course OPEN détecté.
- LIMIT_CLOSED : fin de course CLOSED détecté.
- TIMEOUT : mouvement trop long.
- CONTRADICTION : les deux fins de course sont actifs simultanément.

## Table de transitions

| État courant | Événement | Nouvel état | Action |
|---|---|---|---|
| UNKNOWN | CMD_OPEN | OPENING | Démarrer l'ouverture |
| UNKNOWN | CMD_CLOSE | CLOSING | Démarrer la fermeture |
| CLOSED | CMD_OPEN | OPENING | Démarrer l'ouverture |
| CLOSED | CMD_CLOSE | CLOSED | Ne rien faire |
| OPENING | LIMIT_OPEN | OPEN | Arrêter le servo |
| OPENING | TIMEOUT | SAFE | Arrêter le servo et signaler une erreur |
| OPENING | CONTRADICTION | SAFE | Arrêter le servo et signaler une erreur |
| OPEN | CMD_CLOSE | CLOSING | Démarrer la fermeture |
| OPEN | CMD_OPEN | OPEN | Ne rien faire |
| CLOSING | LIMIT_CLOSED | CLOSED | Arrêter le servo |
| CLOSING | TIMEOUT | SAFE | Arrêter le servo et signaler une erreur |
| CLOSING | CONTRADICTION | SAFE | Arrêter le servo et signaler une erreur |
| SAFE | CMD_OPEN | SAFE | Ignorer la commande |
| SAFE | CMD_CLOSE | SAFE | Ignorer la commande |


## Transitions invalides

| État courant | Événement inattendu | Réaction |
|---|---|---|
| UNKNOWN | LIMIT_OPEN | Passer à OPEN |
| UNKNOWN | LIMIT_CLOSED | Passer à CLOSED |
| CLOSED | LIMIT_OPEN | Passer à SAFE |
| OPEN | LIMIT_CLOSED | Passer à SAFE |
| OPENING | CMD_OPEN | Ignorer la commande |
| OPENING | CMD_CLOSE | Ignorer la commande |
| CLOSING | CMD_CLOSE | Ignorer la commande |
| CLOSING | CMD_OPEN | Ignorer la commande |
| OPENING | LIMIT_CLOSED | Passer à SAFE |
| CLOSING | LIMIT_OPEN | Passer à SAFE |
| SAFE | CMD_OPEN | Ignorer |
| SAFE | CMD_CLOSE | Ignorer |