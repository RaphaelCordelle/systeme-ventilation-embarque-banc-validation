# Checklist d'acceptation physique TP16.5 — archive

La campagne finale a été exécutée le 10 octobre 2026. Les cases cochées sont directement étayées
par le rapport `demo_20261010_124952` ou nécessaires à son exécution. Les cases non cochées ne sont
pas revendiquées par cette preuve et nécessiteraient une nouvelle campagne matérielle.

La configuration nominale publiée utilise 26/25 °C pour la température et 70/65 % pour
l'humidité. Cette checklist conserve la portée historique de la campagne et son fonctionnement global.

- [x] DUT compile sous ESP-IDF 6.1.
- [x] BENCH compile sous ESP-IDF 6.1.
- [x] DUT flash sur COM7 et boot sans reset en boucle.
- [x] BENCH flash sur COM5 et smoke test PASS.
- [x] Les deux versions 1.0.0 sont retournées par GET_VERSIONS.
- [x] BME280 DUT donne T/H/P plausibles.
- [x] BME280 BENCH donne T/H/P de référence plausibles.
- [x] HC-SR04 donne une distance valide avec diviseur de tension sur ECHO.
- [x] CLOSED physique est observé.
- [x] OPEN physique est observé.
- [x] OPENING/CLOSING sont observés.
- [ ] Les deux fins de course simultanées provoquent SAFE + buzzer.
- [ ] Le servo est alimenté en 5 V séparé avec masse commune.
- [x] AUTO provoque une ouverture pendant la campagne historique.
- [x] Le cycle atteint l'état OPEN.
- [x] AUTO provoque une fermeture pendant la campagne historique.
- [x] Le cycle atteint l'état CLOSED.
- [x] Un obstacle est observé pendant la campagne.
- [ ] Une fin de course manquante pendant 3 s provoque SAFE.
- [ ] La commande manuelle OPEN/CLOSE fonctionne et désactive AUTO.
- [x] AUTO_ON réactive la régulation automatique.
- [x] Le superviseur Python enregistre 120 snapshots sans erreur d'exécution.
- [x] JSON généré.
- [x] CSV généré.
- [x] HTML généré.
- [x] XLSX généré.
- [x] Log série brut généré.
- [x] `cd host ; python -m pytest -p no:cacheprovider -q` passe.
