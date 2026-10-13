# TP16.5 — Banc de ventilation intégré final

Cette version consolide les briques réellement développées pendant les TP 02 à 16 en un seul démonstrateur physique cohérent.

> **Statut : baseline 1.0.0 validée sur le banc réel.** La campagne finale conservée a été
> exécutée le 10 octobre 2026 à 12:49:52 et a produit un résultat global `PASS`.

## Ce que contient la version finale

### ESP32 DUT

- BME280 DUT : température, humidité, pression.
- HC-SR04 : distance et détection d'obstacle.
- Deux fins de course : OPEN / CLOSED avec anti-rebond.
- SG90 : ouverture / fermeture du volet.
- LED rouge / jaune / verte héritées du TP04.
- Buzzer : alarme latched en état SAFE.
- FreeRTOS : SensorTask, ControlTask, ActuatorTask et tâche UART.
- Machine à états : UNKNOWN, CLOSED, OPENING, OPEN, CLOSING, SAFE.
- Mode AUTO température/humidité avec hystérésis.
- Réouverture automatique si un obstacle est détecté pendant la fermeture.
- Timeout mouvement -> SAFE.
- Contradiction des fins de course -> SAFE.
- Plusieurs erreurs BME280 consécutives -> SAFE.
- UART cadré vers le Bench Controller.
- Compteur de boot NVS repris du TP02.

### ESP32 BENCH

- BME280 de référence indépendant.
- UART BENCH <-> DUT protégé par mutex.
- Mesure de latence et resynchronisation UART.
- GET_VERSIONS, GET_REFERENCE, télémétrie et diagnostic complet.
- Commandes manuelles OPEN/CLOSE conservées pour les tests.
- Activation/désactivation du mode automatique.
- Smoke test autonome.
- Snapshot combiné DUT + référence destiné au superviseur Python.

### Superviseur Python

`host/demo_supervisor.py` enregistre une démonstration réelle et génère automatiquement :

- log série brut ;
- JSON ;
- CSV ;
- HTML ;
- XLSX Excel ;
- événements de changement d'état ;
- checks de smoke test, validité capteur, cohérence des fins de course et cycle physique observé.

## Architecture

```text
BME280 DUT ───────┐
HC-SR04 ──────────┤
Fins de course ───┤
                  v
          ESP32 DUT / FreeRTOS
          ├─ FSM / logique AUTO
          ├─ SG90 -> volet réel
          ├─ LEDs
          └─ buzzer
                  |
                  | UART 115200 8N1
                  v
          ESP32 Bench Controller
          ├─ BME280 référence
          ├─ comparaison DUT / REF
          ├─ timing / smoke test
          └─ supervision
                  |
                  | USB série
                  v
          Python demo_supervisor.py
          ├─ logs
          ├─ événements
          ├─ CSV / JSON / HTML
          └─ XLSX
```

## Démarrage rapide

Depuis un terminal dans lequel ESP-IDF 6.1 est chargé :

```powershell
.\scripts\build_all.ps1
```

Le script utilise `idf.py` présent dans le `PATH`. S'il est absent, il charge `export.ps1` depuis
la variable d'environnement `IDF_PATH`.

Installer ensuite les dépendances Python :

```powershell
cd ..
pip install -r .\host\requirements.txt
```

Puis lancer l'enregistrement de démonstration :

```powershell
python .\host\demo_supervisor.py --port COM5 --duration 90 --require-cycle
```

Les rapports sont déposés dans `reports/` et le log série brut dans `evidence/`.

## Mode automatique

Les seuils sont centralisés dans `dut_final/main/app_config.h` :

```text
OPEN  si T >= 23 °C OU H >= 70 %
CLOSE si T <= 22 °C ET H <= 65 %
```

L'écart entre ouverture et fermeture crée une hystérésis et évite que le volet oscille autour d'un seul seuil.

Une commande manuelle `OPEN` ou `CLOSE` désactive volontairement AUTO. `AUTO_ON` le réactive.

## Validation conservée

Le rapport final `reports/demo_20261010_124952.*` et le journal
`evidence/demo_serial_20261010_124952.log` constituent la preuve historique de la campagne finale.
Ils ne doivent pas être régénérés ou modifiés pour simuler une nouvelle validation matérielle.

Le dossier `validation_tp16/` conserve les éléments de la baseline TP16 précédente. La configuration
flash BENCH reste volontairement à 2 Mo pour reproduire le binaire validé ; la table de partitions
mono-application tient dans cette capacité même si la carte physique dispose de 4 Mo.

Le TP15 de sécurisation HMAC/anti-rejeu n'est pas intégré : cette version ne revendique donc aucune authentification cryptographique du protocole.
