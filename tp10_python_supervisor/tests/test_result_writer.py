import json

from models import StepResult
from result_writer import save_result


def test_save_result_creates_parent_directory(tmp_path):
    output = tmp_path / "results" / "boot.json"
    save_result(StepResult("boot", "PASS", "HELLO", "HELLO"), output)

    data = json.loads(output.read_text(encoding="utf-8"))
    assert data["status"] == "PASS"
    assert data["expected"] == "HELLO"
    assert data["actual"] == "HELLO"
    assert "timestamp" in data
