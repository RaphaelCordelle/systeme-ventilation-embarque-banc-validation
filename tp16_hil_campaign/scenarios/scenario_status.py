def run(transport) -> dict:
    if not transport.capabilities.get("status", False):
        return {
            "name": "STATUS",
            "result": "SKIPPED",
            "reason": "status not supported",
        }

    status = transport.get_status()
    state = status["state"]

    valid_states = [
        "UNKNOWN",
        "CLOSED",
        "OPENING",
        "OPEN",
        "CLOSING",
        "SAFE",
    ]

    return {
        "name": "STATUS",
        "observed": state,
        "result": "PASS" if state in valid_states else "FAIL",
    }