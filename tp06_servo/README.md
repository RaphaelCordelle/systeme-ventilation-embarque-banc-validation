# TP06 — Servo control with end-stop feedback

**Status:** current source builds successfully; closing feedback was observed, while opening remains unresolved.

## Objective

Drive the ventilation-flap servo toward open and closed positions, stop the PWM when the physical end stop is reached, and abort unsafe or stalled movement.

## Hardware and wiring

- servo signal on GPIO13
- `CLOSED` end stop on GPIO18
- `OPEN` end stop on GPIO19
- active-low switch wiring with internal pull-ups
- separate 5 V servo supply, common ESP32 ground, and supply decoupling used during bench work

## Implementation

`servo_set_pulse()` generates 1.0 or 2.0 ms commands at 50 Hz. `servo_open()` and `servo_close()` monitor the corresponding debounced end stop, stop PWM through `servo_stop()`, and return a status. Both directions use a 3 s movement timeout. `limits_contradictory()` stops movement when both switches are active.

## Validation and result

The recorded monitor output contains one successful `CLOSED detected` event and repeated opening and closing timeouts, each followed by `PWM SERVO CUT`. No `OPEN detected` event and no contradictory-switch event appear in the saved logs. The current source now builds successfully, but it is newer than the last flashed binary and still needs an on-target regression test.

## Concepts

LEDC PWM, servo pulse control, actuator feedback, active-low inputs, debounce, movement timeout, local fault handling, return-status reporting, power-supply diagnosis.

## Link to the bench

This exercise connects the firmware to the physical ventilation flap. Reliable open-position detection and integration with the sensing TPs remain in progress.
