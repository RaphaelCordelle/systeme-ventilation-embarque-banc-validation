CAMPAIGN_NAME = "TP16_HIL_FINAL"

TEST_CASES = [
    {
        "id": "FUNC-001",
        "name": "BOOT",
        "category": "FUNCTIONAL",
        "description": "BENCH and DUT boot correctly.",
    },
    {
        "id": "FUNC-002",
        "name": "STATUS",
        "category": "FUNCTIONAL",
        "description": "DUT returns a valid FSM state.",
    },
    {
        "id": "FUNC-003",
        "name": "OPEN",
        "category": "FUNCTIONAL",
        "description": "DUT transitions from CLOSED to OPEN.",
    },
    {
        "id": "FUNC-004",
        "name": "CLOSE",
        "category": "FUNCTIONAL",
        "description": "DUT transitions from OPEN to CLOSED.",
    },
    {
        "id": "FUNC-005",
        "name": "TELEMETRY",
        "category": "FUNCTIONAL",
        "description": "DUT telemetry is received by the BENCH.",
    },
    {
        "id": "FUNC-006",
        "name": "OPEN_CLOSE_SEQUENCE",
        "category": "FUNCTIONAL",
        "description": "Complete CLOSED -> OPEN -> CLOSED sequence.",
    },
    {
        "id": "FUNC-007",
        "name": "GET_VERSIONS",
        "category": "FUNCTIONAL",
        "description": "BENCH and DUT versions can be retrieved.",
    },
    {
        "id": "FUNC-008",
        "name": "GET_REFERENCE",
        "category": "FUNCTIONAL",
        "description": "BENCH reference BME280 measurement is available.",
    },
    {
        "id": "FUNC-009",
        "name": "SMOKE_TEST",
        "category": "FUNCTIONAL",
        "description": "BENCH autonomous smoke test passes.",
    },

    {
        "id": "FAULT-001",
        "name": "UART_LOSS",
        "category": "FAULT",
        "description": "BENCH detects loss of UART communication.",
    },
    {
        "id": "FAULT-002",
        "name": "UART_RECOVERY",
        "category": "FAULT",
        "description": "Communication recovers after UART reconnection.",
    },
    {
        "id": "FAULT-003",
        "name": "DUT_SENSOR_ABSENT",
        "category": "FAULT",
        "description": "Missing DUT BME280 is detected and telemetry becomes unavailable.",
    },
    {
        "id": "FAULT-004",
        "name": "ENDSTOP_MISSING",
        "category": "FAULT",
        "description": "Missing expected end-stop causes movement timeout and SAFE state.",
    },
    {
        "id": "FAULT-005",
        "name": "ENDSTOP_CONTRADICTION",
        "category": "FAULT",
        "description": "Simultaneous contradictory end-stops cause SAFE state.",
    },
    {
        "id": "FAULT-006",
        "name": "REBOOT_RECOVERY",
        "category": "FAULT",
        "description": "BENCH recovers communication after DUT reboot.",
    },
]

CRITICAL_TESTS = [
    "BOOT",
    "STATUS",
    "OPEN",
    "CLOSE",
    "UART_LOSS",
    "UART_RECOVERY",
    "ENDSTOP_MISSING",
    "ENDSTOP_CONTRADICTION",
    "REBOOT_RECOVERY",
]