# Réutilisation des TP précédents

Cette consolidation repart des versions réellement présentes dans le dépôt et privilégie toujours la brique la plus récente lorsque plusieurs TP implémentent la même fonction.

| Origine | Élément repris dans TP16.5 |
|---|---|
| TP02 | logs de version et compteur de boots NVS |
| TP03 | driver et acquisition BME280 |
| TP04 | fins de course, LED rouge/jaune/verte, buzzer et logique active-low |
| TP05 | HC-SR04, mesure de durée, timeout, domaine valide |
| TP06 | PWM SG90, ouverture/fermeture/stop et alimentation séparée |
| TP07 | machine à états explicite |
| TP08 | architecture FreeRTOS avec SensorTask / ControlTask / ActuatorTask |
| TP09 | UART cadré, parser incrémental, checksum XOR et timeouts |
| TP10 | principe de superviseur Python et résultats structurés |
| TP11 / 11.5 | second ESP32, BME280 référence, comparaison, timing et intégration DUT/BENCH |
| TP12 | comportements de défaut et logique SAFE |
| TP13 | séparation transport / scénario reprise conceptuellement côté hôte |
| TP14 | JSON, CSV, HTML, traçabilité/reporting ; XLSX ajouté pour TP16.5 |
| TP16 | corrections UART BENCH, contradiction fin de course, campagne HIL et preuves finales conservées dans `validation_tp16/` |

## Changements d'intégration spécifiques TP16.5

- suppression du double traitement des fins de course présent dans l'ancien `control_task.c` ;
- un seul propriétaire de l'acquisition capteur : la télémétrie UART réutilise désormais le dernier snapshot FreeRTOS au lieu de relire le BME280 en concurrence ;
- filtre majoritaire sur les fins de course ;
- mode automatique avec hystérésis ;
- obstacle pendant CLOSING -> réouverture ;
- indicateurs visuels et buzzer réintégrés ;
- diagnostic binaire enrichi DUT -> BENCH ;
- `GET_SNAPSHOT` BENCH pour un enregistrement Python propre ;
- génération XLSX en plus des formats JSON/CSV/HTML.

Le TP15 de sécurité n'est pas intégré, conformément au périmètre réellement réalisé.
