import json

from campaign.campaign_config import TEST_CASES
from reporting.final_report import generate_report


def write_results(path, names):
    path.write_text(
        json.dumps({"results": [{"name": name, "result": "PASS"} for name in names]}),
        encoding="utf-8",
    )


def test_report_is_incomplete_when_evidence_is_partial(tmp_path):
    results = tmp_path / "partial.json"
    report = tmp_path / "partial.md"
    write_results(results, ["STATUS", "OPEN", "CLOSE"])

    generate_report(results, report)

    content = report.read_text(encoding="utf-8")
    assert "Campaign status: **INCOMPLETE**" in content
    assert "TELEMETRY | FUNCTIONAL | NOT_RUN" in content


def test_report_accepts_complete_pass_evidence(tmp_path):
    results = tmp_path / "complete.json"
    report = tmp_path / "complete.md"
    write_results(results, [test["name"] for test in TEST_CASES])

    generate_report(results, report)

    content = report.read_text(encoding="utf-8")
    assert "Campaign status: **VALIDATED_WITH_LIMITATIONS**" in content
    assert "**PASS:** 15" in content
