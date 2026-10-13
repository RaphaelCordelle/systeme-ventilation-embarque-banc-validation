from bench_client import parse_snapshot_line


def test_parse_snapshot_line():
    line = (
        "I (1234) BENCH_PROTOCOL: SNAPSHOT state=OPEN auto=1 sensor_valid=1 "
        "distance_valid=1 obstacle=0 T=24.50 H=51.20 P=1008.30 D=42.1 "
        "open=1 closed=0 contradiction=0 ref_T=24.30 ref_H=50.90 "
        "ref_P=1008.10 dT=0.20 dH=0.30 dP=0.20"
    )

    result = parse_snapshot_line(line)

    assert result["state"] == "OPEN"
    assert result["auto"] is True
    assert result["sensor_valid"] is True
    assert result["distance_valid"] is True
    assert result["T"] == 24.50
    assert result["D"] == 42.1
    assert result["ref_valid"] is True
    assert result["dT"] == 0.20
