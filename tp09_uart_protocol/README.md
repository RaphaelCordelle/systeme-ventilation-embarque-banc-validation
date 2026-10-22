# TP09 — Protocole UART

Ce firmware reçoit et décode les trames UART, vérifie leur checksum et répond aux requêtes du banc.
La FSM est initialisée à UNKNOWN ; cette étape teste la communication et la télémétrie du BME280.

Le [format des messages](protocol_v0.md) détaille les requêtes, les réponses et les codes d'erreur.
