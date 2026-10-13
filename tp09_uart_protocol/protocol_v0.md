# TP09 — Protocole UART v0

## Objectif

Définir un protocole simple permettant à un équipement externe de communiquer avec le DUT par UART.

UART transporte uniquement des octets.
Le protocole définit la signification de ces octets.

## Configuration UART

- Baud rate : 115200
- Data bits : 8
- Parity : None
- Stop bits : 1
- Format : 8N1

## Format général d'une trame

SOF | VERSION | TYPE | LENGTH | PAYLOAD | CHECK

### SOF

Valeur indiquant le début d'une nouvelle trame.

Valeur choisie :

0xAA

### VERSION

Version du protocole.

Version actuelle :

0x00

### TYPE

Identifie le type de message.

Types prévus :

- 0x01 : HELLO
- 0x02 : GET_STATUS
- 0x03 : SET_MODE
- 0x04 : GET_TELEMETRY
- 0x05 : GET_VERSION
- 0x80 : RESPONSE
- 0x81 : ERROR

### LENGTH

Nombre d'octets contenus dans PAYLOAD.

### PAYLOAD

Données spécifiques au message.

La taille dépend du type de message.

### CHECK

Champ permettant de vérifier l'intégrité de la trame.

Pour la version 0, un contrôle simple sera utilisé avant l'ajout éventuel d'un CRC.

## Taille maximale

Une trame ne doit pas dépasser 64 octets.

Une trame dépassant cette taille doit être rejetée.

## Timeout de réception

Si une trame commence mais n'est pas terminée dans le délai prévu, elle doit être abandonnée.

Timeout initial :

500 ms

Le parseur doit ensuite revenir à l'attente d'un nouveau SOF.

## Messages

### HELLO

But :
vérifier que la liaison UART et le protocole fonctionnent.

Requête :

TYPE = 0x01
LENGTH = 0

Réponse :

TYPE = 0x80
LENGTH = 0

### GET_STATUS

But :
demander l'état actuel du DUT.

Requête :

TYPE = 0x02
LENGTH = 0

Réponse possible :

TYPE = 0x80

La réponse contient un octet : la valeur de l'état courant de la machine à états.

### SET_MODE

But :
envoyer une commande de test non dangereuse au DUT.

TYPE = 0x03

Le format exact du payload sera défini pendant l'implémentation.

### GET_TELEMETRY

But :
lire l'état et les mesures du BME280 du DUT.

Requête :

TYPE = 0x04
LENGTH = 0

La réponse contient exactement 9 octets, encodés en little-endian :

| Octets | Valeur | Unité |
|---|---|---|
| 0 | état du DUT (`uint8_t`) | - |
| 1..2 | température x100 (`int16_t`) | °C |
| 3..4 | humidité x100 (`uint16_t`) | % |
| 5..8 | pression (`uint32_t`) | Pa |

Le capteur est raccordé comme dans le TP03 : I2C0, SDA GPIO21,
SCL GPIO22, adresse 0x76. Si la mesure est indisponible, le DUT
renvoie une trame ERROR avec le code 0x05.

### GET_VERSION

But : lire la version du firmware DUT.

Requête :

TYPE = 0x05
LENGTH = 0

La réponse contient trois octets : version majeure, mineure et corrective.

## Erreurs prévues

Le DUT doit détecter au minimum :

- version inconnue ;
- type de message inconnu ;
- longueur invalide ;
- trame trop longue ;
- timeout de réception ;
- trame malformée.

Codes renvoyés par l'implémentation actuelle :

- 0x01 : contrôle XOR invalide ;
- 0x02 : version non prise en charge ;
- 0x03 : longueur de SET_MODE invalide ;
- 0x04 : type de message inconnu ;
- 0x05 : télémétrie indisponible.

Le DUT ne doit pas rester bloqué après une erreur.
Il doit revenir à l'attente d'une nouvelle trame.
