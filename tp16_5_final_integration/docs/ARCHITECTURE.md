# Architecture de la version intégrée

## Carte DUT

```text
BME280, HC-SR04, fins de course
              │
         SensorTask → dernier snapshot → diagnostic UART
              │
         ControlTask → machine à états, LED et buzzer
              │
         ActuatorTask → servo SG90
```

`SensorTask` réalise les acquisitions. Le dernier `sensor_snapshot_t` est partagé sous mutex :
la télémétrie réutilise cette mesure sans lancer une lecture I2C concurrente.

`ControlTask` traite les retours de position, les commandes, le mode AUTO et les défauts.
`ActuatorTask` applique les commandes du servo. Une commande manuelle désactive AUTO ;
`AUTO_ON` réactive la régulation par température et humidité.

## Carte BENCH et PC

Le BENCH relie le DUT au PC et possède son propre BME280. Il compare les mesures, chronomètre les
réponses et réalise le smoke test. Un mutex empêche les interrogations périodiques et les commandes
PC d'utiliser simultanément l'UART du DUT.

Le superviseur Python demande des snapshots au BENCH par USB série, enregistre les changements
d'état et génère les rapports d'essai.

## Machine à états

```text
CLOSED → OPENING → OPEN → CLOSING → CLOSED
                           │
                        obstacle → OPENING

Défaut capteur répété, contradiction ou timeout → SAFE
```

Au démarrage, l'état est `UNKNOWN` jusqu'à l'identification de la position.
Les fins de course confirment les positions `OPEN` et `CLOSED`. `SAFE` arrête le mouvement
et verrouille l'alarme jusqu'au redémarrage.
