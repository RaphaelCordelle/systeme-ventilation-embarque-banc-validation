# TP13 — Validation unifiée SIL/HIL

Le même jeu de scénarios utilise soit un transport simulé (`--mode sil`), soit un transport série
(`--mode hil`). STATUS, OPEN, CLOSE et TELEMETRY sont requis ; REBOOT peut être `SKIPPED` si le
transport ne le prend pas en charge.

```text
python runner.py --mode sil
```

Le processus retourne un code non nul si un scénario requis échoue ou n'est pas exécuté.
