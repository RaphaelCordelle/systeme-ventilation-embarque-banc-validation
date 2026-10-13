# TP07 — Exigences du DUT

Ce document définit les comportements attendus du système avant l'implémentation de la machine à états.

| ID | Exigence | Criticité | Vérification |
|---|---|---|---|
| REQ-001 | Sur réception d'une commande OPEN, le système doit démarrer l'ouverture du volet. | Moyenne | Test fonctionnel |
| REQ-002 | Sur réception d'une commande CLOSE, le système doit démarrer la fermeture du volet. | Moyenne | Test fonctionnel |
| REQ-003 | Lorsque le fin de course OPEN est détecté pendant une ouverture, le mouvement doit être arrêté. | Haute | Test fonctionnel |
| REQ-004 | Lorsque le fin de course CLOSED est détecté pendant une fermeture, le mouvement doit être arrêté. | Haute | Test fonctionnel |
| REQ-005 | Si le fin de course OPEN n'est pas détecté dans les 3000 ms suivant le début de l'ouverture, le système doit arrêter le mouvement et signaler une erreur. | Haute | Test fonctionnel |
| REQ-006 | Si le fin de course CLOSED n'est pas détecté dans les 3000 ms suivant le début de la fermeture, le système doit arrêter le mouvement et signaler une erreur. | Haute | Test fonctionnel |
| REQ-007 | Si les fins de course OPEN et CLOSED sont actives simultanément, le système doit considérer l'état comme incohérent. | Haute | Test fonctionnel |
| REQ-008 | En cas d'état incohérent, le système doit arrêter l'actionneur. | Haute | Test fonctionnel |
| REQ-009 | Au démarrage, si seul le fin de course CLOSED est actif, l'état initial doit être CLOSED. | Moyenne | Test |
| REQ-010 | Au démarrage, si seul le fin de course OPEN est actif, l'état initial doit être OPEN. | Moyenne | Test |
| REQ-011 | Au démarrage, si aucun fin de course n'est actif, l'état initial doit être UNKNOWN. | Moyenne | Test |
| REQ-012 | Une commande OPEN reçue lorsque le système est déjà OPEN ne doit pas provoquer de nouveau mouvement. | Faible | Test |
| REQ-013 | Une commande CLOSE reçue lorsque le système est déjà CLOSED ne doit pas provoquer de nouveau mouvement. | Faible | Test |
| REQ-014 | Les seuils temporels du système doivent être définis de manière centralisée. | Faible | Inspection du code |
| REQ-015 | La logique de contrôle ne doit pas dépendre directement des numéros de GPIO. | Moyenne | Inspection du code |