# Architecture logicielle TP16.5

## DUT

```text
Telemetry/BME280 ---\
HC-SR04 ------------> SensorTask ---> latest snapshot ---> UART diagnostics
End-stops ----------/      |
                             v
                         ControlTask ---> FSM
                             |             |
                             |             +--> indicators LEDs/buzzer
                             v
                        ActuatorTask
                             |
                             v
                            SG90
```

`SensorTask` est propriétaire de l'acquisition physique. Le dernier `sensor_snapshot_t` est partagé via un petit module protégé par mutex afin que le protocole UART puisse exposer une télémétrie cohérente sans lancer une seconde lecture I2C concurrente.

`ControlTask` applique les retours de position, les commandes manuelles, le mode automatique, la sécurité obstacle et le timeout de mouvement.

Une commande manuelle bascule automatiquement en MANUAL. `AUTO_ON` réactive la logique température/humidité.

## BENCH

```text
BME280 reference ---> Bench Controller <--- UART ---> DUT
                           |
                           +--> comparison / timing / smoke test
                           |
                           +--> USB serial commands
                                      |
                                      v
                                Python host
```

Le mutex BENCH empêche `ComparisonTask` et la supervision PC d'effectuer deux transactions DUT simultanées.

## États DUT

```text
UNKNOWN
  | fin de course CLOSED / OPEN
  v
CLOSED --OPEN--> OPENING --LIMIT_OPEN--> OPEN
  ^                                  |
  |                                  |
  +----LIMIT_CLOSED<-- CLOSING <-----+
                         |
                         +--OBSTACLE--> OPENING

CONTRADICTION / SENSOR_FAULT / TIMEOUT
                 |
                 v
                SAFE
```
