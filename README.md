# Système de ventilation embarqué et banc de validation

<p align="center">
  <img src="docs/images/banc-esp32.jpg" alt="Banc d’essai ESP32 sur breadboard" width="520">
</p>

Ce dépôt présente l'évolution d'un projet étudiant ESP32, depuis la validation de périphériques
isolés jusqu'à un système DUT/BENCH supervisé par Python. Chaque TP reste un projet ou une étape
autonome afin de conserver la progression pédagogique.

## Architecture finale

- un ESP32 DUT mesure le BME280 et le HC-SR04, lit les fins de course et commande le servo ;
- une FSM FreeRTOS gère les états, le mode automatique et le passage en SAFE ;
- un second ESP32 BENCH échange avec le DUT par UART et fournit une mesure BME280 de référence ;
- le superviseur Python exécute la campagne, collecte les snapshots et exporte les rapports.

## Progression

| Étape | Contenu principal |
| --- | --- |
| TP01 | Non inclus dans ce dépôt. |
| TP02 à TP06 | NVS, GPIO, I2C/BME280, HC-SR04, servo et fins de course. |
| TP07 à TP09 | FSM, architecture FreeRTOS et protocole UART. |
| TP10 | Premier superviseur Python et scénarios YAML. |
| TP11 / TP11.5 | Contrôleur BENCH puis intégration DUT/BENCH. |
| TP12 | Injection de défauts et modèles SIL. |
| TP13 | Interface commune de validation SIL/HIL. |
| TP14 | Traçabilité et rapports JSON, CSV et HTML. |
| TP15 | Étape de sécurisation non intégrée à la version finale, comme indiqué dans le changelog TP16.5. |
| TP16 | Campagne HIL, anomalies, retests et préparation de release. |
| TP16.5 | Baseline finale validée sur le banc physique, avec rapports JSON/CSV/HTML/XLSX. |

## Technologies

ESP-IDF 6.1, ESP32, C, FreeRTOS, UART 115200, I2C, BME280, HC-SR04, SG90, Python,
pytest et génération de rapports. Le signal ECHO du HC-SR04 doit être adapté de 5 V à 3,3 V ;
le servo utilise une alimentation 5 V séparée avec masse commune.

## Compilation et tests logiciels

Chaque dossier contenant un `CMakeLists.txt` de projet peut être compilé séparément :

```text
idf.py build
```

Les configurations reproductibles minimales sont dans les fichiers `sdkconfig.defaults`. Les tests
Python se lancent depuis le dossier du TP concerné avec le cache pytest désactivé :

```text
python -m pytest -p no:cacheprovider -q
```

Les scripts de TP16.5 utilisent un environnement ESP-IDF déjà chargé, ou la variable `IDF_PATH`.
Aucun flash ni test matériel n'est nécessaire pour parcourir ou vérifier les étapes SIL.

## Statut final

La campagne physique TP16.5 du 10 octobre 2026 à 12:49:52 est conservée dans
[`tp16_5_final_integration/reports`](tp16_5_final_integration/reports). Elle a produit 120
échantillons et un résultat global `PASS`. Les essais antérieurs restent des résultats historiques :
un `FAIL` ou un `SKIPPED` n'est pas converti artificiellement en succès.

## Vidéos

Essai du banc, 15 s :

[Voir `essai-banc-01.mp4`](docs/videos/essai-banc-01.mp4)

Essai du banc, 8 s :

[Voir `essai-banc-02.mp4`](docs/videos/essai-banc-02.mp4)

## Code tiers

- [Notices du pilote Bosch BME280](THIRD_PARTY_NOTICES.md)

La licence du code propriétaire du dépôt reste à choisir.
