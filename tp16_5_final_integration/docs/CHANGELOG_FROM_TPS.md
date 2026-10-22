# Ce que reprend la version intégrée

J'ai réuni les fonctions mises au point séparément dans les TP, puis corrigé les problèmes
apparus lors de leur fonctionnement simultané.

| Étapes | Éléments repris |
|---|---|
| TP02–TP03 | Compteur de démarrages NVS et acquisition BME280. |
| TP04–TP06 | Fins de course, LED, buzzer, HC-SR04 et commande PWM du SG90. |
| TP07–TP09 | Machine à états, tâches FreeRTOS, trames UART et parseur. |
| TP10–TP11.5 | Supervision Python, second ESP32 et comparaison DUT/référence. |
| TP12–TP14 | Essais de défauts, scénarios SIL/HIL et rapports traçables. |
| TP16 | Corrections UART, traitement des contradictions et retests. Les résultats sont conservés dans [`validation_tp16/`](../validation_tp16/). |

## Travail d'intégration

- Une seule tâche lit les capteurs ; l'UART utilise le dernier snapshot sous mutex.
- Les fins de course sont filtrés et leur contradiction est transmise à la machine à états.
- Le mode AUTO utilise une hystérésis ; un obstacle pendant la fermeture déclenche la réouverture.
- Les LED et le buzzer indiquent l'état du système et les défauts.
- `GET_DIAGNOSTICS` et `GET_SNAPSHOT` exposent l'état complet au superviseur.
- L'enregistrement Python produit aussi un classeur XLSX.

Cette version utilise le protocole UART avec contrôle XOR, sans authentification ni protection
contre le rejeu.
