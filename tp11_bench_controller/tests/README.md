# Vérifications matérielles du TP11

- Flasher le TP09 sur le DUT et le TP11 sur le contrôleur de banc.
- Relier TX GPIO17 du banc au RX du DUT, RX GPIO16 au TX du DUT et les GND.
- Vérifier que chaque carte détecte son BME280 à l'adresse 0x76.
- Confirmer une réponse valide à `GET_STATUS`.
- Confirmer une réponse de 14 octets à `GET_TELEMETRY`, checksum XOR compris.
- Vérifier que les températures, humidités et pressions affichées sont plausibles.
- Laisser fonctionner le banc plusieurs minutes et relever les écarts.
- Relever les écarts entre les deux BME280 ; aucun seuil d'étalonnage n'est fixé dans ce TP.
