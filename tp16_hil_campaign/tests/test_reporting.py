import csv
import json
from pathlib import Path

from reporting.campaign import add_result, create_campaign, finish_campaign
from reporting.csv_report import save_csv_report
from reporting.html_report import save_html_report
from reporting.json_report import save_json_report


def test_reports_share_identifier_and_escape_html(tmp_path: Path):
    campaign = create_campaign("sil")
    campaign["metadata"] = {"python_version": "test", "git_commit": "test"}
    add_result(
        campaign,
        {"name": "STATUS", "expected": "CLOSED", "observed": "<CLOSED>", "result": "PASS"},
    )
    finish_campaign(campaign)

    paths = [
        Path(save_json_report(campaign, tmp_path)),
        Path(save_csv_report(campaign, tmp_path)),
        Path(save_html_report(campaign, tmp_path)),
    ]

    assert len({path.stem for path in paths}) == 1
    assert json.loads(paths[0].read_text(encoding="utf-8"))["results"][0]["result"] == "PASS"
    with paths[1].open(encoding="utf-8", newline="") as file:
        assert list(csv.DictReader(file))[0]["observed"] == "<CLOSED>"
    assert "&lt;CLOSED&gt;" in paths[2].read_text(encoding="utf-8")
