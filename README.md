# Dual Motor V2

An ESP32 robotic car with one motor for forward/reverse and one motor for steering.

**Current revision: V2.6.0 BLE motor-test and noise investigation update — October 17, 2026.** The damaged C3 is excluded. The car will use the ESP32-WROOM-32D and receive manual commands from a phone or computer over Wi-Fi or Bluetooth. The earlier C3/S3 handheld-controller design is historical. **No C3 or S3 controller is used or required in the active build.** The block diagrams and current decision record define the plan.

## Current plan

- ESP32-WROOM-32D development board as the vehicle controller.
- L293D to control two existing 3–6 V brushed DC motors: drive and steering.
- Manual wireless control plus planned automatic obstacle avoidance.
- Front ultrasonic sensor, GPS, buzzer, and two white headlights.
- Three 1.5 V cells in series for a nominal 4.5 V motor pack.
- Separate rechargeable electronics battery; 1,000 mAh and 500 mAh batteries are available.
- Use two independent boost converters: 6 V motor rail and 5 V electronics rail; complete integration and load capability remain unverified.
- Use a TP4056-style charger for the electronics cell and two on/off switches, one per supply branch (reported three-terminal switch: likely SPDT used for an SPST function); charge with both load switches off.
- Use GNU Octave for initial movement and electrical estimates, then replace assumptions with measurements.

**Status:** planning, reported drive-motor bench tests, and three hypothetical MATLAB/Octave examples. Electronics switch/LED power-up is now user-reported; its electrical validation, full vehicle firmware, GPS/map/API integration, steering, charging thermal behavior, and load capability are not validated. The motor unexpectedly ran with its motor-power switch off during a USB-powered test; that path remains unresolved.

## Documentation

| Topic | File |
|---|---|
| Active design decisions | [Decisions](documentation/DECISIONS.md) |
| Tests, notes, and observations | [Test log](documentation/testing/TEST_LOG.md) |
| Power terminals, rails, charger, and switches | [Power pinout plan](documentation/pin-configuration/POWER_PINOUT.md) |
| Signal and power block diagrams | [Block diagrams](diagrams/README.md) |
| Parts, power, control, and measurement plan | [Vehicle plan](documentation/architecture/VEHICLE_V2.md) |
| Octave script and assumptions | [Simulation notes](simulations/README.md) |
| Current checklist and earlier progress | [Progress](progress/PROGRESS.md) |
| Electrical and mechanical limits | [Limitations](limitations/LIMITATIONS.md) |
| Electronics power-chain model | [MATLAB / Octave script](simulations/electronics_power_chain.m) |
| BLE motor test code / commands | [Testing guide](firmware/testing/README.md) |
| Motor noise and steering-load slowdown | [Investigation](documentation/testing/MOTOR_NOISE_AND_LOAD.md) |
| Revision record | [V2.6.0](versions/V2.6.0.md) |
| Earlier controller overview | [Historical README](documentation/history/CONTROLLER_README.md) |

## First Octave result

With an assumed 65 mm wheel, 120 wheel RPM, and a 1.5 s response time, the example approaches **0.41 m/s** and covers **3.47 m in 10 s**. The user ran the initial script and shared the movement plots.

These are hypothetical results. The movement model does not yet couple battery voltage, current, driver loss, mass, or motor torque to speed. The boost comparison in the baseline script changes electrical estimates only. The latest power/control model adds a simplified voltage-to-RPM assumption; it still does not model torque or mass.

## Latest reported tests and model

- BLE motor-control test code is recorded: startup drive duty100%, integer speed_N commands, latched directions, and stop-all on lowercase stop/disconnect. No complete vehicle firmware is released.
- Steering noise and drive noise around speed 30 persist; latest 1,000 uF/50 V capacitor experiment did not eliminate sound. Drive slows when steering activates. Causes remain unmeasured.

- Electronics supply: user reports correcting an overlapping battery/wire connection, working switch, and ESP32 power LED. Accurate output voltage, load capability, and heating remain unverified.
- New electronics-chain model: stage voltages, battery versus output current, switch/protection/boost losses, and a separate charging-loss example.

- Direct boosted drive motor: visibly faster than the earlier 3.7 V battery test; no RPM/current measurement.
- Through L293D: visually estimated roughly 50% slower; driver loss versus supply sag not measured.
- Motor switch off: motor still ran when USB powered the ESP32 board/driver logic; switching motor power on increased speed. The source path is not diagnosed.
- Second Octave example: forward/disable/reverse sequence, two supply current estimates, and shutdown at 18 s. User-shared plots match the script assumptions.

See the test log for charging heat and multimeter troubleshooting. Model outputs are not hardware measurements.

## Historical controller work

C3/S3 firmware and earlier controller notes are retained as development history. They do not implement the current ESP32-only vehicle plan. The S3 display and joystick controller are outside the active build scope.
