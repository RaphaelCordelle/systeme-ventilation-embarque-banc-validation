from datetime import datetime


def create_campaign(mode: str) -> dict:
    now = datetime.now()
    return {
        "mode": mode.upper(),
        "started_at": now.isoformat(timespec="seconds"),
        "report_id": now.strftime("%Y%m%d_%H%M%S"),
        "results": [],
    }

def add_result(
    campaign: dict,
    result: dict,
) -> None:
    campaign["results"].append(result)


def finish_campaign(
    campaign: dict,
) -> None:
    campaign["finished_at"] = (
        datetime.now().isoformat(
            timespec="seconds"
        )
    )
