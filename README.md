# Système de ventilation embarqué et banc de validation

J'ai construit un système de ventilation sur ESP32, puis le banc qui permet de le commander,
d'observer ses états et de rejouer des scénarios de validation. Le dépôt garde toutes les étapes,
des premiers périphériques jusqu'à l'intégration DUT/BENCH supervisée par Python.

## Pourquoi ce projet ?

Je ne voulais pas écrire tout le système final d'un bloc. J'ai commencé par tester chaque fonction
séparément, puis je les ai réunies à mesure qu'elles devenaient fiables. Quand un problème apparaît,
cette progression aide à retrouver la brique responsable au lieu de déboguer tout le système en même
temps.

Les TP ne sont donc pas une série de dossiers indépendants : ils retracent la construction du projet
et les choix faits pendant l'intégration.

## Architecture

```text
BME280 / HC-SR04 / fins de course / servo
                    |
                    v
                ESP32 DUT
        FSM + FreeRTOS + logique SAFE
                    |
           protocole UART 115200
                    |
                    v
               ESP32 BENCH
       commandes + BME280 de référence
                    |
                   USB
                    |
                    v
           Superviseur Python
                    |
                    v
       scénarios / assertions / rapports
```

Le DUT est l'équipement testé. Il lit les capteurs, commande le servo, surveille les fins de course
et fait évoluer la machine à états. En cas de contradiction, de timeout ou de défaut capteur, la
logique de contrôle peut arrêter le mouvement et passer en `SAFE`.

Le BENCH est un second ESP32, indépendant du DUT. Il lui envoie des commandes par UART, récupère ses
états et ses télémétries, puis les transmet au superviseur Python. Le superviseur exécute les scénarios
et décide du résultat `PASS`, `FAIL`, `ERROR` ou `SKIPPED` à partir des réponses observées.

## Construction progressive

Les premiers TP isolent les GPIO, l'I2C, les capteurs, le servo et les fins de course. La FSM,
FreeRTOS et le protocole UART arrivent ensuite, avant la supervision Python et le second ESP32.

Cette méthode permet de valider un comportement avant d'en dépendre dans l'étape suivante. Elle est
aussi proche d'une démarche d'intégration et de validation : assembler progressivement, provoquer
des défauts, corriger, puis rejouer les mêmes contrôles.

## Du SIL au HIL

### SIL

Le SIL utilise un transport simulé à la place du matériel. Il m'a permis de tester les scénarios, les
parsers, les cas d'erreur, les décisions de campagne et la génération des rapports sans mobiliser les
deux cartes. Les défauts difficiles à reproduire physiquement pouvaient aussi être injectés de façon
déterministe.

### HIL

Une fois les deux ESP32, les capteurs et les actionneurs réunis, le HIL a servi à vérifier le système
réel. Les campagnes ont couvert notamment les commandes d'ouverture et de fermeture, le mode AUTO,
l'obstacle pendant la fermeture, les fins de course et la télémétrie. Les rapports conservés indiquent
explicitement ce qui a été observé et ce qui n'est pas revendiqué.

## Quelques problèmes rencontrés

### Réception UART fragmentée

Une commande envoyée au BENCH pouvait arriver en plusieurs morceaux et être interprétée comme une
commande inconnue, ou provoquer un timeout côté Python. Le BENCH reconstruit maintenant les lignes
complètes et l'accès concurrent à l'UART du DUT est protégé par un mutex FreeRTOS.

### Contradiction des fins de course

Le driver détectait bien les deux fins de course actives en même temps, mais cette information était
perdue lors de sa conversion en deux booléens. Un champ explicite a été ajouté au snapshot capteur
pour transmettre la contradiction à la FSM, qui peut alors déclencher `SAFE`.

### Verdict global en SIL

Un script terminé sans exception ne suffit pas à valider une campagne : un scénario requis peut être
absent ou `SKIPPED`. Le runner calcule donc le verdict à partir de chaque résultat et renvoie un code
non nul en cas d'échec. Le rapport final reste `INCOMPLETE` lorsqu'une preuve manque.

## Progression des TP

- **TP02 à TP06** — GPIO, NVS, I2C/BME280, HC-SR04, servo et fins de course.
- **TP07 à TP09** — machine à états, tâches FreeRTOS et protocole UART.
- **TP10 à TP11.5** — superviseur Python, contrôleur BENCH et première intégration DUT/BENCH.
- **TP12 à TP14** — injection de fautes, validation SIL/HIL et rapports traçables.
- **TP16 à TP16.5** — campagne HIL, corrections, retests et intégration finale.

Le TP15 de sécurisation est volontairement absent de la version finale : il ne faisait pas partie du
périmètre réellement intégré.

## Validation

- 39 tests Python exécutés : **39 PASS**, 0 échec, 0 erreur.
- 13 projets ESP-IDF compilés avec succès.
- Builds finaux DUT et BENCH : **PASS**.
- Campagne physique HIL conservée dans
  [`tp16_5_final_integration/reports`](tp16_5_final_integration/reports).

Les résultats historiques restent tels qu'ils ont été produits : un `FAIL` ou un `SKIPPED` n'est pas
transformé en succès après coup.

## Technologies

C, Python, ESP-IDF 6.1, FreeRTOS, ESP32, UART, I2C, Git, pytest et PowerShell.

Le matériel principal comprend deux ESP32, deux BME280, un HC-SR04, un servomoteur SG90 et deux fins
de course. Le signal ECHO du HC-SR04 doit être adapté de 5 V à 3,3 V ; le servo utilise une alimentation
5 V séparée avec masse commune.

## Démonstration

Une photo propre du montage final et une courte vidéo de présentation seront ajoutées dans
[`media/`](media/). Deux essais courts déjà versionnés restent consultables :
[`15 s`](docs/videos/essai-banc-01.mp4) et [`8 s`](docs/videos/essai-banc-02.mp4).

## Organisation du dépôt

- [`tp02_gpio_nvs/`](tp02_gpio_nvs/) à [`tp14_reporting_traceability/`](tp14_reporting_traceability/)
  contiennent les étapes de construction et leurs explications.
- [`tp16_hil_campaign/`](tp16_hil_campaign/) conserve la campagne HIL, les anomalies et les retests.
- [`tp16_5_final_integration/`](tp16_5_final_integration/) contient les firmwares DUT/BENCH finaux,
  le superviseur et les preuves de validation.
- [`.github/workflows/sil-tests.yml`](.github/workflows/sil-tests.yml) exécute les tests logiciels en CI.
- [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md) regroupe les notices du code tiers utilisé.

Chaque projet ESP-IDF se compile depuis son propre dossier avec `idf.py build`. Les suites Python se
lancent depuis le dossier du TP concerné avec `python -m pytest -p no:cacheprovider -q`.
