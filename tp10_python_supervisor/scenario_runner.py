from models import StepResult


class ScenarioRunner:
    def __init__(self, transport):
        self.transport = transport

    def run(self, scenario: dict) -> StepResult:
        name = scenario.get("name", "scenario")
        command = str(scenario["action"]["command"])
        expected = str(scenario["expect"]["response"])

        try:
            self.transport.send(command.encode("ascii"))
            response = self.transport.receive()
        except Exception as error:
            return StepResult(name, "ERROR", expected, str(error))

        if isinstance(response, bytes):
            actual = response.decode("utf-8", errors="replace").strip()
        else:
            actual = str(response).strip()

        if not actual:
            status = "BLOCKED"
        else:
            status = "PASS" if actual == expected else "FAIL"

        return StepResult(name, status, expected, actual)
