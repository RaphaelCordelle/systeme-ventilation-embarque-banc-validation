# Procédure vidéo de démonstration

Objectif : montrer en 1 à 2 minutes que le projet n'est pas seulement du code, mais un système embarqué complet avec capteurs, actionneur, retour physique et banc de validation.

## Préparation

1. Monter mécaniquement le volet pour que le mouvement du SG90 déclenche réellement les deux fins de course.
2. Positionner le volet en CLOSED avant le boot.
3. Vérifier que l'obstacle HC-SR04 peut être placé à moins de 15 cm sans toucher le mécanisme.
4. Flasher DUT puis BENCH.
5. Placer le terminal Python dans la zone filmée.
6. Lancer :

```powershell
python .\host\demo_supervisor.py --port COM5 --duration 90 --require-cycle
```

## Séquence recommandée

### 1. Boot / preuve du banc

La vidéo montre brièvement les deux ESP32 et le mécanisme. Le superviseur doit afficher :

```text
Versions: BENCH 1.0.0 | DUT 1.0.0
Smoke test: PASS
Control: AUTO ON
```

Le volet démarre CLOSED : LED rouge.

### 2. Ouverture automatique par température

Réchauffer doucement le BME280 DUT jusqu'au seuil nominal d'ouverture de 26 °C. Ne pas chauffer
directement avec une source très chaude.

Attendu :

```text
CLOSED -> OPENING -> OPEN
```

Le servo entraîne réellement le volet et le mécanisme appuie sur le fin de course OPEN. La LED passe jaune pendant le mouvement puis verte à l'ouverture.

### 3. Fermeture automatique

Quand la température revient au seuil nominal de fermeture de 25 °C, le DUT déclenche :

```text
OPEN -> CLOSING -> CLOSED
```

Le fin de course CLOSED doit être déclenché par le volet lui-même.

La configuration nominale publiée utilise 26/25 °C et 70/65 %. La vidéo archivée reste une preuve
historique du fonctionnement global DUT/BENCH, du mouvement, des capteurs, des fins de course,
du mode AUTO et de la supervision.

### 4. Obstacle

Pendant une fermeture, présenter un objet à moins de 15 cm du HC-SR04.

Attendu :

```text
CLOSING -> obstacle -> OPENING -> OPEN
```

Le volet doit cesser la fermeture et se rouvrir.

### 5. Alarme / SAFE facultatif

Pour une seconde prise vidéo, lancer une session dédiée :

```powershell
python .\host\demo_supervisor.py --port COM5 --duration 30 --fault-demo
```

Puis activer simultanément les deux fins de course de façon sûre.

Attendu :

```text
SAFE
LED rouge + jaune
buzzer actif
```

SAFE reste verrouillé jusqu'au reboot.

### 6. Preuve finale

Arrêter le script avec `Ctrl+C` si nécessaire. Il génère dans `reports/` :

```text
.json
.csv
.html
.xlsx
```

et conserve le flux série dans `evidence/`.
