import pytest

from scenario_loader import validate_scenario


def test_valid_scenario():
    scenario = {
        "name": "boot",
        "setup": {
            "mode": "normal"
        },
        "action": {
            "command": "HELLO"
        },
        "expect": {
            "response": "HELLO"
        }
    }

    validate_scenario(scenario)


def test_missing_key():
    scenario = {
        "name": "boot",
        "action": {
            "command": "HELLO"
        },
        "expect": {
            "response": "HELLO"
        }
    }

    with pytest.raises(ValueError):
        validate_scenario(scenario)