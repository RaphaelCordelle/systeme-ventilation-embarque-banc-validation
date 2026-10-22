# Validation matérielle du 10 octobre 2026

La session `demo_20261010_124952` a enregistré **120 mesures** sur les firmwares DUT et BENCH
**1.0.0**, avec un verdict global **PASS**.

| Contrôle du rapport | Résultat | Observation |
|---|---|---|
| `SMOKE_TEST` | PASS | Capteur de référence et communication avec le DUT disponibles. |
| `DUT_SENSOR_VALIDITY` | PASS | 120 mesures capteur valides sur 120. |
| `ENDSTOP_CONSISTENCY` | PASS | Aucun échantillon avec les deux fins de course actifs. |
| `AUTO_MODE_OBSERVED` | PASS | Mode AUTO présent dans le diagnostic du DUT. |
| `FULL_OPEN_CLOSE_CYCLE` | PASS | États CLOSED, OPENING, OPEN et CLOSING enregistrés. |
| `SAFE_STATE` | NOT_OBSERVED | Aucun passage en SAFE pendant cette session normale. |

Des obstacles sont également présents dans les mesures. Le rapport ne vérifie pas à lui seul
toutes les réactions de sécurité : contradiction forcée, fin de course manquante ou défaut capteur
prolongé. Il ne mesure pas non plus les seuils de déclenchement exacts.

Les seuils du code publié sont **26/25 °C** et **70/65 % d'humidité**.
Les contrôles électriques et la calibration du servo sont décrits dans le [câblage](WIRING.md)
et la [fiche de calibration](SERVO_CALIBRATION.md).

## Fichiers de la session

- [Rapport JSON et contrôles détaillés](../reports/demo_20261010_124952.json)
- [Mesures CSV](../reports/demo_20261010_124952.csv)
- [Rapport HTML](../reports/demo_20261010_124952.html)
- [Classeur Excel](../reports/demo_20261010_124952.xlsx)
- [Journal série brut](../evidence/demo_serial_20261010_124952.log)
