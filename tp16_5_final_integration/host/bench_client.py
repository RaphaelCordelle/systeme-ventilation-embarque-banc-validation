from __future__ import annotations

import re
import time
from dataclasses import dataclass, field
from typing import Callable

try:
    import serial
except ImportError:  # allows parser/report tests without hardware dependencies
    serial = None

ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")


def strip_ansi(text: str) -> str:
    return ANSI_RE.sub("", text)


def parse_snapshot_line(line: str) -> dict:
    """Parse the machine-readable part of a BENCH 'SNAPSHOT ...' log line."""
    clean = strip_ansi(line).strip()
    marker = "SNAPSHOT "
    index = clean.find(marker)
    if index < 0:
        raise ValueError(f"No SNAPSHOT payload in line: {line!r}")

    payload = clean[index + len(marker):]
    values: dict[str, object] = {}

    for token in payload.split():
        if "=" not in token:
            continue
        key, raw = token.split("=", 1)
        raw = raw.strip().strip(",")
        values[key] = raw

    required = {
        "state",
        "auto",
        "sensor_valid",
        "distance_valid",
        "obstacle",
        "T",
        "H",
        "P",
        "D",
        "open",
        "closed",
        "contradiction",
    }

    missing = required - values.keys()
    if missing:
        raise ValueError(f"Missing SNAPSHOT fields: {sorted(missing)}")

    bool_fields = {
        "auto",
        "sensor_valid",
        "distance_valid",
        "obstacle",
        "open",
        "closed",
        "contradiction",
        "ref_valid",
    }
    float_fields = {
        "T",
        "H",
        "P",
        "D",
        "ref_T",
        "ref_H",
        "ref_P",
        "dT",
        "dH",
        "dP",
    }

    parsed: dict[str, object] = {"state": str(values["state"])}

    for key, value in values.items():
        if key == "state":
            continue
        if key in bool_fields:
            parsed[key] = str(value) == "1"
        elif key in float_fields:
            parsed[key] = float(str(value))
        else:
            parsed[key] = value

    if "ref_valid" not in parsed:
        parsed["ref_valid"] = "ref_T" in parsed

    return parsed


@dataclass
class BenchClient:
    port: str
    baudrate: int = 115200
    timeout_s: float = 0.2
    serial_port: object | None = None
    raw_lines: list[str] = field(default_factory=list)

    def connect(self) -> None:
        if serial is None:
            raise RuntimeError("pyserial is required. Run: pip install -r host/requirements.txt")

        self.serial_port = serial.Serial(
            port=self.port,
            baudrate=self.baudrate,
            timeout=self.timeout_s,
        )
        time.sleep(0.6)
        self.serial_port.reset_input_buffer()

    def close(self) -> None:
        if self.serial_port is not None and self.serial_port.is_open:
            self.serial_port.close()

    def _write(self, command: str) -> None:
        if self.serial_port is None:
            raise RuntimeError("BenchClient is not connected")
        self.serial_port.write((command + "\n").encode("ascii"))
        self.serial_port.flush()

    def _readline(self) -> str:
        if self.serial_port is None:
            raise RuntimeError("BenchClient is not connected")

        raw = self.serial_port.readline()
        if not raw:
            return ""

        line = strip_ansi(raw.decode(errors="ignore")).strip()
        if line:
            self.raw_lines.append(line)
        return line

    def _command_until(
        self,
        command: str,
        predicate: Callable[[str, list[str]], bool],
        timeout_s: float = 3.0,
    ) -> list[str]:
        if self.serial_port is None:
            raise RuntimeError("BenchClient is not connected")

        self.serial_port.reset_input_buffer()
        self._write(command)

        lines: list[str] = []
        deadline = time.monotonic() + timeout_s

        while time.monotonic() < deadline:
            line = self._readline()
            if not line:
                continue
            lines.append(line)
            if predicate(line, lines):
                return lines

        raise TimeoutError(f"Timeout waiting for response to {command!r}")

    def get_versions(self) -> dict[str, str]:
        lines = self._command_until(
            "GET_VERSIONS",
            lambda line, _: "DUT_VERSION=" in line,
        )

        bench = "unknown"
        dut = "unknown"
        for line in lines:
            if "BENCH_VERSION=" in line:
                bench = line.split("BENCH_VERSION=", 1)[1].split()[0]
            if "DUT_VERSION=" in line:
                dut = line.split("DUT_VERSION=", 1)[1].split()[0]

        return {"bench": bench, "dut": dut}

    def run_smoke(self) -> bool:
        lines = self._command_until(
            "RUN_SMOKE",
            lambda line, _: "RUN_SMOKE ->" in line,
            timeout_s=5.0,
        )
        return "RUN_SMOKE -> PASS" in lines[-1]

    def set_auto(self, enabled: bool) -> None:
        expected = "AUTO -> ON" if enabled else "AUTO -> OFF"
        command = "AUTO_ON" if enabled else "AUTO_OFF"
        self._command_until(
            command,
            lambda line, _: expected in line,
        )

    def get_status(self) -> int:
        lines = self._command_until(
            "FORWARD_COMMAND GET_STATUS",
            lambda line, _: "FORWARD_COMMAND GET_STATUS -> state=" in line,
        )
        match = re.search(r"state=(\d+)", lines[-1])
        if not match:
            raise ValueError(f"Cannot parse status: {lines[-1]}")
        return int(match.group(1))

    def open_manual(self) -> None:
        self._command_until(
            "FORWARD_COMMAND OPEN",
            lambda line, _: "FORWARD_COMMAND OPEN ->" in line,
        )

    def close_manual(self) -> None:
        self._command_until(
            "FORWARD_COMMAND CLOSE",
            lambda line, _: "FORWARD_COMMAND CLOSE ->" in line,
        )

    def get_snapshot(self) -> dict:
        lines = self._command_until(
            "GET_SNAPSHOT",
            lambda line, _: "SNAPSHOT state=" in line,
            timeout_s=4.0,
        )

        for line in reversed(lines):
            if "SNAPSHOT state=" in line:
                return parse_snapshot_line(line)

        raise ValueError("No parsable SNAPSHOT line received")
