# USB Serial motor control and command logs

October 17, 2026. Current test sketch supports BLE and USB Serial on the original ESP32-WROOM-32D. Pulse remains 150 ms for Test 2; no GPIO, PWM frequency/duty, or drive polarity changes.

## Use

Download [DualMotorV2_BLE_Pulse.ino](../../firmware/testing/DualMotorV2_BLE_Pulse/DualMotorV2_BLE_Pulse.ino), save in a folder named DualMotorV2_BLE_Pulse, and upload using Arduino-ESP32 core 3.x. Open Serial Monitor at 115200 baud and select Newline (Both NL & CR is also accepted). Use USB with the established separate motor supply; do not bridge USB 5 V with another external 5 V source unless the board's power arrangement permits it. Opening Serial Monitor can reset some development boards; startup motors are stopped.

Commands, one line each:
- forward / back: latch driving using the previously reversed electrical mapping.
- left / right: one 150 ms steering pulse after the 50 ms transition guard, then steering bridge disabled.
- speed_0 through speed_100: integer drive setting; speed_0 stops both motors.
- stop: stop both motors and re-arm a same-side steering pulse.
- status: print current commanded state without changing motion.
- telemetry_on: enable periodic status at approximately 250 ms intervals.
- telemetry_off: disable periodic status; command and output-change logs remain.
- help: list commands.

Startup speed is 100 with motors stopped. Same-side steering messages do not extend or restart a pulse until opposite or stop. For independent repeated steering tests, send stop, reset the mechanism's starting position safely, then send left/right again.

## What the logs mean

Example format (illustrative, not a hardware result):

    t=1000 rx[usb]: left
    t=1050 src=usb speed=100 drive=-1 pwm=255 steer=-1 pwm=128 remain=150 ms
    pins D[0,1] S[1,0]; configured pulse=150 ms; COMMANDS, not sensor readings

- t: ESP32 uptime in milliseconds, not wall-clock time.
- src: USB/BLE source of the latest motor command; status/help/telemetry commands do not take ownership.
- speed: requested drive-duty percentage; it is not measured RPM.
- drive/steer: firmware direction state (drive -1 corresponds to forward in this wiring revision; +1 is back; steering -1 left, +1 right; zero stopped). During transition, direction state may be nonzero while PWM is still zero.
- pwm: configured LEDC duty from 0 to 255, not measured volts/current. Steering remains 128/255 (~50%).
- remain: software pulse time remaining; zero when pulse inactive or the command is still awaiting its transition delay.
- pins: digital output latch levels for each bridge direction pair. These are not measurements at motor terminals.
- Output-change logs show when drive/steer duty or direction changes, including the steering cutoff.

The periodic interval is longer than the pulse, so use output-change events for its start/end rather than expecting periodic samples to resolve it. The loop and serial output introduce timing granularity; this is not an oscilloscope or hard real-time trace. A final requested state may be logged instead of every intermediate state when multiple commands arrive together.

No voltage, current, CV/CC, speed, wheel angle, sound, stall detection, or actual torque is sensed by this firmware. Correlate the printed times with visual motion and the bench display; enter actual observations in the test log rather than inventing sensor data.

## Mixed control and stopping

BLE and USB feed the same motor state. Commands from either may change it; use one control interface at a time during comparisons. BLE connection/disconnection transitions stop all motors, even if a prior command came from USB; this preserves a clear stop on link changes. After that event USB can run without a BLE connection. BLE disconnect does not continuously block USB operation.

Unknown commands, oversized/invalid serial lines, BLE queue overflow, and stop disable both motors. Serial lines over 96 characters are discarded through their next newline. Serial input polling is bounded per loop. USB cable removal is not reliably detected by this UART interface: there is no USB disconnect stop or drive watchdog. Drive remains latched until a stop or other existing stop condition. Use the physical power switch to stop/isolate motor power as needed; software stop is not hardware power isolation.

## Validation

Existing pulse/drive host tests and the new serial/mixed-source host tests passed. These compile actual parser, motor, serial input and loop functions with mocked GPIO/PWM, UART, time, BLE state and queue operations. They verify offline USB operation, CRLF/fragmented input, status without motion changes, pulse cutoff, speed, stop, overflow, and BLE transition stops with later USB recovery. They do not compile against actual ESP32/BLE libraries or validate hardware electrical behavior.

Run from repo root:

    python3 tests/test_ble_pulse.py firmware/testing/DualMotorV2_BLE_Pulse/DualMotorV2_BLE_Pulse.ino
    python3 tests/test_serial_control.py firmware/testing/DualMotorV2_BLE_Pulse/DualMotorV2_BLE_Pulse.ino

API references:
- https://docs.espressif.com/projects/arduino-esp32/en/latest/api/serial.html
- https://docs.espressif.com/projects/arduino-esp32/en/latest/api/ledc.html
