import re
import time
import serial

from transports.device_transport import DeviceTransport


STATE_NAMES = {
    0: "UNKNOWN",
    1: "CLOSED",
    2: "OPENING",
    3: "OPEN",
    4: "CLOSING",
    5: "SAFE",
}


class SerialTransport(DeviceTransport):
    def __init__(self, port: str, baudrate: int = 115200):
        self.port = port
        self.baudrate = baudrate
        self.serial_port = None

        self.capabilities = {
            "status": True,
            "open": True,
            "close": True,
            "telemetry": False,
            "reboot": False,
        }
    def connect(self) -> None:
        self.serial_port = serial.Serial(
            port=self.port,
            baudrate=self.baudrate,
            timeout=0.2,
        )

        time.sleep(0.5)

        self.serial_port.reset_input_buffer()

    def close(self) -> None:
        if self.serial_port is not None:
            self.serial_port.close()

    def _send_command(self, command: str) -> None:
        message = (command + "\n").encode()

        self.serial_port.write(message)

    def get_status(self) -> dict:
        self.serial_port.reset_input_buffer()

        self._send_command(
            "FORWARD_COMMAND GET_STATUS"
        )

        deadline = time.time() + 2.0

        while time.time() < deadline:
            line = (
                self.serial_port
                .readline()
                .decode(errors="ignore")
                .strip()
            )

            match = re.search(
                r"FORWARD_COMMAND GET_STATUS -> state=(\d+)",
                line,
            )

            if match:
                state_number = int(match.group(1))

                state_name = STATE_NAMES.get(
                    state_number,
                    "UNKNOWN",
                )

                return {
                    "state": state_name
                }

        raise TimeoutError(
            "No GET_STATUS response from BENCH"
        )

    def get_telemetry(self) -> dict:
        return {
            "raw": "not implemented yet"
        }

    def open_actuator(self) -> None:
        self._send_command(
            "FORWARD_COMMAND OPEN"
        )

    def close_actuator(self) -> None:
        self._send_command(
            "FORWARD_COMMAND CLOSE"
        )