import argparse

from reporting.csv_report import save_csv_report
from reporting.json_report import save_json_report
from reporting.html_report import save_html_report
from reporting.traceability import build_traceability
from reporting.metadata import get_metadata

from transports.simulated_transport import SimulatedTransport
from transports.serial_transport import SerialTransport


from scenarios.scenario_status import run as run_status
from scenarios.scenario_open import run as run_open
from scenarios.scenario_close import run as run_close
from scenarios.scenario_telemetry import run as run_telemetry
from scenarios.scenario_reboot import run as run_reboot

from reporting.campaign import (
    create_campaign,
    add_result,
    finish_campaign,
)

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

    campaign = create_campaign(
        args.mode
    )
    campaign["metadata"] = get_metadata()


    transport.connect()

    try:
        print()
        print(
            f"=== TP16 CAMPAIGN - "
            f"{args.mode.upper()} ==="
        )

        for scenario in scenarios:
            result = scenario(
                transport
            )

            add_result(
                campaign,
                result,
            )

            print(result)

    finally:
        transport.close()

    campaign["status"] = "PASS" if campaign_succeeded(campaign["results"]) else "FAIL"
    finish_campaign(
        campaign
    )

    campaign["traceability"] = build_traceability(
    campaign
    )

    report_path = save_json_report(
        campaign
    )
    csv_report_path = save_csv_report(
    campaign
    )
    html_report_path = save_html_report(
    campaign
    )

    print()
    print(
        f"JSON report saved: "
        f"{report_path}"
    )

    print(
        f"CSV report saved: "
        f"{csv_report_path}"
    )
    print(
    f"HTML report saved: "
    f"{html_report_path}"
    )

    return 0 if campaign["status"] == "PASS" else 1



if __name__ == "__main__":
    raise SystemExit(main())
