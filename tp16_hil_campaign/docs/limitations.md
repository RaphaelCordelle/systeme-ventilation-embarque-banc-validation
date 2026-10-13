# TP16 - Known limitations

## Mechanical actuation

The current HIL setup requires manual activation of the OPEN and CLOSED
end-stops during actuator movement tests.

The firmware state machine, commands and end-stop handling are validated,
but the complete autonomous mechanical flap sequence is not considered
fully integrated in this campaign.

## Automated HIL transport

The Python SerialTransport currently automates STATUS, OPEN and CLOSE.

Some functions validated manually through BENCH logs, including telemetry
and DUT reboot recovery, are not yet directly exposed as automated Python
transport capabilities.

## Fault injection

UART loss, DUT sensor absence and end-stop faults were injected physically.

Advanced synthetic fault injection such as controlled sensor drift or frozen
measurements was not implemented in this campaign.

## CAN

CAN is not part of TP16.

A dedicated CAN extension is planned separately.

## Security

Protocol security is outside the scope of this project version.