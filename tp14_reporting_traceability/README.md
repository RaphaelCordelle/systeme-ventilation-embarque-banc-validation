# TP14 — Rapports et traçabilité

Ce TP complète la campagne SIL/HIL avec des métadonnées, une matrice de traçabilité et des exports
JSON, CSV et HTML. Les trois formats d'une même campagne utilisent le même identifiant temporel.

```text
python runner.py --mode sil
python -m pytest -p no:cacheprovider -q
```

Les [tests GitHub Actions](../.github/workflows/sil-tests.yml) exécutent les contrôles logiciels
et la campagne SIL.
