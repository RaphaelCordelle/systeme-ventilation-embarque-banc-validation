from dataclasses import dataclass
from typing import Optional


@dataclass
class Command:
    name: str
    value: Optional[int] = None


@dataclass
class Response:
    message_type: str
    payload: bytes


@dataclass
class Observation:
    name: str
    value: str


@dataclass
class StepResult:
    name: str
    status: str
    expected: str
    actual: str


@dataclass
class TestResult:
    name: str
    status: str
    steps: list[StepResult]