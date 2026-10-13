from html import escape
from pathlib import Path
from datetime import datetime


def save_html_report(
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
        f"{timestamp}.html"
    )

    output_path = reports_path / filename

    test_rows = ""

    for result in campaign["results"]:
        test_rows += f"""
        <tr>
            <td>{escape(str(result.get("name", "")))}</td>
            <td>{escape(str(result.get("expected", "")))}</td>
            <td>{escape(str(result.get("observed", "")))}</td>
            <td>{escape(str(result.get("result", "")))}</td>
            <td>{escape(str(result.get("duration_s", "")))}</td>
            <td>{escape(str(result.get("reason", "")))}</td>
        </tr>
        """

    traceability_rows = ""

    for item in campaign.get(
        "traceability",
        [],
    ):
        traceability_rows += f"""
        <tr>
            <td>{escape(str(item.get("requirement_id", "")))}</td>
            <td>{escape(str(item.get("description", "")))}</td>
            <td>{escape(str(item.get("test", "")))}</td>
            <td>{escape(str(item.get("result", "")))}</td>
        </tr>
        """

    html = f"""
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <title>TP16 Validation Report</title>
    </head>

    <body>
        <h1>TP16 Validation Report</h1>

        <p><strong>Mode:</strong> {escape(str(campaign["mode"]))}</p>
        <p><strong>Started:</strong> {escape(str(campaign["started_at"]))}</p>
        <p><strong>Finished:</strong> {escape(str(campaign["finished_at"]))}</p>
        <h2>Campaign Metadata</h2>

        <p>
            <strong>Python version:</strong>
            {escape(str(campaign.get("metadata", {}).get("python_version", "unknown")))}
        </p>

        <p>
            <strong>Git commit:</strong>
            {escape(str(campaign.get("metadata", {}).get("git_commit", "unknown")))}
        </p>
        <h2>Test Results</h2>

        <table border="1" cellpadding="6">
            <tr>
                <th>Name</th>
                <th>Expected</th>
                <th>Observed</th>
                <th>Result</th>
                <th>Duration (s)</th>
                <th>Reason</th>
            </tr>

            {test_rows}

        </table>

        <h2>Traceability Matrix</h2>

        <table border="1" cellpadding="6">
            <tr>
                <th>Requirement</th>
                <th>Description</th>
                <th>Test</th>
                <th>Result</th>
            </tr>

            {traceability_rows}

        </table>

    </body>
    </html>
    """

    with open(
        output_path,
        "w",
        encoding="utf-8",
    ) as file:
        file.write(html)

    return str(output_path)
