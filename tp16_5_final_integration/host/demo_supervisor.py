from __future__ import annotations

import argparse
import platform
import sys
import time
from datetime import datetime
from pathlib import Path

from bench_client import BenchClient
from reporting import save_all

ROOT = Path(__file__).resolve().parent.parent
REPORTS_DIR = ROOT / "reports"
EVIDENCE_DIR = ROOT / "evidence"


def normalize_sample(snapshot: dict, elapsed_s: float) -> dict:
    return {
        "elapsed_s": round(elapsed_s, 2),
        "timestamp": datetime.now().isoformat(timespec="milliseconds"),
        "state": snapshot["state"],
        "auto": snapshot["auto"],
        "sensor_valid": snapshot["sensor_valid"],
        "distance_valid": snapshot["distance_valid"],
        "obstacle": snapshot["obstacle"],
        "temperature_c": snapshot["T"],
        "humidity_pct": snapshot["H"],
        "pressure_hpa": snapshot["P"],
        "distance_cm": snapshot["D"],
        "limit_open": snapshot["open"],
        "limit_closed": snapshot["closed"],
        "contradiction": snapshot["contradiction"],
        "ref_valid": snapshot.get("ref_valid", False),
        "ref_temperature_c": snapshot.get("ref_T"),
        "ref_humidity_pct": snapshot.get("ref_H"),
        "ref_pressure_hpa": snapshot.get("ref_P"),
        "delta_temperature_c": snapshot.get("dT"),
        "delta_humidity_pct": snapshot.get("dH"),
        "delta_pressure_hpa": snapshot.get("dP"),
    }


def _sequence_seen(states: list[str], pattern: list[str]) -> bool:
    index = 0
    for state in states:
        if state == pattern[index]:
            index += 1
            if index == len(pattern):
                return True
    return False


def build_checks(session: dict, require_cycle: bool, fault_demo: bool) -> list[dict]:
    samples = session["samples"]
    states = {sample["state"] for sample in samples}
    state_series = [sample["state"] for sample in samples]

    valid_count = sum(1 for sample in samples if sample["sensor_valid"])
    valid_ratio = valid_count / len(samples) if samples else 0.0

    contradiction_seen = any(sample["contradiction"] for sample in samples)
    safe_seen = "SAFE" in states
    auto_seen = any(sample["auto"] for sample in samples)

    cycle_seen = _sequence_seen(
        state_series,
        ["CLOSED", "OPENING", "OPEN", "CLOSING", "CLOSED"],
    )

    checks = [
        {
            "name": "SMOKE_TEST",
            "result": "PASS" if session.get("smoke_pass") else "FAIL",
            "details": "BENCH reference sensor + DUT communication",
        },
        {
            "name": "DUT_SENSOR_VALIDITY",
            "result": "PASS" if valid_ratio >= 0.9 else "FAIL",
            "details": f"{valid_count}/{len(samples)} valid samples",
        },
        {
            "name": "ENDSTOP_CONSISTENCY",
            "result": (
                "PASS"
                if not contradiction_seen
                else ("EXPECTED_FAULT" if fault_demo else "FAIL")
            ),
            "details": "No contradictory end-stop sample" if not contradiction_seen else "Contradiction observed",
        },
        {
            "name": "AUTO_MODE_OBSERVED",
            "result": "PASS" if auto_seen else "NOT_OBSERVED",
            "details": "Automatic mode bit observed in DUT diagnostics",
        },
        {
            "name": "FULL_OPEN_CLOSE_CYCLE",
            "result": "PASS" if cycle_seen else ("FAIL" if require_cycle else "NOT_OBSERVED"),
            "details": "States seen: " + ", ".join(sorted(states)),
        },
        {
            "name": "SAFE_STATE",
            "result": (
                ("PASS" if safe_seen else "FAIL")
                if fault_demo
                else ("OBSERVED" if safe_seen else "NOT_OBSERVED")
            ),
            "details": (
                "SAFE expected in fault-demo mode"
                if fault_demo
                else "SAFE is not required for a normal demo run"
            ),
        },
    ]

    return checks


def main() -> int:
    parser = argparse.ArgumentParser(
        description="TP16.5 final physical demo recorder"
    )
    parser.add_argument("--port", default="COM5")
    parser.add_argument("--baudrate", type=int, default=115200)
    parser.add_argument("--duration", type=float, default=90.0)
    parser.add_argument("--interval", type=float, default=0.5)
    parser.add_argument(
        "--manual",
        action="store_true",
        help="Do not enable automatic temperature/humidity control",
    )
    parser.add_argument(
        "--require-cycle",
        action="store_true",
        help="Require CLOSED->OPENING->OPEN->CLOSING->CLOSED in this order",
    )
    parser.add_argument(
        "--fault-demo",
        action="store_true",
        help="Treat a deliberate end-stop contradiction and SAFE state as expected",
    )
    args = parser.parse_args()

    started = datetime.now()
    stamp = started.strftime("%Y%m%d_%H%M%S")
    client = BenchClient(args.port, args.baudrate)

    session = {
        "project": "TP16.5 Final Integrated Demo",
        "started_at": started.isoformat(timespec="seconds"),
        "environment": {
            "python": platform.python_version(),
            "platform": platform.platform(),
            "port": args.port,
            "baudrate": args.baudrate,
        },
        "versions": {},
        "smoke_pass": False,
        "samples": [],
        "events": [],
    }

    print("\n=== TP16.5 FINAL INTEGRATED DEMO ===")
    print(f"Port: {args.port} | duration: {args.duration:.0f}s | interval: {args.interval:.1f}s")

    try:
        client.connect()
        session["versions"] = client.get_versions()
        print(
            f"Versions: BENCH {session['versions']['bench']} | "
            f"DUT {session['versions']['dut']}"
        )

        session["smoke_pass"] = client.run_smoke()
        print(f"Smoke test: {'PASS' if session['smoke_pass'] else 'FAIL'}")

        if not args.manual:
            client.set_auto(True)
            print("Control: AUTO ON")
        else:
            client.set_auto(False)
            print("Control: MANUAL")

        start_monotonic = time.monotonic()
        previous_state = None
        previous_obstacle = False

        while time.monotonic() - start_monotonic < args.duration:
            loop_start = time.monotonic()
            snapshot = client.get_snapshot()
            elapsed = loop_start - start_monotonic
            sample = normalize_sample(snapshot, elapsed)
            session["samples"].append(sample)

            state = sample["state"]
            if state != previous_state:
                session["events"].append(
                    {
                        "elapsed_s": round(elapsed, 2),
                        "event": "STATE_CHANGE",
                        "details": f"{previous_state or 'BOOT'} -> {state}",
                    }
                )
                previous_state = state

            if sample["obstacle"] and not previous_obstacle:
                session["events"].append(
                    {
                        "elapsed_s": round(elapsed, 2),
                        "event": "OBSTACLE_DETECTED",
                        "details": f"distance={sample['distance_cm']:.1f} cm",
                    }
                )
            previous_obstacle = sample["obstacle"]

            distance_text = (
                f"{sample['distance_cm']:5.1f}cm"
                if sample["distance_valid"]
                else "  INVALID"
            )

            print(
                f"[{elapsed:6.1f}s] {state:8s} | "
                f"T={sample['temperature_c']:5.2f}C "
                f"H={sample['humidity_pct']:5.2f}% | "
                f"D={distance_text} | "
                f"obs={int(sample['obstacle'])} "
                f"limits(O/C)={int(sample['limit_open'])}/{int(sample['limit_closed'])} "
                f"AUTO={int(sample['auto'])}"
            )

            remaining = args.interval - (time.monotonic() - loop_start)
            if remaining > 0:
                time.sleep(remaining)

    except KeyboardInterrupt:
        print("\nRecording stopped by user; reports will still be generated.")
    except Exception as exc:
        session["runtime_error"] = f"{type(exc).__name__}: {exc}"
        print(f"\nERROR: {session['runtime_error']}")
    finally:
        client.close()

    session["finished_at"] = datetime.now().isoformat(timespec="seconds")
    session["checks"] = build_checks(session, args.require_cycle, args.fault_demo)

    states_seen = sorted({s["state"] for s in session["samples"]})
    failed = [c for c in session["checks"] if c["result"] == "FAIL"]
    overall = "FAIL" if failed or session.get("runtime_error") else "PASS"

    session["summary"] = {
        "overall": overall,
        "states_seen": states_seen,
        "sample_count": len(session["samples"]),
        "obstacle_observed": any(s["obstacle"] for s in session["samples"]),
        "safe_observed": any(s["state"] == "SAFE" for s in session["samples"]),
    }

    EVIDENCE_DIR.mkdir(parents=True, exist_ok=True)
    raw_log_path = EVIDENCE_DIR / f"demo_serial_{stamp}.log"
    raw_log_path.write_text("\n".join(client.raw_lines) + "\n", encoding="utf-8")
    session["raw_serial_log"] = str(raw_log_path)

    try:
        paths = save_all(session, REPORTS_DIR, f"demo_{stamp}")
    except Exception as exc:
        print(f"Report generation error: {exc}")
        return 2

    print("\n=== REPORTS ===")
    for kind, path in paths.items():
        print(f"{kind.upper():5s}: {path}")
    print(f"RAW  : {raw_log_path}")
    print(f"Overall: {overall}")

    return 0 if overall == "PASS" else 1


if __name__ == "__main__":
    sys.exit(main())
