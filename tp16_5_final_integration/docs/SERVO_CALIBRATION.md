# Calibration mécanique validée du SG90

La baseline TP16.5 utilise les valeurs suivantes :

```text
OPEN  = 0.9 ms
CLOSE = 2.5 ms
PWM   = 50 Hz
GPIO  = 13
```

Ces valeurs correspondent au montage physique validé. Elles ne doivent pas être modifiées sans une
nouvelle calibration matérielle.

1. Déconnecter mécaniquement le palonnier du volet si le montage peut forcer.
2. Vérifier l'alimentation 5 V dédiée et la masse commune.
3. Tester une commande OPEN puis CLOSE.
4. Vérifier le sens de déplacement.
5. Monter le palonnier de façon à ce que les deux positions atteignent les fins de course sans forcer.
6. Après toute modification mécanique, ajuster uniquement `SERVO_OPEN_PULSE_MS` et
   `SERVO_CLOSE_PULSE_MS` dans `dut_final/main/app_config.h`.
7. Refaire au moins 10 cycles OPEN/CLOSE avant d'accepter une nouvelle calibration.

Le firmware coupe le PWM dès que la position attendue est confirmée et bascule en SAFE si l'extrémité n'est pas atteinte avant 3 secondes.
