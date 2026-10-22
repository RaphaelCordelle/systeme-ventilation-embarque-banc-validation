# Version intégrée du volet et du banc

Cette version réunit les deux firmwares ESP32 et le superviseur Python. Elle reprend les fonctions
testées dans les étapes précédentes : acquisition, commande du volet, retour des fins de course,
mode automatique et suivi des essais.

[Vidéo du montage](../media/demonstration-volet.mp4) · [Photos](../media/) ·
[Rapport matériel du 10 octobre 2026](reports/demo_20261010_124952.json)

## Les trois parties

| Partie | Rôle |
|---|---|
| [`dut_final/`](dut_final/) | Lit le BME280, le HC-SR04 et les fins de course ; pilote le SG90, les LED et le buzzer sous FreeRTOS. |
| [`bench_final/`](bench_final/) | Interroge le DUT par UART, lit le BME280 de référence et expose les commandes au PC. |
| [`host/`](host/) | Enregistre les mesures, vérifie le cycle observé et produit les rapports JSON, CSV, HTML et XLSX. |

Le mode AUTO ouvre le volet à **26 °C ou 70 % d'humidité** et le ferme à **25 °C et 65 %**.
Une commande manuelle `OPEN` ou `CLOSE` désactive AUTO ; `AUTO_ON` le réactive.
Un obstacle pendant la fermeture provoque une réouverture. Les défauts capteur répétés, une
contradiction des fins de course ou un timeout de mouvement entraînent l'état `SAFE`.

## Compiler et lancer

Prérequis : ESP-IDF 6.1, Python 3.13 et PowerShell. Depuis ce dossier, dans un terminal ESP-IDF :

```powershell
.\scripts\build_all.ps1
python -m pip install -r .\host\requirements.txt
```

Après vérification du [câblage](docs/WIRING.md), flasher les deux cartes en adaptant les ports :

```powershell
.\scripts\flash_all.ps1 -DutPort COM7 -BenchPort COM5
.\scripts\run_demo.ps1 -BenchPort COM5 -Duration 90
```

Le superviseur attend un cycle complet d'ouverture et de fermeture. Il écrit les rapports dans
`reports/` et le flux série brut dans `evidence/`.

Les scripts de compilation et de flash utilisent `idf.py` ou chargent ESP-IDF depuis `IDF_PATH`.
La [calibration du SG90](docs/SERVO_CALIBRATION.md) décrit les réglages du mécanisme.

## Validation

La session du **10 octobre 2026** contient 120 mesures et un verdict global `PASS` : communication
DUT/BENCH, mesures capteur valides, cohérence des fins de course et cycle d'ouverture/fermeture
observé. L'état `SAFE` n'a pas été sollicité pendant cette session normale.

- [Rapports JSON, CSV, HTML et Excel](reports/)
- [Journal série de la session](evidence/demo_serial_20261010_124952.log)
- [Détail des contrôles et de leur portée](docs/ACCEPTANCE_CHECKLIST.md)
- [Campagne TP16 précédente](validation_tp16/)

Pour exécuter les tests du superviseur :

```powershell
cd .\host
python -m pytest -p no:cacheprovider -q
```

## Référence technique

[Architecture](docs/ARCHITECTURE.md) · [Commandes PC](docs/COMMANDS.md) ·
[Protocole UART](docs/PROTOCOL.md) · [Intégration des TP](docs/CHANGELOG_FROM_TPS.md)

Les seuils sont définis dans [`app_config.h`](dut_final/main/app_config.h). Les rapports archivés
décrivent la session enregistrée et ne constituent pas une nouvelle mesure de ces seuils.
Le protocole n'intègre ni HMAC ni protection contre le rejeu. La configuration flash du BENCH
reste à 2 Mo, comme celle utilisée pour la campagne conservée.
