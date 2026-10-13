# Fault Catalog

| ID | Faute | Mode d'injection | Risque | Comportement attendu | Retour à la normale |
|---|---|---|---|---|---|
| F01 | Capteur BME280 déconnecté | Débrancher le BME280 ou simuler une lecture invalide | Faible | La mesure devient indisponible et une erreur est signalée. Le système ne doit pas considérer une valeur invalide comme correcte | Rebrancher le capteur ou désactiver la faute |
| F02 | Capteur figé | Conserver volontairement la dernière valeur mesurée | Faible | La valeur reste constante malgré les variations réelles. Le défaut doit être identifiable lors de l'analyse | Désactiver l'injection et reprendre les mesures réelles |
| F03 | Dérive du capteur | Ajouter un offset constant à la mesure | Faible | La mesure reste plausible mais devient systématiquement fausse | Supprimer l'offset injecté |
| F04 | Bruit de mesure | Ajouter une perturbation variable aux mesures | Faible | Les valeurs fluctuent anormalement autour de la valeur réelle | Désactiver l'injection de bruit |
| F05 | Distance invalide | Forcer une distance impossible, par exemple -1 cm | Faible | La donnée doit être détectée comme invalide et ne pas être utilisée comme mesure normale | Désactiver la faute |
| F06 | Servo lent | Simuler ou provoquer un mouvement dépassant le temps maximal autorisé | Faible à moyen | Le mouvement dépasse le timeout, provoque EVENT_TIMEOUT puis le passage en SAFE | Réinitialiser le DUT et restaurer le fonctionnement normal |
| F07 | Servo bloqué | Empêcher le mouvement ou empêcher l'activation de la fin de course attendue | Moyen | Absence de fin de course dans le délai autorisé, timeout puis SAFE et ACTUATOR_STOP | Débloquer le servo puis réinitialiser le DUT |
| F08 | Fins de course contradictoires | Forcer OPEN=1 et CLOSED=1 simultanément | Faible | Détection de contradiction, EVENT_CONTRADICTION, passage en SAFE et arrêt de l'actionneur | Relâcher les fins de course puis réinitialiser le DUT |
| F09 | Perte UART BENCH-DUT | Débrancher temporairement TX/RX ou supprimer volontairement une réponse | Faible | Le BENCH ne reçoit aucune réponse et doit détecter une erreur de communication ou un timeout | Rebrancher l'UART ou désactiver l'injection |
| F10 | Réponse UART retardée | Ajouter volontairement un délai avant la réponse | Faible | La réponse est reçue mais dépasse la latence maximale autorisée | Supprimer le délai injecté |
| F11 | Checksum UART corrompu | Modifier volontairement le checksum d'une trame | Faible | La trame doit être rejetée comme invalide et ne doit pas déclencher de commande | Envoyer de nouveau une trame correcte |
| F12 | Reboot du DUT | Appuyer sur RESET ou provoquer un redémarrage contrôlé | Faible | Le BENCH constate temporairement la perte de communication puis le DUT redémarre dans un état initial connu | Attendre le redémarrage complet et réinitialiser l'état du système |