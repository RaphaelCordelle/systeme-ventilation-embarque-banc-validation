# TP16 — Synthèse des essais du 4 octobre 2026

Cette étape porte sur les commandes entre les deux ESP32, les retours des fins de course et
la reprise des échanges après un défaut. Les fins de course étaient actionnés manuellement
pendant les essais de mouvement.

## Campagnes automatisées enregistrées

| Scénario | Essai de 15:03 | Essai de 15:04 |
|---|---|---|
| STATUS | PASS | PASS |
| OPEN | FAIL | PASS |
| CLOSE | FAIL | PASS |
| TELEMETRY | SKIPPED | SKIPPED |
| REBOOT | SKIPPED | SKIPPED |

Les commandes OPEN et CLOSE échouent lors du premier essai, puis atteignent les états attendus
lors du second. La télémétrie et le redémarrage ne sont pas pris en charge par le transport
Python de cette étape ; ils restent marqués `SKIPPED`.

Sources : [premier essai](./campaign_hil_20261004_150321.json) et
[second essai](./campaign_hil_20261004_150408.json).
Ces enregistrements ne constituent pas une validation automatique de tous les scénarios TP16.

## Corrections et retests

J'ai corrigé la reconstruction des commandes PC fragmentées et protégé l'accès à l'UART du DUT
par un mutex. J'ai aussi ajouté un champ explicite au snapshot pour transmettre la contradiction
des fins de course à la machine à états.

Le [suivi des anomalies](../anomalies/anomaly_log.csv) décrit ces deux corrections.
Le [journal de retest](../retests/retest_log.csv) contient sept contrôles marqués `PASS` : lecture d'état,
ouverture, fermeture, contradiction des fins de course et reprise après reset.

Les [limites de cette campagne](../docs/limitations.md) précisent la portée des essais.
La [session de la version intégrée du 10 octobre 2026](../../tp16_5_final_integration/reports/demo_20261010_124952.json) contient les résultats
ultérieurs du montage complet : 120 mesures et un cycle d'ouverture/fermeture observé.
