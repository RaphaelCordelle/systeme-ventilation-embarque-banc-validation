REQUIREMENTS = {
    "REQ-STATUS-001": {
        "description": "The DUT shall report a valid system state.",
        "test": "STATUS",
    },

    "REQ-OPEN-001": {
        "description": "The DUT shall reach OPEN state after an open command.",
        "test": "OPEN",
    },

    "REQ-CLOSE-001": {
        "description": "The DUT shall reach CLOSED state after a close command.",
        "test": "CLOSE",
    },

    "REQ-TELEMETRY-001": {
        "description": "The DUT shall provide telemetry data when supported.",
        "test": "TELEMETRY",
    },

    "REQ-REBOOT-001": {
        "description": "The DUT shall recover after a reboot.",
        "test": "REBOOT",
    },
}