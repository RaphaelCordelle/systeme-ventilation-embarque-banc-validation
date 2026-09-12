# Secure Embedded Validation Bench

Personal ESP32 project developed as a sequence of practical firmware exercises. The current work covers GPIO, persistent storage, I2C sensing, ultrasonic timing, PWM output, physical end-stop feedback, and serial diagnostics. Each peripheral was brought up separately before system-level integration. The mechanical target is a small instrumented ventilation flap; a single integrated application is not implemented yet.

## Project objective

Build a physical embedded system progressively, validating one interface at a time before combining the sensor and actuator paths. The exercises keep hardware faults, invalid inputs, and incomplete tests visible instead of treating a successful build as sufficient validation.

## Current architecture

```text
BME280 -------- I2C ----------\
HC-SR04 ---- TRIG / ECHO ------+--> ESP32 --> status LEDs
OPEN / CLOSED limit switches --/       |----> PWM buzzer
                                       `----> 50 Hz PWM --> servo --> ventilation flap
```

This diagram groups the blocks already implemented across the exercises. They are not yet integrated into one firmware image.

## Exercise progression

| Exercise | Objective and implementation | Recorded result | Main concepts |
| --- | --- | --- | --- |
| [TP02 — GPIO, logs and NVS](hello_world/README.md) | Toggle a GPIO every second, log the state, and maintain a boot counter in NVS. | Current source builds successfully. LED state changes are present in older serial logs; the current NVS revision still needs a flash test. | GPIO, NVS, ESP-IDF logging, FreeRTOS delay |
| [TP03 — I2C / BME280](tp03_i2c/README.md) | Connect the Bosch driver to the ESP-IDF I2C master API on GPIO21/22. | Chip ID `0x60` detected at `0x76`; temperature, pressure, and humidity samples recorded. | I2C, callbacks, device handles, sensor data |
| [TP04 — GPIO / position feedback](tp04_gpio/README.md) | Read two active-low end stops and signal four logical states with LEDs and a buzzer. | `OPEN`, `CLOSED`, `BETWEEN`, `CONTRADICTORY`, and unstable transitions appear in the logs. | GPIO, pull-ups, debounce, enums, PWM, fault states |
| [TP05 — HC-SR04](tp05_gpio/README.md) | Measure echo pulse width and convert it to distance with timeout and range checks. | Current source builds successfully. Valid distances and repeated `ECHO stuck high` failures were recorded on the previous flashed revision. | Microsecond timing, timeout, validation, 5 V / 3.3 V interface |
| [TP06 draft — basic servo PWM](tp06_gpio/README.md) | Generate a fixed 1.5 ms servo pulse on GPIO13. | Source-only draft; no standalone ESP-IDF project or test record. | LEDC, PWM frequency, duty-cycle conversion |
| [TP06 — servo and end stops](tp06_servo/README.md) | Move the servo toward each end stop, stop PWM on feedback, and abort on timeout or contradictory inputs. | Current source builds successfully. `CLOSED` was detected once; opening and several closing cycles timed out. No recorded contradictory-input test. | Actuator control, physical feedback, timeout, local fault handling |

## Hardware

- ESP32 development board, target `esp32` (DEVKITV1 in the project setup)
- Bosch BME280 environmental sensor; chip ID `0x60` is recorded
- HC-SR04 ultrasonic sensor with a 5 V to 3.3 V ECHO divider
- 5 V hobby servo used for the flap prototype
- two mechanical end-stop switches
- red, yellow, and green status LEDs
- buzzer and transistor interface
- resistors, breadboard, external 5 V supply, decoupling capacitor, and jumper wires
- multimeter for continuity and voltage checks

Exact resistor, transistor, and capacitor references are not documented in the source tree yet.

## Environment

- C with ESP-IDF APIs
- ESP-IDF `v6.1-beta1`, target `esp32`
- FreeRTOS timing primitives used from the ESP-IDF runtime
- VS Code and PowerShell
- ESP-IDF serial monitor at 115200 baud
- Git for repository versioning

Each complete TP remains an independent ESP-IDF project. From one of those directories:

```text
idf.py set-target esp32
idf.py build
idf.py flash monitor
```

`tp06_gpio` is the exception: it is an incomplete source-only draft and has no project-level CMake file.

## Diagnostics and validation

- GPIO output and serial-log checks on the initial LED exercise
- BME280 identification and continuous measurement logging
- manual activation of both end stops, including contradictory and unstable states
- HC-SR04 valid-distance checks, range validation, and echo timeout diagnostics
- servo power and common-ground checks with a multimeter
- movement timeout and PWM shutdown on the servo exercise
- iterative flash and serial-port troubleshooting; the local logs include both failed attempts and successful flashes

The generated build and monitor logs remain available locally but are excluded from Git.

## Current state

### Completed and recorded

- ESP32 GPIO output and serial monitoring
- BME280 communication and environmental readings over I2C
- end-stop decoding with four position states
- LED and buzzer signalling for position feedback
- HC-SR04 pulse timing, valid readings, range checks, and error reporting
- servo PWM generation, timeout handling, PWM shutdown, and one recorded `CLOSED` detection

### In progress

- flash and on-target validation of the latest TP02, TP05, and TP06 source revisions
- reliable physical detection of the servo `OPEN` position
- complete validation of the contradictory end-stop path in TP06
- a more modular C structure and a global state machine
- integration of the separate exercises into one ventilation-flap firmware

### Planned later

- explicit FreeRTOS multi-task architecture
- UART communication between two ESP32 boards
- SPI and CAN/TWAI exercises
- Python supervisor and automated bench control
- SIL/HIL tests and fault injection
- requirements-to-test-result traceability

These planned items are not implemented in the repository.

## Photos and videos

Media will be added after the next hardware validation session.

- `docs/images/` — assembly, wiring, individual TP setups, and serial-monitor captures
- `docs/videos/` — ventilation-flap demonstration

No photograph or video is currently included.

## Repository layout

```text
.
|-- hello_world/       TP02 GPIO, logs, and NVS (legacy folder name retained)
|-- tp03_i2c/          BME280 over I2C
|-- tp04_gpio/         End stops, status LEDs, and buzzer
|-- tp05_gpio/         HC-SR04 timing and validation
|-- tp06_gpio/         Incomplete basic servo PWM draft
|-- tp06_servo/        Servo control with end-stop feedback
|-- docs/images/       Planned photographs and captures
|-- docs/videos/       Planned demonstrations
`-- THIRD_PARTY_NOTICES.md
```

## Third-party code

TP03 includes the Bosch Sensortec BME280 Sensor API v3.5.1 under the BSD 3-Clause license. See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
