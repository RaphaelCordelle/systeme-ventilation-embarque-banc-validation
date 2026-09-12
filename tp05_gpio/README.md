# TP05 — HC-SR04 ultrasonic measurement

**Status:** current source builds successfully; the recorded hardware run predates this revision.

## Objective

Measure distance from an HC-SR04 by generating a trigger pulse, timing the echo pulse, and rejecting missing or out-of-range measurements.

## Hardware and wiring

- TRIG on GPIO21
- ECHO on GPIO22
- HC-SR04 powered from 5 V with common ground
- resistive divider on ECHO to adapt the 5 V signal to the ESP32 3.3 V input

## Implementation

`trigger_sensor()` generates the 10 us trigger pulse. `measure_distance()` uses `esp_timer_get_time()` to time ECHO, applies a 30 ms timeout, converts time of flight to centimetres, and validates the 2–400 cm range. `DistanceMeasurement` carries the value, validity flag, and timestamp.

`main/tp05_hcsr04_main.c` is an earlier trigger-only draft. The current CMake project compiles `main/tp05_gpio_main.c` instead.

## Validation and result

The logs include valid readings such as 20.0 cm and 20.4 cm, alongside repeated `ECHO stuck high` and invalid-measurement reports. This exercised both the nominal conversion and one timeout path. The current source builds successfully, but it was modified after the last recorded flash and still needs an on-target regression test.

## Concepts

Microsecond timing, busy-wait pulse capture, timeout handling, data validity, timestamping, sensor range checks, 5 V to 3.3 V level adaptation.

## Link to the bench

The distance measurement is a separate sensing input that can later contribute to flap supervision or bench scenarios.
