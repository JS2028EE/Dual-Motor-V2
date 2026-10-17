# BLE motor-control TEST firmware

## Current: pulse steering, latched drive — October 17, 2026

Use [DualMotorV2_BLE_Pulse.ino](DualMotorV2_BLE_Pulse/DualMotorV2_BLE_Pulse.ino). This revision implements bounded steering pulses and reverses drive polarity to match the user's reported swapped motor wires. It is test firmware, not a validated complete vehicle controller.

Original ESP32-WROOM-32D, Arduino-ESP32 core 3.x. No C3/S3 controller and no extra Arduino libraries. BLE name remains `DualMotorV2`; Serial Monitor 115200.

| Command | Current behavior |
|---|---|
| `forward` | Drive remains forward until changed/stopped; electrical polarity reversed from prior sketch |
| `back` | Drive remains backward until changed/stopped; electrical polarity reversed from prior sketch |
| `left` | One 150 ms left steering pulse, then steering bridge disabled |
| `right` | One 150 ms right steering pulse, then steering bridge disabled |
| `speed_0`–`speed_100` | Integer drive-duty setting; 0 stops both |
| `stop` | Disables both bridges, clears pending steering and drive, re-arms steering |

Initial drive setting remains100%, with both motors stopped at startup. Drive stays latched with no command timeout. Steering PWM remains128/255 (about50%), and both channels remain1 kHz: no voltage, steering-duty, or frequency increase is included.

The 150 ms limit starts when steering is actually energized, after the short command-transition delay. It is an uncalibrated candidate selected for the source-only Test 2 comparison, not proof of full wheel travel. Same-side repeat messages are ignored during and after the pulse, so a held/repeated button cannot continuously extend steering. Send the opposite direction or `stop` before requesting a new same-side pulse. Forward/back and speed changes do not re-arm completed steering. Stop/disconnect clears all motion.

## Position is not held electrically

At pulse completion, steering enable becomes zero and both direction outputs become LOW. The bridge is disabled (coast), not actively holding an angle or applying a position lock. The wheels may remain turned, spring back, or move under external load depending on the mechanism. No steering sensor or end-stop detection exists. A timed pulse can still reach an end stop before its timer expires.

## Wiring and BLE

| ESP32 GPIO | L293D physical pin | Function |
|---|---|---|
| 25 | 1 | Drive enable/PWM |
| 26 | 2 | Drive input1 |
| 27 | 7 | Drive input2 |
| 14 | 9 | Steering enable/PWM |
| 32 | 10 | Steering input1 |
| 33 | 15 | Steering input2 |

L293D pin8 ->6 V motor rail; pin16 ->5 V logic rail. Pins4/5/12/13 share load ground. Drive motor3/6; steering11/14. Use two separate10k pull-downs, pin1->GND and pin9->GND. Remove fixed5 V control jumpers before GPIO connection. Refer to the [power plan](../../documentation/pin-configuration/POWER_PINOUT.md).

- Service: `6e400001-b5a3-f393-e0a9-e50e24dcca9e`.
- RX write: `6e400002-b5a3-f393-e0a9-e50e24dcca9e`.
- TX status READ/NOTIFY: `6e400003-b5a3-f393-e0a9-e50e24dcca9e`.
- UTF-8 complete commands per write, or newline-separated complete commands.
- Configure button release to send nothing if motion should remain latched. A received `stop` or supported false packet still stops both.

## Emergency software stop and limits

Lowercase `stop` is the user's emergency software command, not a hardware emergency-stop circuit. BLE disconnect, unknown command, and input overflow also stop motor requests. Coast stopping is not instantaneous mechanical braking or physical power isolation. The earlier unexpected motor-power path is not diagnosed by this code.

Steering stall is the user's plausible explanation for noise/shared-load slowdown; continuous noise reduction was reported at 150 ms, while quantitative improvement and the new 150 ms result remain unmeasured. Supply limit behavior, driver heating, low-duty buzzing, and actual steering travel remain to be checked. Keep motor rail at6 V; do not interpret 50% PWM as exactly half the supply power.

## Test history

- [Previous latched-steering sketch](DualMotorV2_BLE_Latched/DualMotorV2_BLE_Latched.ino): continuous steering until stop/opposite command. Historical comparison only.
- [Original timed test](history/DualMotorV2_BLE_Timed/DualMotorV2_BLE_Timed.ino): speed0 startup, decimal-speed messages and drive timeout. Historical only.

## Verification

Host tests extract the actual parser/control functions and mock GPIO/PWM. They check reversed drive polarity, latched drive during steering, pulse duration, same-side-repeat suppression, opposite re-arm, stop cancellation, speed0, and unsigned timer rollover.

Run from repository root:
```bash
python3 tests/test_ble_pulse.py firmware/testing/DualMotorV2_BLE_Pulse/DualMotorV2_BLE_Pulse.ino
```

Requires Python3 and g++. Tests passed in the assistant environment. No actual ESP32-core compilation, BLE radio test, motor-power measurement, steering-angle validation, or independent confirmation of user-reported noise reduction was performed.

## Current three-test plan

Default STEER_PULSE_MS is now 150 ms for Test 2. Restore 200 ms for the combined Test 3. See the [test matrix and exact capacitor wiring](../../documentation/testing/STEERING_TEST_MATRIX.md). Test 1 results are recorded; Test 2 and Test 3 results are pending.
