# TP08 — Architecture FreeRTOS

J'ai séparé l'acquisition, la décision et l'actionnement pour éviter qu'une seule boucle gère tout.
Les tâches échangent des copies de données dans des files FreeRTOS.

```text
SensorTask → snapshot → ControlTask → commande → ActuatorTask
                            ↑
                     commandes utilisateur
```

| Tâche | Responsabilité | Déclenchement |
|---|---|---|
| `SensorTask` | Construit un snapshot horodaté des capteurs. | Toutes les 100 ms |
| `ControlTask` | Applique la machine à états, traite les commandes, surveille les fins de course et le timeout. | À réception d'un snapshot |
| `ActuatorTask` | Reçoit `OPEN`, `CLOSE` ou `STOP`. | À réception d'une commande |

Les files capteur et actionneur conservent la dernière valeur avec `xQueueOverwrite`.
La machine à états appartient à `ControlTask` ; aucune autre tâche ne prend de décision de mouvement.
Le timeout de mouvement est fixé à 3 secondes.

À cette étape, les mesures sont factices et les actions du servo sont écrites dans les logs.
Le contrôle des défauts fait partie de `ControlTask`. L'acquisition et l'actionnement physiques
sont réunis dans la [version intégrée](../tp16_5_final_integration/).
