from requirements.requirements import REQUIREMENTS


def build_traceability(
    campaign: dict,
) -> list:
    traceability = []

    for requirement_id, requirement in REQUIREMENTS.items():

        test_name = requirement["test"]

        test_result = None

        for result in campaign["results"]:
            if result.get("name") == test_name:
                test_result = result
                break

        traceability.append(
            {
                "requirement_id": requirement_id,
                "description": requirement["description"],
                "test": test_name,
                "result": (
                    test_result.get("result")
                    if test_result
                    else "NOT_RUN"
                ),
            }
        )

    return traceability