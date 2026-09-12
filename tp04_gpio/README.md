# TP04 — GPIO position feedback and signalling

**Status:** built, flashed, and exercised across all four logical position states.

## Objective

Read two physical end stops, reject unstable transitions, classify the flap position, and provide local visual and audible feedback.

## Hardware and wiring

- `CLOSED` end stop on GPIO18
- `OPEN` end stop on GPIO19
- active-low switch wiring with internal pull-ups
- red LED on GPIO25, yellow LED on GPIO26, green LED on GPIO27
- buzzer command on GPIO23

## Implementation

`read_debounced_gpio()` compares two samples separated by 20 ms. `get_position()` returns `OPEN`, `CLOSED`, `BETWEEN`, or `CONTRADICTORY`. The selected state drives the three LEDs; the contradictory state also enables a 2 kHz LEDC buzzer output.

## Validation and result

The serial logs contain `OPEN`, `CLOSED`, `BETWEEN`, `CONTRADICTORY`, and unstable-input messages. This confirms that each branch was reached during manual switch testing. Successful build and flash records are present.

## Concepts

GPIO inputs and outputs, internal pull-ups, active-low logic, software debounce, enums, Boolean state decoding, LEDC PWM, fault signalling.

## Link to the bench

The two end stops provide physical position feedback and a local contradictory-state diagnostic for the motorized flap.
