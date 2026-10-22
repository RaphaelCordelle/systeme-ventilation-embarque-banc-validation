# TP12 — Essais de défauts sur le matériel

J'ai provoqué trois défauts sur les deux ESP32 pour observer leur détection et la reprise des
échanges. Ces essais ont servi à mettre au point le banc avant la campagne d'intégration.

| Essai | Manipulation | Observation |
|---|---|---|
| Perte UART | Déconnexion temporaire de DUT TX vers BENCH RX, cartes alimentées. | Le BENCH signale l'absence de télémétrie valide. Les échanges reprennent après reconnexion. |
| Perte BME280 | Interruption de la communication I2C du capteur DUT. | Le DUT renvoie l'erreur `0x05` ; le BENCH suspend la comparaison. Les mesures reprennent après reconnexion. |
| Redémarrage DUT | Reset du DUT pendant les interrogations du BENCH. | La coupure est détectée, mais la première version du BENCH ne retrouve pas systématiquement la communication. |

Les messages `GET_TELEMETRY invalid length`, `GET_TELEMETRY DUT error: 5` et
`Periodic comparison unavailable` permettent de distinguer ces situations dans le terminal.

## Correction de la reprise UART

Le redémarrage du DUT laissait parfois des octets parasites dans le flux reçu par le BENCH.
La lecture s'arrêtait sur le premier octet inattendu. J'ai modifié la réception pour ignorer ces
octets jusqu'au prochain début de trame `0xAA`, puis reprendre le décodage.

Le suivi des anomalies et des retests se trouve dans la [campagne TP16](../tp16_hil_campaign/).
La [version intégrée](../tp16_5_final_integration/) reprend cette réception UART.
