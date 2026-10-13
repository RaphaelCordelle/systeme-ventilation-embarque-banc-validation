from pathlib import Path

from reporting import save_all


def test_report_bundle(tmp_path: Path):
    session = {
        "started_at": "2026-10-05T00:00:00",
        "finished_at": "2026-10-05T00:00:10",
        "versions": {"bench": "1.0.0", "dut": "1.0.0"},
        "summary": {"overall": "PASS", "states_seen": ["CLOSED", "OPEN"]},
        "checks": [{"name": "SMOKE_TEST", "result": "PASS", "details": "ok"}],
        "events": [{"elapsed_s": 0.0, "event": "STATE_CHANGE", "details": "BOOT -> CLOSED"}],
        "samples": [
            {
                "elapsed_s": 0.0,
                "timestamp": "2026-10-05T00:00:00.000",
                "state": "CLOSED",
                "auto": True,
                "sensor_valid": True,
                "distance_valid": True,
                "obstacle": False,
                "temperature_c": 24.2,
                "humidity_pct": 50.0,
                "pressure_hpa": 1000.0,
                "distance_cm": 40.0,
                "limit_open": False,
                "limit_closed": True,
                "contradiction": False,
                "ref_valid": True,
                "ref_temperature_c": 24.1,
                "ref_humidity_pct": 49.8,
                "ref_pressure_hpa": 999.9,
                "delta_temperature_c": 0.1,
                "delta_humidity_pct": 0.2,
                "delta_pressure_hpa": 0.1,
            }
        ],
    }

    paths = save_all(session, tmp_path, "demo")

    assert set(paths) == {"json", "csv", "html", "xlsx"}
    for path in paths.values():
        assert Path(path).is_file()
        assert Path(path).stat().st_size > 0
