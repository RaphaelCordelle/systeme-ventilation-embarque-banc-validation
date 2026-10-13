# HIL Tests - Fault Injection

Cette phase a pour objectif de vérifier le comportement du système complet sur le matériel réel.

Contrairement aux tests SIL réalisés en Python, les tests HIL utilisent directement :

- le DUT ESP32 ;
- le BENCH ESP32 ;
- les capteurs réels ;
- la liaison UART ;
- les tâches FreeRTOS ;
- la machine à états du DUT.

Le principe général est :

Faute injectée
→ détection de l'anomalie
→ réaction du système
→ observation dans les logs
→ retour au fonctionnement normal.

---

## HIL-01 - Perte de communication UART

### Injection

La liaison UART de retour entre le DUT et le BENCH est volontairement interrompue.

La connexion suivante est coupée temporairement :

DUT TX → BENCH RX

Les deux ESP32 restent alimentés et continuent à fonctionner.

### Résultat

Le BENCH ne reçoit plus de télémétrie valide et affiche notamment :

GET_TELEMETRY invalid length

Periodic comparison unavailable

Le DUT continue cependant à fonctionner.

Après rétablissement de la liaison UART, les échanges reprennent normalement.

### Conclusion

La perte de communication est correctement détectée sans provoquer de blocage du système.

Résultat : VALIDÉ

---

## HIL-02 - Perte du capteur BME280 du DUT

### Injection

La communication I2C du BME280 du DUT est interrompue volontairement.

Le DUT reste alimenté et les autres fonctions continuent de tourner.

### Résultat

Le DUT détecte l'échec de lecture du capteur et renvoie une erreur au BENCH.

Le BENCH affiche notamment :

GET_TELEMETRY DUT error: 5

Periodic comparison unavailable

Aucune fausse mesure n'est utilisée comme une mesure valide.

Après restauration de la liaison avec le capteur, la télémétrie reprend.

### Conclusion

La perte du capteur est correctement détectée et le reste du système continue de fonctionner.

Résultat : VALIDÉ

---

## HIL-03 - Reboot du DUT

### Injection

Le DUT est redémarré brutalement pendant que le BENCH continue à l'interroger.

### Résultat attendu

Le comportement souhaité est :

communication normale
→ reboot DUT
→ perte temporaire de communication
→ redémarrage du DUT
→ resynchronisation UART
→ reprise automatique de la télémétrie.

### Résultat observé

La perte de communication est correctement détectée par le BENCH :

GET_TELEMETRY invalid length

Periodic comparison unavailable

Le DUT redémarre correctement.

Cependant, la reprise de communication n'est pas encore systématiquement automatique après le premier redémarrage.

Un nouveau reset permet actuellement de retrouver la communication.

### Amélioration prévue

La gestion de récupération UART sera améliorée afin de rendre la reconnexion automatique après un redémarrage du DUT.

Les pistes prévues sont :

- réinitialiser le parser UART après un timeout ;
- vider les données résiduelles présentes dans le buffer UART ;
- remettre explicitement le parser dans son état d'attente du SOF ;
- tester automatiquement la reprise de communication après reboot.

Résultat : DÉTECTION VALIDÉE / RÉCUPÉRATION À AMÉLIORER

---

## Bilan

Les essais HIL montrent que le banc est capable de détecter plusieurs défauts réels :

- perte de communication UART ;
- perte d'un capteur I2C ;
- redémarrage du DUT.

Les deux premiers scénarios récupèrent correctement après retour à la normale.

Le test de reboot a permis d'identifier un défaut réel de robustesse concernant la resynchronisation de la communication.

Cette anomalie sera utilisée comme cas de test de non-régression lors de l'amélioration du protocole.

J’ai corrigé le problème de redémarrage du DUT en modifiant la réception UART du BENCH. Avant, si le BENCH recevait un octet incorrect après le reset, il arrêtait directement la lecture. J’ai donc fait en sorte qu’il ignore les octets invalides jusqu’à retrouver le début d’une trame valide (0xAA). Comme ça, après le redémarrage du DUT, la communication peut se resynchroniser automatiquement.