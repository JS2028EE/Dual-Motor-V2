# HC-SR04 front obstacle sensor integration

Status: firmware and wiring plan added for the original ESP32-WROOM-32D. Hardware bring-up and range calibration are still pending.

## Purpose

The HC-SR04 measures the distance to an object in front of the car. The firmware prints the latest distance in USB Serial and adds a conservative forward-drive interlock:

- A fresh valid reading above 20 cm permits a new `forward` command.
- A reading at or below 20 cm stops an active forward command and blocks another one.
- An invalid, missing, or stale reading blocks forward movement.
- `back`, `left`, `right`, and `stop` retain their existing control behavior.
- When the obstacle clears, the car does not restart automatically. Send a new `forward` command.

This is a first test-stage obstacle stop, not a certified collision-avoidance or emergency-stop system. It does not detect objects outside the sensor's beam, guarantee braking distance, or protect against sensor faults that produce plausible readings.

## Pin assignment

The existing motor-control GPIO assignments remain unchanged.

| HC-SR04 | ESP32-WROOM-32D | Notes |
|---|---|---|
| VCC | 5V / regulated ELEC_5V | HC-SR04 module supply |
| GND | GND / SYSTEM_GND | Must share signal reference with ESP32 |
| TRIG | GPIO18 | ESP32 3.3 V output to sensor trigger |
| ECHO | GPIO19 through divider | Never connect 5 V Echo directly to ESP32 |

### Echo voltage divider — required

Use a 1 kΩ resistor in series from HC-SR04 ECHO to a junction. Connect the junction to ESP32 GPIO19. Connect a 2 kΩ resistor from that junction to GND.

`HC-SR04 ECHO -> 1 kΩ -> junction -> GPIO19`

`junction -> 2 kΩ -> GND`

At a 5.0 V Echo level, the divider produces approximately 3.33 V at GPIO19. Check resistor values and wiring before powering the sensor. Do not omit the divider and do not put 5 V on an ESP32 GPIO.

The sensor's VCC should use the regulated 5 V electronics rail. Do not power it from the ESP32 3V3 pin. Confirm that the 5 V converter and board input can supply the sensor in addition to the existing electronics load.

## Firmware behavior

Current sketch: [DualMotorV2_BLE_Pulse.ino](DualMotorV2_BLE_Pulse/DualMotorV2_BLE_Pulse.ino)

- TRIG pulse: 10 microseconds.
- Measurement interval: approximately 70 ms.
- Obstacle threshold: 20 cm.
- Echo timeout: 30 ms.
- A reading older than 250 ms is treated as stale.
- Echo pulse width is converted using approximately 0.0343 cm per microsecond, divided by two for the outbound and return path.
- Echo timing uses an interrupt so the main motor-control loop does not wait in a long blocking `pulseIn()` call.

The front sensor only protects forward travel. Reverse remains available so the operator can back away from an obstacle. Steering pulses remain timed at 200 ms. If an obstacle is detected while driving forward, the drive bridge is disabled; the steering bridge is not automatically changed.

A sensor timeout is treated as unknown distance, not as proof that the path is clear. This can prevent forward travel if the sensor is disconnected, miswired, pointed into open space beyond its usable range, or unable to receive an echo. That conservative behavior must be evaluated during testing.

## First bring-up procedure

1. Keep the motor power branch OFF and raise the driven wheels clear of the table.
2. Disconnect the portable external 5 V load supply before USB programming until the ESP32 board's USB/external-power isolation is verified. Do not connect two 5 V sources together.
3. Wire the HC-SR04 VCC, GND, TRIG, and divided ECHO signal exactly as shown above.
4. Upload the current sketch to the original ESP32-WROOM-32D using Arduino-ESP32 core 3.x.
5. Open Serial Monitor at 115200 baud. Use Newline for commands.
6. Send `status`. A valid reading should appear as `ultrasonic=xx.x cm`.
7. Hold a flat object in front of the sensor at several measured distances and compare the Serial readings with a ruler.
8. With motor power still OFF, confirm that the status changes to obstacle YES at 20 cm or closer.
9. Only after the readings are stable, conduct a supervised low-risk motor test with the wheels raised. Verify that forward is blocked near the threshold and that reverse still works.
10. Test the obstacle stop on the floor only in a clear area, at low speed, with a hand ready to cut motor power. Do not rely on software as the only stop mechanism.

## Serial commands

Existing commands remain: `forward`, `back`, `left`, `right`, `speed_0` through `speed_100`, `stop`, `status`, `help`, `telemetry_on`, and `telemetry_off`.

Use `status` to inspect the latest distance and whether forward is allowed. Use `telemetry_on` to print status periodically. The BLE driving commands remain unchanged; sensor status is currently logged to USB Serial and is not yet added as a dedicated BLE telemetry field or website display.

## Verification limits

The firmware and host test harness have been updated, but the new sketch has not yet been compiled with the ESP32 Arduino core or tested on the physical sensor. GPIO pin availability, interrupt behavior, sensor range, false echoes, stopping distance, motor coast distance, and power-rail stability still require hardware validation. Do not treat the 20 cm threshold as a measured safe stopping distance.
