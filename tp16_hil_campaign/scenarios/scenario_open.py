import time


def run(transport) -> dict:
    """
    Scenario commun SIL/HIL :
    - demande l'ouverture
    - attend que l'etat OPEN soit atteint
    - echoue si OPEN n'est pas atteint avant le timeout
    """

    transport.open_actuator()

    timeout_s = 3.0
    start_time = time.time()

    last_state = "UNKNOWN"

    while time.time() - start_time < timeout_s:
        status = transport.get_status()
        last_state = status["state"]

        if last_state == "OPEN":
            return {
                "name": "OPEN",
                "expected": "OPEN",
                "observed": last_state,
                "result": "PASS",
                "duration_s": round(
                    time.time() - start_time,
                    3
                ),
            }

        time.sleep(0.2)

    return {
        "name": "OPEN",
        "expected": "OPEN",
        "observed": last_state,
        "result": "FAIL",
        "duration_s": round(
            time.time() - start_time,
            3
        ),
    }