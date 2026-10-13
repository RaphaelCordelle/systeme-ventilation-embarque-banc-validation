# Câblage final TP16.5

## DUT ESP32

| Fonction | Broche ESP32 | Notes |
|---|---:|---|
| BME280 SDA | GPIO32 | logique 3,3 V |
| BME280 SCL | GPIO33 | logique 3,3 V |
| HC-SR04 TRIG | GPIO21 | sortie ESP32 |
| HC-SR04 ECHO | GPIO22 | **pont diviseur 5 V -> ~3,3 V obligatoire** |
| Servo SG90 signal | GPIO13 | alimentation servo 5 V séparée |
| Fin de course CLOSED | GPIO18 | active-low, pull-up interne |
| Fin de course OPEN | GPIO19 | active-low, pull-up interne |
| LED rouge | GPIO25 | résistance série |
| LED jaune | GPIO26 | résistance série |
| LED verte | GPIO27 | résistance série |
| Buzzer | GPIO23 | PWM 2 kHz ; respecter le courant admissible / driver si nécessaire |
| UART TX DUT | GPIO17 | vers RX BENCH GPIO16 |
| UART RX DUT | GPIO16 | depuis TX BENCH GPIO17 |

## BENCH ESP32

| Fonction | Broche ESP32 |
|---|---:|
| BME280 référence SDA | GPIO21 |
| BME280 référence SCL | GPIO22 |
| UART TX BENCH | GPIO17 |
| UART RX BENCH | GPIO16 |

## UART entre cartes

```text
BENCH GPIO17 TX  ---> DUT GPIO16 RX
BENCH GPIO16 RX  <--- DUT GPIO17 TX
BENCH GND         --- DUT GND
```

Ne pas relier les sorties 3V3 des deux cartes entre elles.

## Servo

Le SG90 reste alimenté par une source 5 V dédiée. La masse de cette alimentation doit être commune avec la masse du DUT. Ne pas alimenter le servo depuis le 3,3 V ESP32.

## Sens des fins de course

Les entrées utilisent une pull-up interne :

```text
repos  = 1
appui  = 0
```

- CLOSED actif seul -> position CLOSED ;
- OPEN actif seul -> position OPEN ;
- aucun actif -> BETWEEN ;
- les deux actifs -> CONTRADICTORY -> SAFE.
