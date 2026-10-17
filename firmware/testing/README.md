# BLE motor-control TEST code

Recorded October 17, 2026. These sketches are testing artifacts, not a production controller or an autonomous car implementation. Current hardware remains the original ESP32-WROOM-32D plus L293D; no C3 or S3 controller.

## Active recorded test

[DualMotorV2_BLE_Latched.ino](DualMotorV2_BLE_Latched/DualMotorV2_BLE_Latched.ino) is a behavior-preserving copy of the revised latched-control code shared in chat, with documentation/comments consolidated. It records the tested design; no noise fix or supply upgrade is silently included.

- Arduino-ESP32 core 3.x; built-in BLE/FreeRTOS.
- BLE name: `DualMotorV2`.
- Nordic UART service: `6e400001-b5a3-f393-e0a9-e50e24dcca9e`.
- App writes UTF-8 commands to RX: `6e400002-b5a3-f393-e0a9-e50e24dcca9e`.
- Status READ/NOTIFY: `6e400003-b5a3-f393-e0a9-e50e24dcca9e`.
- One command per write or newline-separated complete commands. Actual app must use this format; compatibility has not been independently verified.
- Serial Monitor: 115200 baud.

## Commands and why they exist

| Command | Behavior |
|---|---|
| `forward` | Latches drive forward at selected PWM duty |
| `back` | Latches drive reverse at selected PWM duty |
| `left` | Latches steering left at fixed 128/255 duty |
| `right` | Latches steering right at fixed 128/255 duty |
| `speed_0` through `speed_100` | Integer drive-duty setting; 0 disables both bridges |
| `stop` | Disables both drive and steering bridges |

Initial speed setting is 100%, but both motors start disabled until a direction command. Speed is duty cycle, not measured wheel speed. At `speed_30`, the drive bridge uses approximately 30% duty (about 77/255), not a guaranteed 30% RPM or constant 1.8 V motor supply.

The speed control permits adjustable drive power and comparisons across duty settings. The user specifically highlighted noise around a setting of 30, while settings above that appeared smoother/quieter. The slider changes drive duty only; it does not change fixed steering duty.

Forward/back and steering states are independent. Releasing a button leaves the state unchanged unless the app sends a release packet such as `stop` or a supported false value. Configure release messages accordingly. Commands remain latched with no movement timeout; steering has no duration limit in this version.

Changing between directions disables the affected bridge for a short 50 ms command transition. This is not a measured mechanical stopping interval or validation of reversal current.

## Stop and emergency use

The user selected lowercase `stop` as the button for stopping the entire car during testing. Code sets both enable PWM duties to zero and clears both direction pairs. That disables drive; it does not guarantee an instant mechanical halt, active braking, power isolation, or response if the BLE link/software fails.

BLE disconnection, unknown input, and input overflow also request a software stop. Disconnection clears old motion; reconnecting does not resume it. This is not an independently validated hardware emergency stop. Physical supply switches remain available, and the earlier unintended motor-power path remains unresolved.

Continuous steering against an end stop can stall/heat the motor and consume substantial current. Do not leave it latched against a stop. Pulse/limit/feedback-based steering is a proposed next change, not present in the archived test behavior.

## GPIO wiring

| ESP32 GPIO | L293D physical pin | Function |
|---|---|---|
| 25 | 1 | Drive enable/PWM |
| 26 | 2 | Drive input 1 |
| 27 | 7 | Drive input 2 |
| 14 | 9 | Steering enable/PWM |
| 32 | 10 | Steering input 1 |
| 33 | 15 | Steering input 2 |

L293D pin 8 is the 6 V motor rail; pin 16 is the 5 V logic rail. All four driver ground pins 4/5/12/13 connect to shared load ground. Drive motor is between 3/6; steering is between 11/14. Remove fixed 5 V control jumpers before GPIO wiring. Use two separate 10k pull-downs: pin 1 to GND and pin 9 to GND, not a resistor between pins 1 and 9.

Both PWM channels are configured at 1,000 Hz, 8-bit resolution. Fixed 50% steering therefore remains PWM-driven even when drive speed is 100%.

## Earlier test code

[Timed version](history/DualMotorV2_BLE_Timed/DualMotorV2_BLE_Timed.ino) preserves the earlier speed-0/decimal-speed sketch with 700 ms drive timeout and steering pulses. It is historical and does not describe the user's latest requested behavior.

## Verification

The command parser and motor-request state transitions of the saved latched version were host-tested with g++: integer speed boundaries, decimal/invalid rejection, forward/back/left/right latching, stop-all, and speed-zero behavior. This does not exercise real BLE/PWM, compile against the actual ESP32 core, or verify hardware.

The user reported motor behavior/noise during BLE testing. Exact uploaded binary, full source identity, electrical readings, and disconnect/stop test performance have not been independently verified. See [test log](../../documentation/testing/TEST_LOG.md) and [motor-noise investigation](../../documentation/testing/MOTOR_NOISE_AND_LOAD.md).
