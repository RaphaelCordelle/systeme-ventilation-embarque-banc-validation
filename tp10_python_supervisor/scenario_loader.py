import yaml


def load_scenario(path):
    with open(path, "r", encoding="utf-8") as file:
        scenario = yaml.safe_load(file)

    validate_scenario(scenario)

    return scenario


def validate_scenario(scenario):
    required_keys = ["name", "setup", "action", "expect"]

    for key in required_keys:
        if key not in scenario:
            raise ValueError(f"Cle manquante dans le scenario : {key}")