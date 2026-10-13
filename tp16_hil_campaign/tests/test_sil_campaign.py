from transports.simulated_transport import SimulatedTransport

from scenarios.scenario_status import run as run_status
from scenarios.scenario_open import run as run_open
from scenarios.scenario_close import run as run_close
from scenarios.scenario_telemetry import run as run_telemetry
from scenarios.scenario_reboot import run as run_reboot


def test_sil_campaign():
    transport = SimulatedTransport()

    transport.connect()

    try:
        scenarios = [
            run_status,
            run_open,
            run_close,
            run_telemetry,
            run_reboot,
        ]
        results = []

        for scenario in scenarios:
            result = scenario(
                transport
            )

            results.append(
                result
            )

    finally:
        transport.close()

    by_name = {result["name"]: result["result"] for result in results}
    for required in ("STATUS", "OPEN", "CLOSE", "TELEMETRY"):
        assert by_name[required] == "PASS"

    assert by_name["REBOOT"] == "SKIPPED"
