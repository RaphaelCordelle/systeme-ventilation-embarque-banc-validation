TEST_READINESS = {
    "BOOT": {
        "status": "TO_IMPLEMENT",
        "note": "Boot visible dans les logs, mais pas encore de scenario Python dedie."
    },

    "STATUS": {
        "status": "READY",
        "note": "GET_STATUS fonctionne deja en HIL."
    },

    "OPEN": {
        "status": "MANUAL_READY",
        "note": "Commande automatisee, fin de course OPEN encore actionnee manuellement."
    },

    "CLOSE": {
        "status": "MANUAL_READY",
        "note": "Commande automatisee, fin de course CLOSED encore actionnee manuellement."
    },

    "TELEMETRY": {
        "status": "TO_IMPLEMENT",
        "note": "Le BENCH lit la telemetrie, mais SerialTransport.get_telemetry() HIL n'est pas encore implemente."
    },

    "OPEN_CLOSE_SEQUENCE": {
        "status": "TO_IMPLEMENT",
        "note": "OPEN et CLOSE existent separement, sequence complete a creer."
    },

    "UART_LOSS": {
        "status": "MANUAL_READY",
        "note": "Defaut deja teste auparavant, injection par debranchement UART."
    },

    "DUT_SENSOR_ABSENT": {
        "status": "MANUAL_READY",
        "note": "Injection possible en debranchant le BME280 DUT."
    },

    "SENSOR_FROZEN": {
        "status": "TO_IMPLEMENT",
        "note": "Pas encore de mecanisme propre pour figer artificiellement une mesure."
    },

    "SENSOR_DRIFT": {
        "status": "TO_IMPLEMENT",
        "note": "Pas encore de mecanisme d'injection de derive."
    },

    "INVALID_DISTANCE": {
        "status": "TO_VERIFY",
        "note": "Injection possible, mais comportement attendu a verifier avant campagne."
    },

    "SERVO_BLOCKED": {
        "status": "TO_VERIFY",
        "note": "A tester apres reintegration complete du mecanisme servo/volet."
    },

    "ENDSTOP_MISSING": {
        "status": "MANUAL_READY",
        "note": "Ne pas actionner le fin de course attendu et observer le timeout."
    },

    "ENDSTOP_CONTRADICTION": {
        "status": "TO_VERIFY",
        "note": "Scenario deja tente mais comportement HIL pas encore prouve proprement."
    },

    "REBOOT_RECOVERY": {
        "status": "MANUAL_READY",
        "note": "Recovery UART deja observee, scenario final a formaliser."
    },
}


def print_readiness():
    for name, info in TEST_READINESS.items():
        print(
            f"{name:22} "
            f"{info['status']:14} "
            f"{info['note']}"
        )


if __name__ == "__main__":
    print_readiness()