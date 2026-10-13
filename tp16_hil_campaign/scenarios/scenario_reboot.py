def run(transport) -> dict:
    if not transport.capabilities.get(
        "reboot",
        False,
    ):
        return {
            "name": "REBOOT",
            "result": "SKIPPED",
            "reason": "reboot not supported by this transport",
        }

    reboot = getattr(transport, "reboot", None)
    if reboot is None:
        return {"name": "REBOOT", "result": "FAIL", "reason": "reboot capability has no implementation"}

    try:
        reboot()
    except Exception as error:
        return {"name": "REBOOT", "result": "FAIL", "reason": str(error)}

    return {"name": "REBOOT", "result": "PASS"}
