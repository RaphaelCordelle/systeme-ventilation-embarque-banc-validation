from demo_supervisor import _sequence_seen, build_checks


def test_sequence_seen_requires_order():
    assert _sequence_seen(
        ["CLOSED", "OPENING", "OPEN", "CLOSING", "CLOSED"],
        ["CLOSED", "OPENING", "OPEN", "CLOSING", "CLOSED"],
    )
    assert not _sequence_seen(
        ["CLOSED", "OPEN", "CLOSING", "CLOSED"],
        ["CLOSED", "OPENING", "OPEN", "CLOSING", "CLOSED"],
    )


def test_checks_pass_complete_normal_cycle():
    states = ["CLOSED", "OPENING", "OPEN", "CLOSING", "CLOSED"]
    samples = [
        {
            "state": state,
            "sensor_valid": True,
            "contradiction": False,
            "auto": True,
        }
        for state in states
    ]
    session = {"samples": samples, "smoke_pass": True}

    checks = build_checks(session, require_cycle=True, fault_demo=False)
    results = {check["name"]: check["result"] for check in checks}

    assert results["SMOKE_TEST"] == "PASS"
    assert results["DUT_SENSOR_VALIDITY"] == "PASS"
    assert results["ENDSTOP_CONSISTENCY"] == "PASS"
    assert results["FULL_OPEN_CLOSE_CYCLE"] == "PASS"
