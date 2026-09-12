# TP03 — I2C and BME280

**Status:** built, flashed, and exercised with recorded sensor output.

## Objective

Configure an ESP32 I2C master and connect the Bosch BME280 driver to read environmental data.

## Hardware and wiring

- BME280 powered from 3.3 V with a common ground
- SDA on GPIO21
- SCL on GPIO22
- fixed 7-bit address `0x76`

## Implementation

`main/tp03_i2c_main.c` creates the I2C bus and device handles at 100 kHz. The functions `bme280_i2c_read()`, `bme280_i2c_write()`, and `bme280_delay_us()` adapt the Bosch API to ESP-IDF. The sensor runs in normal mode with 1x oversampling and one sample logged per second.

The code addresses `0x76` directly; it does not implement a general I2C bus scanner.

## Validation and result

The monitor logs show a BME280 chip ID of `0x60` followed by temperature, pressure, and humidity measurements. Successful flash records are also present. Earlier logs include serial-port access failures, which were resolved during bring-up.

## Concepts

I2C master configuration, device handles, C callbacks, third-party driver integration, sensor configuration, structured measurements, serial diagnostics.

## Link to the bench

The BME280 provides the environmental sensing block intended for later ventilation control logic.
