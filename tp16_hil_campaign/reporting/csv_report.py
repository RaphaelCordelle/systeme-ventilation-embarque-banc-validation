import csv
from pathlib import Path
from datetime import datetime


def save_csv_report(
    campaign: dict,
    reports_dir: str = "reports",
) -> str:
    reports_path = Path(reports_dir)

    reports_path.mkdir(
        parents=True,
        exist_ok=True,
    )

    timestamp = campaign.get("report_id", datetime.now().strftime("%Y%m%d_%H%M%S"))

    filename = (
        f"campaign_"
        f"{campaign['mode'].lower()}_"
        f"{timestamp}.csv"
    )

    output_path = reports_path / filename

    with open(
        output_path,
        "w",
        newline="",
        encoding="utf-8",
    ) as file:

        fieldnames = [
            "mode",
            "name",
            "expected",
            "observed",
            "result",
            "duration_s",
            "reason",
        ]

        writer = csv.DictWriter(
            file,
            fieldnames=fieldnames,
        )

        writer.writeheader()

        for result in campaign["results"]:
            writer.writerow(
                {
                    "mode": campaign["mode"],
                    "name": result.get("name", ""),
                    "expected": result.get("expected", ""),
                    "observed": result.get("observed", ""),
                    "result": result.get("result", ""),
                    "duration_s": result.get("duration_s", ""),
                    "reason": result.get("reason", ""),
                }
            )

    return str(output_path)
