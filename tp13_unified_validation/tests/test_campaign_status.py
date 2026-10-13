from runner import campaign_succeeded


def test_required_scenarios_must_pass():
    results = [
        {"name": "STATUS", "result": "PASS"},
        {"name": "OPEN", "result": "PASS"},
        {"name": "CLOSE", "result": "PASS"},
        {"name": "TELEMETRY", "result": "PASS"},
        {"name": "REBOOT", "result": "SKIPPED"},
    ]
    assert campaign_succeeded(results)

    results[3]["result"] = "SKIPPED"
    assert not campaign_succeeded(results)


def test_any_explicit_failure_fails_campaign():
    results = [
        {"name": "STATUS", "result": "PASS"},
        {"name": "OPEN", "result": "PASS"},
        {"name": "CLOSE", "result": "PASS"},
        {"name": "TELEMETRY", "result": "PASS"},
        {"name": "REBOOT", "result": "FAIL"},
    ]
    assert not campaign_succeeded(results)
