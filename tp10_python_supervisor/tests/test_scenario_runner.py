from scenario_runner import ScenarioRunner


class FakeTransport:
    def __init__(self, response):
        self.response = response
        self.sent_data = None

    def send(self, data):
        self.sent_data = data

    def receive(self):
        return self.response


def test_scenario_pass():
    transport = FakeTransport(b"HELLO")

    runner = ScenarioRunner(transport)

    scenario = {
        "action": {
            "command": "HELLO"
        },
        "expect": {
            "response": "HELLO"
        }
    }

    result = runner.run(scenario)

    assert result.status == "PASS"
    assert result.expected == "HELLO"
    assert result.actual == "HELLO"
    assert transport.sent_data == b"HELLO"


def test_scenario_fail():
    transport = FakeTransport(b"ERROR")

    runner = ScenarioRunner(transport)

    scenario = {
        "action": {
            "command": "HELLO"
        },
        "expect": {
            "response": "HELLO"
        }
    }

    result = runner.run(scenario)

    assert result.status == "FAIL"
    assert result.expected == "HELLO"
    assert result.actual == "ERROR"


def test_scenario_blocked():
    transport = FakeTransport(b"")

    runner = ScenarioRunner(transport)

    scenario = {
        "action": {
            "command": "HELLO"
        },
        "expect": {
            "response": "HELLO"
        }
    }

    result = runner.run(scenario)

    assert result.status == "BLOCKED"


class ErrorTransport:
    def send(self, data):
        raise RuntimeError("Port serie indisponible")

    def receive(self):
        return b""


def test_scenario_error():
    transport = ErrorTransport()

    runner = ScenarioRunner(transport)

    scenario = {
        "action": {
            "command": "HELLO"
        },
        "expect": {
            "response": "HELLO"
        }
    }

    result = runner.run(scenario)

    assert result.status == "ERROR"
