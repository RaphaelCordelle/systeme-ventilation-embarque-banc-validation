import time


def run(transport) -> dict:
    if not transport.capabilities.get("close", False):
        return {
            "name": "CLOSE",
            "result": "SKIPPED",
            "reason": "close not supported",
        }

    transport.close_actuator()

    timeout_s = 3.0
    start_time = time.time()
    last_state = "UNKNOWN"

    while time.time() - start_time < timeout_s:
        status = transport.get_status()
        last_state = status["state"]

        if last_state == "CLOSED":
            return {
                "name": "CLOSE",
                "expected": "CLOSED",
                "observed": last_state,
                "result": "PASS",
                "duration_s": round(
                    time.time() - start_time,
                    3,
                ),
            }

        time.sleep(0.2)

    return {
        "name": "CLOSE",
        "expected": "CLOSED",
        "observed": last_state,
        "result": "FAIL",
        "duration_s": round(
            time.time() - start_time,
            3,
        ),
    }