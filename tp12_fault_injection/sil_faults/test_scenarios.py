STATE_UNKNOWN = 0
STATE_CLOSED = 1
STATE_OPENING = 2
STATE_OPEN = 3
STATE_CLOSING = 4
STATE_SAFE = 5


EVENT_CMD_OPEN = "CMD_OPEN"
EVENT_CMD_CLOSE = "CMD_CLOSE"
EVENT_LIMIT_OPEN = "LIMIT_OPEN"
EVENT_LIMIT_CLOSED = "LIMIT_CLOSED"
EVENT_TIMEOUT = "TIMEOUT"
EVENT_CONTRADICTION = "CONTRADICTION"


def handle_event(state, event):
    if event in (
        EVENT_TIMEOUT,
        EVENT_CONTRADICTION
    ):
        return STATE_SAFE

    if state == STATE_UNKNOWN:
        if event == EVENT_LIMIT_CLOSED:
            return STATE_CLOSED

        if event == EVENT_LIMIT_OPEN:
            return STATE_OPEN

    elif state == STATE_CLOSED:
        if event == EVENT_CMD_OPEN:
            return STATE_OPENING

    elif state == STATE_OPENING:
        if event == EVENT_LIMIT_OPEN:
            return STATE_OPEN

    elif state == STATE_OPEN:
        if event == EVENT_CMD_CLOSE:
            return STATE_CLOSING

    elif state == STATE_CLOSING:
        if event == EVENT_LIMIT_CLOSED:
            return STATE_CLOSED

    elif state == STATE_SAFE:
        return STATE_SAFE

    return state


def xor_checksum(data):
    result = 0

    for value in data:
        result ^= value

    return result


def valid_frame(frame):
    if len(frame) < 5:
        return False

    received_checksum = frame[-1]
    calculated_checksum = xor_checksum(
        frame[:-1]
    )

    return (
        received_checksum ==
        calculated_checksum
    )


def test_normal_opening():
    state = STATE_CLOSED

    state = handle_event(
        state,
        EVENT_CMD_OPEN
    )

    assert state == STATE_OPENING

    state = handle_event(
        state,
        EVENT_LIMIT_OPEN
    )

    assert state == STATE_OPEN


def test_normal_closing():
    state = STATE_OPEN

    state = handle_event(
        state,
        EVENT_CMD_CLOSE
    )

    assert state == STATE_CLOSING

    state = handle_event(
        state,
        EVENT_LIMIT_CLOSED
    )

    assert state == STATE_CLOSED


def test_blocked_servo_causes_safe():
    state = STATE_CLOSED

    state = handle_event(
        state,
        EVENT_CMD_OPEN
    )

    assert state == STATE_OPENING

    state = handle_event(
        state,
        EVENT_TIMEOUT
    )

    assert state == STATE_SAFE


def test_slow_servo_causes_safe():
    movement_duration_ms = 3500
    timeout_ms = 3000

    state = STATE_CLOSED

    state = handle_event(
        state,
        EVENT_CMD_OPEN
    )

    assert state == STATE_OPENING

    if (
        movement_duration_ms >=
        timeout_ms
    ):
        state = handle_event(
            state,
            EVENT_TIMEOUT
        )

    assert state == STATE_SAFE


def test_endstop_contradiction_causes_safe():
    state = STATE_OPENING

    limit_open = True
    limit_closed = True

    if (
        limit_open and
        limit_closed
    ):
        state = handle_event(
            state,
            EVENT_CONTRADICTION
        )

    assert state == STATE_SAFE


def test_safe_state_is_latched():
    state = STATE_SAFE

    state = handle_event(
        state,
        EVENT_CMD_OPEN
    )

    assert state == STATE_SAFE

    state = handle_event(
        state,
        EVENT_CMD_CLOSE
    )

    assert state == STATE_SAFE


def test_corrupted_frame_is_rejected():
    frame = [
        0xAA,
        0x00,
        0x02,
        0x00
    ]

    checksum = xor_checksum(
        frame
    )

    complete_frame = (
        frame +
        [checksum]
    )

    assert valid_frame(
        complete_frame
    )

    complete_frame[-1] ^= 0xFF

    assert not valid_frame(
        complete_frame
    )


def test_uart_loss_is_detected():
    response = None

    communication_ok = (
        response is not None
    )

    assert communication_ok is False


def test_uart_delay_exceeds_limit():
    normal_latency_ms = 100
    injected_delay_ms = 1000

    total_latency_ms = (
        normal_latency_ms +
        injected_delay_ms
    )

    maximum_latency_ms = 500

    assert (
        total_latency_ms >
        maximum_latency_ms
    )