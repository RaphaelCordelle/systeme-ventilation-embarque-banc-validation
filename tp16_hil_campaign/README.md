# TP16 — Campagne HIL

Cette campagne regroupe les scénarios DUT/BENCH, les [anomalies](anomalies/anomaly_log.csv), les
[retests](retests/retest_log.csv) et les [rapports](reports/). Les résultats gardent le verdict de chaque essai.

Le générateur `reporting/final_report.py` lit un fichier JSON de résultats. Un scénario requis
absent ou non exécuté donne un verdict `INCOMPLETE`.

```text
python reporting/final_report.py reports/campaign_hil_20261004_150408.json
python -m pytest -p no:cacheprovider -q
```
