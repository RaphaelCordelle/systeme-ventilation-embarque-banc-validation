# Protocole UART DUT <-> BENCH — TP16.5

La structure de trame historique est conservée :

```text
SOF | VERSION | TYPE | LENGTH | PAYLOAD | CHECK
AA  | 00      | 1 B  | 1 B    | 0..56 B | XOR
```

Le `CHECK` est le XOR de tous les octets précédents de la trame. Il détecte des corruptions simples ; ce n'est pas un mécanisme de sécurité cryptographique.

## Messages

| Type | Nom | Payload requête | Réponse |
|---:|---|---|---|
| `0x01` | HELLO | aucun | RESPONSE vide |
| `0x02` | GET_STATUS | aucun | état sur 1 octet |
| `0x03` | SET_MODE | `0=CLOSE`, `1=OPEN` | valeur acquittée |
| `0x04` | GET_TELEMETRY | aucun | état + T/H/P |
| `0x05` | GET_VERSION | aucun | major/minor/patch |
| `0x06` | SET_AUTO | `0=OFF`, `1=ON` | valeur acquittée |
| `0x07` | GET_DIAGNOSTICS | aucun | snapshot diagnostic |
| `0x80` | RESPONSE | — | réponse normale |
| `0x81` | ERROR | — | code erreur 1 octet |

## GET_TELEMETRY

Payload réponse, 9 octets :

```text
state          u8
T x100         i16 little-endian
H x100         u16 little-endian
pressure Pa    u32 little-endian
```

## GET_DIAGNOSTICS

Payload réponse, 12 octets :

```text
state          u8
T x100         i16 LE
H x100         u16 LE
pressure Pa    u32 LE
distance mm    u16 LE (0xFFFF si invalide)
flags          u8
```

Bits `flags` :

```text
bit 0 : BME280 valide
bit 1 : distance valide
bit 2 : obstacle
bit 3 : fin de course OPEN
bit 4 : fin de course CLOSED
bit 5 : contradiction fins de course
bit 6 : mode AUTO actif
```

Les messages `0x01` à `0x05` restent compatibles avec la baseline TP11.5/TP16. Les deux extensions TP16.5 sont `SET_AUTO` et `GET_DIAGNOSTICS`.
