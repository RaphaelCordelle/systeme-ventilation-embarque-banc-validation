# TP16 — Campagne HIL

Cette étape conserve les scénarios, anomalies, retests et rapports de la campagne DUT/BENCH. Les
résultats historiques restent inchangés : `FAIL` et `SKIPPED` gardent leur signification.

Le générateur `reporting/final_report.py` exige désormais un fichier JSON de résultats. Il produit
un statut `INCOMPLETE` lorsque des preuves manquent et refuse de fabriquer des PASS.

```text
python reporting/final_report.py reports/campaign_hil_20261004_150408.json
python -m pytest -p no:cacheprovider -q
```
