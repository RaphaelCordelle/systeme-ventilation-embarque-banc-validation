# TP09 — Protocole UART v0

Le DUT reçoit des trames binaires sur une liaison **115200 bauds, 8N1**.
Cette version teste les échanges et la lecture du BME280 ; la commande du mécanisme est intégrée
dans les étapes suivantes.

## Trame

```text
SOF  | VERSION | TYPE | LENGTH | PAYLOAD  | CHECK
0xAA | 0x00    | 1 o  | 1 o    | 0..56 o  | 1 o
```

`LENGTH` compte les octets du payload. `CHECK` est le XOR de tous les octets précédents,
y compris `SOF`. La taille maximale est donc de 61 octets.
Ce contrôle détecte des corruptions simples et n'authentifie pas les messages.

Le parseur abandonne une trame si la longueur dépasse 56 octets ou si la réception reste
interrompue pendant 500 ms. Il attend ensuite un nouveau `0xAA`.

## Messages

| Type | Requête | Réponse `0x80` |
|---|---|---|
| `0x01` | `HELLO`, sans payload | Payload vide |
| `0x02` | `GET_STATUS`, sans payload | État de la FSM sur 1 octet |
| `0x03` | `SET_MODE`, payload de 1 octet | Octet reçu, renvoyé à l'identique |
| `0x04` | `GET_TELEMETRY`, sans payload | État et mesures sur 9 octets |
| `0x05` | `GET_VERSION`, sans payload | Version majeure, mineure, corrective sur 3 octets |

Dans ce TP, `SET_MODE` est un échange de test : il ne commande pas le servo.

## Télémétrie

| Octets | Valeur | Encodage |
|---|---|---|
| 0 | État du DUT | `uint8_t` |
| 1–2 | Température en °C × 100 | `int16_t`, little-endian |
| 3–4 | Humidité en % × 100 | `uint16_t`, little-endian |
| 5–8 | Pression en Pa | `uint32_t`, little-endian |

Le BME280 utilise I2C0, SDA GPIO21, SCL GPIO22 et l'adresse `0x76`, comme au TP03.

## Erreurs

Les réponses `0x81` contiennent un code sur un octet :

| Code | Signification |
|---|---|
| `0x01` | XOR invalide |
| `0x02` | Version non prise en charge |
| `0x03` | Longueur de `SET_MODE` invalide |
| `0x04` | Type de message inconnu |
| `0x05` | Mesure BME280 indisponible |

Le parseur rejette directement les versions et longueurs invalides pendant la réception.
Les autres contrôles sont réalisés lors du traitement de la trame.

Le [protocole de la version intégrée](../tp16_5_final_integration/docs/PROTOCOL.md) ajoute le
mode automatique et le diagnostic détaillé.
