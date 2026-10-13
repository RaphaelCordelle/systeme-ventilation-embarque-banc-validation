import argparse
from pathlib import Path
import csv
import json
import sys
from datetime import datetime


ROOT = Path(__file__).resolve().parent.parent

if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))


from campaign.campaign_config import TEST_CASES


REPORTS_DIR = ROOT / "reports"
ANOMALY_FILE = ROOT / "anomalies" / "anomaly_log.csv"
RETEST_FILE = ROOT / "retests" / "retest_log.csv"
RELEASE_FILE = ROOT / "release" / "release_candidate.json"
LIMITATIONS_FILE = ROOT / "docs" / "limitations.md"
VALID_RESULTS = {"PASS", "FAIL", "ERROR", "SKIPPED", "NOT_RUN"}


def read_csv(path):
    if not path.exists():
        return []

    with path.open(
        "r",
        encoding="utf-8",
        newline=""
    ) as file:
        return list(
            csv.DictReader(file)
        )


def read_json(path):
    with path.open(
        "r",
        encoding="utf-8"
    ) as file:
        return json.load(file)


def load_results(path: Path) -> dict[str, str]:
    data = read_json(path)
    results = data.get("results")
    if not isinstance(results, list) or not results:
        raise ValueError("Le fichier de résultats ne contient aucune preuve exploitable.")

    by_name = {}
    for item in results:
        name = item.get("name")
        result = item.get("result")
        if not name or result not in VALID_RESULTS:
            raise ValueError("Chaque résultat doit contenir un nom et un statut valide.")
        by_name[name] = result

    return by_name


def markdown_cell(value) -> str:
    return str(value).replace("|", "\\|").replace("\n", " ")


def generate_report(results_path, output_path=None):
    REPORTS_DIR.mkdir(parents=True, exist_ok=True)

    anomalies = read_csv(
        ANOMALY_FILE
    )

    retests = read_csv(
        RETEST_FILE
    )

    release = read_json(
        RELEASE_FILE
    )
    final_results = load_results(Path(results_path))

    timestamp = datetime.now().strftime(
        "%Y%m%d_%H%M%S"
    )

    report_path = Path(output_path) if output_path else REPORTS_DIR / f"TP16_final_validation_{timestamp}.md"
    report_path.parent.mkdir(parents=True, exist_ok=True)


    lines = []

    lines.append(
        "# TP16 - Final HIL Validation Report\n"
    )

    lines.append(
        f"Generated: {datetime.now().isoformat(timespec='seconds')}\n"
    )

    lines.append(
        "## Release\n"
    )

    lines.append(
        f"- Status: **{release['status']}**"
    )

    lines.append(
        f"- BENCH: {release['bench']['project']} "
        f"v{release['bench']['version']}"
    )

    lines.append(
        f"- DUT: {release['dut']['project']} "
        f"v{release['dut']['version']}"
    )

    lines.append(
        f"- ESP-IDF: {release['environment']['esp_idf']}"
    )

    lines.append(
        ""
    )


    lines.append(
        "## Campaign results\n"
    )

    lines.append(
        "| ID | Scenario | Category | Result |"
    )

    lines.append(
        "|---|---|---|---|"
    )


    pass_count = 0

    for test in TEST_CASES:
        result = final_results.get(
            test["name"],
            "NOT_RUN"
        )

        if result == "PASS":
            pass_count += 1

        lines.append(
            f"| {test['id']} | "
            f"{test['name']} | "
            f"{test['category']} | "
            f"{result} |"
        )


    lines.append(
        ""
    )

    executed_count = sum(
        final_results.get(test["name"], "NOT_RUN") not in {"NOT_RUN", "SKIPPED"}
        for test in TEST_CASES
    )
    lines.append(f"**Executed:** {executed_count}")

    lines.append(
        f"**PASS:** {pass_count}"
    )

    lines.append(
        f"**Other:** {len(TEST_CASES) - pass_count}"
    )

    lines.append(
        ""
    )


    lines.append(
        "## Anomalies\n"
    )

    if anomalies:
        lines.append(
            "| ID | Classification | Status | Symptom | Root cause |"
        )

        lines.append(
            "|---|---|---|---|---|"
        )

        for anomaly in anomalies:
            lines.append(
                f"| {markdown_cell(anomaly['anomaly_id'])} | "
                f"{markdown_cell(anomaly['classification'])} | "
                f"{markdown_cell(anomaly['status'])} | "
                f"{markdown_cell(anomaly['symptom'])} | "
                f"{markdown_cell(anomaly['root_cause'])} |"
            )
    else:
        lines.append(
            "No anomaly recorded."
        )


    lines.append(
        ""
    )

    lines.append(
        "## Retests and non-regression\n"
    )

    if retests:
        lines.append(
            "| Retest | Anomaly | Test | Result |"
        )

        lines.append(
            "|---|---|---|---|"
        )

        for retest in retests:
            lines.append(
                f"| {markdown_cell(retest['retest_id'])} | "
                f"{markdown_cell(retest['anomaly_id'])} | "
                f"{markdown_cell(retest['test'])} | "
                f"{markdown_cell(retest['result'])} |"
            )
    else:
        lines.append(
            "No retest recorded."
        )


    lines.append(
        ""
    )

    lines.append(
        "## Conclusion\n"
    )

    statuses = [final_results.get(test["name"], "NOT_RUN") for test in TEST_CASES]
    if all(status == "PASS" for status in statuses):
        campaign_status = release["status"]
        conclusion = "Tous les scénarios définis disposent d'une preuve PASS dans le fichier fourni."
    elif any(status in {"FAIL", "ERROR"} for status in statuses):
        campaign_status = "FAILED"
        conclusion = "Au moins un scénario a échoué. Le rapport ne conclut pas à une validation."
    else:
        campaign_status = "INCOMPLETE"
        conclusion = "Des scénarios sont absents ou non exécutés. Le rapport reste incomplet."

    lines.append(conclusion)

    lines.append(
        ""
    )

    lines.append(
        "## Validation"
    )

    lines.append(
        ""
    )

    lines.append(
        f"Campaign status: **{campaign_status}**"
    )

    lines.append(
        ""
    )

    lines.append(f"Evidence source: `{Path(results_path).name}`")


    report_path.write_text(
        "\n".join(lines),
        encoding="utf-8"
    )

    print(f"Final report generated: {report_path}")
    return report_path


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Génère un rapport TP16 à partir de résultats existants.")
    parser.add_argument("results_file", help="Fichier JSON contenant une liste 'results'.")
    parser.add_argument("--output", help="Chemin du rapport Markdown à produire.")
    arguments = parser.parse_args()
    generate_report(arguments.results_file, arguments.output)
