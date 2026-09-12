# TP02 — GPIO, logs and NVS

**Status:** current source builds successfully; on-target validation is older than the current revision.

## Objective

Set up a basic ESP-IDF application, drive an LED output periodically, report firmware state over the serial monitor, and store a persistent boot counter.

## Hardware and wiring

- ESP32 development board
- LED output on GPIO18
- LED series resistor and common ground; the resistor value is not recorded in the project files
- USB connection for power, flash, and monitoring

## Implementation

`main/hello_world_main.c` initializes NVS, recovers from incompatible or full NVS pages, increments `boot_count`, and logs the firmware version. GPIO18 is toggled every second using `vTaskDelay()`.

Main entry point: `app_main()`.

## Validation and result

Serial logs record repeated `LED ON` and `LED OFF` messages at one-second intervals. The current source now builds successfully, but it was edited after the last flashed binary and the available logs do not record its `FW_VERSION` or boot counter output. A fresh flash and NVS persistence check are still required.

## Concepts

GPIO output, NVS key/value storage, ESP-IDF logging, error recovery, FreeRTOS ticks, firmware versioning.

## Link to the bench

This exercise establishes the basic build, flash, monitor, GPIO, and persistence workflow used by the later sensor and actuator TPs.
