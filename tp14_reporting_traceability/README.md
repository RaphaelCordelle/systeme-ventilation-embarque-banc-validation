# TP14 — Rapports et traçabilité

Ce TP complète la campagne SIL/HIL avec des métadonnées, une matrice de traçabilité et des exports
JSON, CSV et HTML. Les trois formats d'une même campagne utilisent le même identifiant temporel.

```text
python runner.py --mode sil
python -m pytest -p no:cacheprovider -q
```

La CI publique se trouve dans `.github/workflows/sil-tests.yml` à la racine du dépôt.
