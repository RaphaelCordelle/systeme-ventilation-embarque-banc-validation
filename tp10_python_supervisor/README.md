# TP10 — Superviseur Python

Ce premier superviseur charge des scénarios YAML, envoie une commande au transport série et compare
la réponse reçue. Chaque scénario produit un résultat JSON dans `results/`.

```text
python -m pip install -r requirements.txt
python -m pytest -p no:cacheprovider -q
python main.py --port COM5
```
