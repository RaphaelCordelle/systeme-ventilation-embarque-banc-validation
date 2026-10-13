# Commandes BENCH disponibles sur USB série

Toutes les commandes sont terminées par un retour ligne.

| Commande | Effet |
|---|---|
| `GET_VERSIONS` | affiche les versions BENCH et DUT |
| `GET_REFERENCE` | lit le BME280 de référence |
| `GET_DUT_TELEMETRY` | retourne état + T/H/P du DUT |
| `GET_DIAGNOSTICS` | retourne T/H/P + distance + obstacle + fins de course + AUTO |
| `GET_SNAPSHOT` | combine diagnostic DUT, BME280 référence et écarts |
| `AUTO_ON` | active la régulation automatique |
| `AUTO_OFF` | désactive la régulation automatique |
| `RUN_SMOKE` | relance le smoke test BENCH |
| `FORWARD_COMMAND GET_STATUS` | lit l'état DUT |
| `FORWARD_COMMAND OPEN` | ouverture manuelle ; désactive AUTO |
| `FORWARD_COMMAND CLOSE` | fermeture manuelle ; désactive AUTO |

## Exemple de snapshot

```text
SNAPSHOT state=CLOSED auto=1 sensor_valid=1 distance_valid=1 obstacle=0 T=24.20 H=51.00 P=1008.40 D=42.1 open=0 closed=1 contradiction=0 ref_T=24.10 ref_H=50.80 ref_P=1008.20 dT=0.10 dH=0.20 dP=0.20
```

Cette ligne est volontairement stable et facilement analysable par Python. Les logs humains détaillés restent disponibles en parallèle.
