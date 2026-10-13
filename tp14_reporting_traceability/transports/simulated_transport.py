from transports.device_transport import DeviceTransport


class SimulatedTransport(DeviceTransport):
    def __init__(self):
        self.connected = False
        self.state = "CLOSED"

        self.capabilities = {
            "status": True,
            "open": True,
            "close": True,
            "telemetry": True,
            "reboot": False,
        }

    def connect(self) -> None:
        self.connected = True

    def close(self) -> None:
        self.connected = False

    def get_status(self) -> dict:
        return {
            "state": self.state
        }

    def get_telemetry(self) -> dict:
        return {
            "temperature": 22.0,
            "humidity": 50.0,
            "pressure": 1013.0,
        }

    def open_actuator(self) -> None:
        self.state = "OPEN"

    def close_actuator(self) -> None:
        self.state = "CLOSED"