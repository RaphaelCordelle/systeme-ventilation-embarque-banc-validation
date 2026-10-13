import json
from dataclasses import asdict
from datetime import datetime
from pathlib import Path


def save_result(result, path):
    output_path = Path(path)
    output_path.parent.mkdir(parents=True, exist_ok=True)

    result_dict = asdict(result)
    result_dict["timestamp"] = datetime.now().isoformat()

    with output_path.open("w", encoding="utf-8") as file:
        json.dump(result_dict, file, indent=4, ensure_ascii=False)
