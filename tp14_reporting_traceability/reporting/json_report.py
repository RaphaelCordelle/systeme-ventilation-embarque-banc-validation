import json
from pathlib import Path
from datetime import datetime


def save_json_report(
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
        f"{timestamp}.json"
    )

    output_path = (
        reports_path / filename
    )

    with open(
        output_path,
        "w",
        encoding="utf-8",
    ) as file:
        json.dump(
            campaign,
            file,
            indent=4,
            ensure_ascii=False,
        )

    return str(output_path)
