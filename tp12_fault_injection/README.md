# TP12 — Injection de défauts

J'ai testé la réaction du système aux défauts capteur, aux fins de course et aux interruptions UART.
Le [catalogue](fault_catalog.md) décrit les cas étudiés ; les [essais matériels](HIL_TEST.md)
résument les observations et la correction de la reprise UART.

[`sil_faults/`](sil_faults/) contient le modèle Python et ses tests. Les fichiers `fault_injection.c`
et `.h` sont un exercice séparé, hors de la compilation du DUT.
