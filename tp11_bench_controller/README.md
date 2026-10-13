# TP11 — Contrôleur de banc

Ce firmware pilote un ESP32 de banc indépendant du DUT. Au démarrage, il :

1. initialise son BME280 de référence ;
2. initialise la liaison UART vers le DUT ;
3. demande l'état du DUT avec `GET_STATUS` ;
4. demande ses mesures avec `GET_TELEMETRY` ;
5. calcule les écarts température, humidité et pression ;
6. exécute un smoke test minimal.

## Raccordements

### BME280 de référence

- bus : I2C0 ;
- SDA : GPIO21 ;
- SCL : GPIO22 ;
- adresse : 0x76.

Le pilote Bosch et la configuration sont repris du TP03.

### UART vers le DUT

- débit : 115200 bauds, 8N1 ;
- TX du banc : GPIO17 vers RX du DUT ;
- RX du banc : GPIO16 vers TX du DUT ;
- masse commune obligatoire.

Le protocole attendu est celui du TP09, version `0x00`. La télémétrie
contient 9 octets : état, température x100, humidité x100 et pression en Pa.

## Validation

La compilation vérifie la cohérence logicielle. La validation finale requiert
deux ESP32, deux BME280, le câblage UART et une vérification des valeurs réelles.
Les tolérances ne sont volontairement pas figées avant cette campagne de mesures.
