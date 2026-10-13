import argparse

from transports.simulated_transport import SimulatedTransport
from transports.serial_transport import SerialTransport

from scenarios.scenario_status import run as run_status
from scenarios.scenario_open import run as run_open
from scenarios.scenario_close import run as run_close
from scenarios.scenario_telemetry import run as run_telemetry
from scenarios.scenario_reboot import run as run_reboot

REQUIRED_SCENARIOS = {"STATUS", "OPEN", "CLOSE", "TELEMETRY"}


def campaign_succeeded(results: list[dict]) -> bool:
    by_name = {result["name"]: result["result"] for result in results}
    required_ok = all(by_name.get(name) == "PASS" for name in REQUIRED_SCENARIOS)
    no_failure = all(result["result"] not in {"FAIL", "ERROR"} for result in results)
    return required_ok and no_failure

def main():
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--mode",
        choices=["sil", "hil"],
        required=True,
    )

    parser.add_argument(
        "--port",
        default="COM5",
    )

    args = parser.parse_args()

    if args.mode == "sil":
        transport = SimulatedTransport()
    else:
        transport = SerialTransport(
            port=args.port
        )

    scenarios = [
        run_status,
        run_open,
        run_close,
        run_telemetry,
        run_reboot,
    ]

    transport.connect()
    results = []

    try:
        print()
        print(
            f"=== TP13 CAMPAIGN - {args.mode.upper()} ==="
        )

        for scenario in scenarios:
            result = scenario(transport)
            results.append(result)
            print(result)

    finally:
        transport.close()

    return 0 if campaign_succeeded(results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
