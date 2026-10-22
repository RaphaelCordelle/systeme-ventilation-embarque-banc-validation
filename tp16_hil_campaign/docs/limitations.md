# Portée de la campagne TP16

Cette campagne correspond à l'étape précédant la [version intégrée](../../tp16_5_final_integration/).

- **Mécanique :** les fins de course OPEN et CLOSED ont été actionnés manuellement pendant les
  essais de mouvement. Le cycle mécanique autonome n'est pas validé par cette campagne.
- **Automatisation :** le transport série Python couvre STATUS, OPEN et CLOSE. La télémétrie et
  la reprise après reset ont été examinées dans les logs du BENCH, sans scénario Python automatisé.
- **Défauts :** perte UART, absence du capteur DUT et défauts des fins de course ont été provoqués
  physiquement. La dérive et le gel des mesures ne sont pas couverts par ces essais matériels.

Le CAN et l'authentification du protocole sont hors du périmètre de cette campagne.
Le [rapport du 10 octobre](../../tp16_5_final_integration/reports/demo_20261010_124952.json)
conserve les résultats de l'intégration suivante.
