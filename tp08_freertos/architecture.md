# TP08 — Architecture FreeRTOS

## Objectif

Passer d'une logique séquentielle à une architecture multitâche FreeRTOS.

Chaque tâche doit avoir :
- une responsabilité claire ;
- une fréquence ou une condition de réveil ;
- des données dont elle est responsable ;
- un moyen de communiquer avec les autres tâches.

## Tâches prévues

### SensorTask

Responsabilité :
- lire les capteurs ;
- construire un snapshot cohérent des mesures ;
- transmettre ce snapshot à ControlTask.

Données principales :
- distance HC-SR04 ;
- état des fins de course ;
- éventuellement données du capteur I2C.

Fréquence cible :
- environ toutes les 100 ms.

Communication :
- envoi d'un snapshot par queue.

---

### ControlTask

Responsabilité :
- recevoir les snapshots capteurs ;
- exécuter la machine à états du TP07 ;
- prendre les décisions de contrôle ;
- produire des commandes pour l'actionneur.

Fréquence :
- déclenchée principalement par réception d'un nouveau snapshot ou événement.

Communication :
- reçoit les données de SensorTask ;
- envoie des commandes à ActuatorTask.

---

### ActuatorTask

Responsabilité :
- être la seule tâche autorisée à piloter directement le servo ;
- appliquer les commandes OPEN, CLOSE et STOP ;
- gérer l'actionnement matériel.

Communication :
- reçoit les commandes de ControlTask.

---

### SafetyTask

Responsabilité :
- surveiller l'état général du système ;
- détecter certaines situations anormales indépendamment du contrôle principal.

Remarque :
Cette tâche ne sera ajoutée que si elle apporte une vraie surveillance indépendante.

## Ownership des ressources

- SensorTask possède les lectures capteurs.
- ControlTask possède la logique de décision et la machine à états.
- ActuatorTask possède le servo.
- Les données ne doivent pas être modifiées directement par plusieurs tâches.
- Les échanges se feront principalement avec des queues.

## Fréquences initiales

| Tâche | Fréquence / déclenchement |
|---|---|
| SensorTask | Toutes les 100 ms |
| ControlTask | À réception d'un nouveau snapshot |
| ActuatorTask | À réception d'une commande |
| SafetyTask | À définir si nécessaire |

## Principe d'architecture

SensorTask
    |
    | snapshot
    v
ControlTask
    |
    | commande
    v
ActuatorTask


Temps       0 ms      100       200       300       400       ...

SensorTask   X---------X---------X---------X---------X
             snapshot  snapshot  snapshot  snapshot

ControlTask  X----X---------X---------X---------X
             traitement des snapshots / commandes

ActuatorTask      X-----------------------X
                  OPEN                    STOP

Exemple :
0 ms     : commande OPEN
20 ms    : ControlTask traite CMD_OPEN
25 ms    : ActuatorTask reçoit OPEN
1200 ms  : SensorTask détecte LIMIT_OPEN
1220 ms  : ControlTask traite LIMIT_OPEN
1225 ms  : ActuatorTask reçoit STOP