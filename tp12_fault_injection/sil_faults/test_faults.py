from fault_model import FaultModel


def test_sensor_disconnected():
    fault = FaultModel()

    sensor = fault.sensor_disconnected()

    assert sensor["valid"] is False


def test_sensor_frozen():
    fault = FaultModel()

    first = fault.freeze_distance(40.0)
    second = fault.freeze_distance(70.0)

    assert first == 40.0
    assert second == 40.0


def test_sensor_drift():
    fault = FaultModel()

    result = fault.drift_distance(
        40.0,
        5.0
    )

    assert result == 45.0


def test_sensor_noise():
    fault = FaultModel()

    result = fault.noise_distance(
        40.0,
        -3.0
    )

    assert result == 37.0


def test_invalid_distance():
    fault = FaultModel()

    result = fault.invalid_distance()

    assert result < 0.0


def test_servo_slow():
    fault = FaultModel()

    movement_time = fault.servo_slow(
        1500
    )

    assert movement_time > 3000


def test_servo_blocked():
    fault = FaultModel()

    movement_detected = fault.servo_blocked()

    assert movement_detected is False


def test_endstop_contradiction():
    fault = FaultModel()

    endstops = fault.endstop_contradiction()

    assert endstops["limit_open"] is True
    assert endstops["limit_closed"] is True


def test_uart_loss():
    fault = FaultModel()

    response = fault.uart_loss()

    assert response is None


def test_uart_delay():
    fault = FaultModel()

    latency = fault.uart_delay(
        100
    )

    assert latency == 1100


def test_corrupted_checksum():
    fault = FaultModel()

    normal_checksum = 0x42

    corrupted = fault.corrupted_checksum(
        normal_checksum
    )

    assert corrupted != normal_checksum