def run(transport) -> dict:
    if not transport.capabilities.get(
        "telemetry",
        False,
    ):
        return {
            "name": "TELEMETRY",
            "result": "SKIPPED",
            "reason": "telemetry not supported by this transport",
        }

    telemetry = transport.get_telemetry()
    required_fields = ("temperature", "humidity", "pressure")
    valid = isinstance(telemetry, dict) and all(
        isinstance(telemetry.get(field), (int, float)) for field in required_fields
    )

    return {
        "name": "TELEMETRY",
        "observed": telemetry,
        "result": "PASS" if valid else "FAIL",
    }
