# Démonstration du projet

[**Voir la vidéo — 41 secondes**](../../media/demonstration-volet.mp4)

La vidéo présente le volet, les essais sur la maquette et le suivi des mesures sur le PC.
Les [deux photos du montage et du terminal](../../media/) donnent une vue plus détaillée du matériel.

## Rejouer un essai

Depuis `tp16_5_final_integration/`, après compilation, flash et contrôle du
[câblage](WIRING.md) :

```powershell
python .\host\demo_supervisor.py --port COM5 --duration 90 --require-cycle
```

Adapter le port à celui du BENCH. Faire varier doucement les conditions autour du BME280 DUT
pour observer un cycle `CLOSED → OPENING → OPEN → CLOSING → CLOSED`.
Les seuils du code publié sont 26/25 °C et 70/65 % d'humidité.

Pendant une fermeture, un obstacle à moins de 15 cm du HC-SR04 déclenche la réouverture.
Le superviseur conserve les mesures dans `reports/` et le flux série dans `evidence/`.

Le [rapport du 10 octobre 2026](../reports/demo_20261010_124952.json) décrit la session archivée ;
la vidéo est une présentation du projet, pas une mesure des seuils du code publié.
