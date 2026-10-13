import argparse
from pathlib import Path

from serial_transport import SerialTransport
from scenario_loader import load_scenario
from scenario_runner import ScenarioRunner
from result_writer import save_result


ROOT = Path(__file__).resolve().parent


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", default="COM5")
    args = parser.parse_args()

    transport = SerialTransport(args.port)
    failed = False

    try:
        transport.open()

        runner = ScenarioRunner(transport)

        scenario_files = sorted((ROOT / "scenarios").glob("*.yaml"))

        for scenario_path in scenario_files:
            scenario = load_scenario(scenario_path)

            print(f"Execution : {scenario['name']}")

            result = runner.run(scenario)

            print("Status :", result.status)
            print("Expected :", result.expected)
            print("Actual :", result.actual)

            save_result(result, ROOT / "results" / f"{scenario['name']}_result.json")
            failed = failed or result.status != "PASS"

    finally:
        transport.close()

    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
