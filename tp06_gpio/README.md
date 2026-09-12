# TP06 draft — basic servo PWM

**Status:** incomplete source-only draft.

## Objective

Generate a standard 50 Hz control signal for a hobby servo and calculate the duty value for a 1.5 ms pulse.

## Hardware and wiring

- servo signal on GPIO13
- separate 5 V servo supply and common ground are required by the bench setup

## Implementation

`main/tp06_gpio_main.c` configures LEDC in 16-bit mode at 50 Hz and applies a fixed 1.5 ms pulse. There is no position command sequence, end-stop feedback, timeout, or PWM shutdown in this draft.

Main entry point: `app_main()`.

## Validation and result

This folder has no project-level or component `CMakeLists.txt`, no `sdkconfig`, and no build or serial log. It cannot be built as a standalone ESP-IDF project in its current state. No physical result is claimed.

## Concepts

LEDC timer/channel setup, PWM period, pulse-width to duty-cycle conversion.

## Link to the bench

This draft is the first servo-control step. The completed feedback and timeout work continues in `tp06_servo`.
