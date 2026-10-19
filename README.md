# Dual Motor V2

An ESP32-WROOM-32D robotic car with one drive motor, one steering motor, and an L293D driver. A phone or computer will communicate directly with the car over Wi-Fi or Bluetooth through a dedicated app.

**Documentation revision V2.8.0 — October 19, 2026.** This update consolidates the vehicle and app plan. It does not add Wi-Fi firmware or a working app.

## Working milestone

Jay reports successful driving and steering together using a 6 V bench supply with a 0.60 A current limit and 200 ms steering pulses. The supply display reportedly stayed at or below approximately 0.55 A during observed interactions. These are user-reported bench results, not transient-current or thermal certification.

Current [motor test firmware](firmware/testing/DualMotorV2_BLE_Pulse/DualMotorV2_BLE_Pulse.ino) supports:
- BLE text commands and USB Serial at 115200 baud.
- Latched forward/back driving, with the electrical mapping reversed for the motor wiring.
- One 200 ms left/right steering pulse at approximately 50% PWM duty, then steering off.
- Integer speed_0 through speed_100; startup setting 100 with motors stopped.
- stop for both motors; BLE link-change stops and input-error handling.
- Command-state logs, not sensor measurements.

Same-side steering repeats are ignored until opposite or stop. USB unplug is not a detected stop; no drive watchdog exists yet. Earlier motor test sketches remain for reproducing the investigation.

## App and vehicle roadmap

| Capability | Status |
|---|---|
| ESP32 motor control over Bluetooth BLE | Test firmware and user hardware results recorded |
| USB Serial commands and logs | Implemented testing interface |
| Wi-Fi vehicle link | Selected transport; implementation and validation pending |
| App with simultaneous driving/steering | Planned |
| Sensor readings and status | Planned; sensor integration pending |
| GPS location and map/navigation | Planned |
| Waypoints and automated movement | Planned; localization, feedback and stopping checks required |
| Speed and distance traveled | Planned; measurement method/calibration pending |
| Buzzer, front ultrasonic sensor, white headlights | Planned |

The app should display actual measurements with freshness/validity status, distinguish manual and automatic modes, and provide an accessible stop. Navigation goals go to the vehicle; local control and obstacle/stop decisions must not depend on the app staying connected. See [app plan](documentation/ui/APP_PLAN.md) and [wireless plan](documentation/network/WIRELESS_LINK.md).

## Hardware and power

- Original ESP32-WROOM-32D development board; L293D; two existing 3–6 V brushed motors.
- Motor rail: 6 V target. Electronics rail: separate regulated 5 V into the verified development-board input and driver logic.
- New selected allocation: 1,000 mAh rechargeable cell for motors; smaller electronics cell, previously identified as 500 mAh, exact label to confirm.
- Two separate boost stages and switched supply branches; protection, charging and portable load capability require verification.
- Common protected load ground; positive rails remain separate. Motor return current must not run through ESP32 PCB traces.
- The 1,000 mAh capacity is not a discharge-current rating. See [battery allocation](documentation/power/BATTERY_REALLOCATION.md).

The bench success does not resolve earlier unintended motor power with its supply switch off, charger heating, or all mixed USB/battery power paths. See the limitations before extending the build.

## Documentation

| Topic | Reference |
|---|---|
| Current decisions | [Decision record](documentation/DECISIONS.md) |
| Architecture | [Vehicle plan](documentation/architecture/VEHICLE_V2.md) |
| Signal and power block diagrams | [Diagrams](diagrams/README.md) |
| Future control/telemetry app | [App roadmap](documentation/ui/APP_PLAN.md) |
| Wi-Fi/Bluetooth roles and protocol status | [Wireless plan](documentation/network/WIRELESS_LINK.md) |
| Power terminals | [Power pinout](documentation/pin-configuration/POWER_PINOUT.md) |
| Latest battery allocation | [Battery decision](documentation/power/BATTERY_REALLOCATION.md) |
| Working movement milestone | [Results](documentation/testing/DRIVING_STEERING_MILESTONE.md) |
| Observations and sequential tests | [Test log](documentation/testing/TEST_LOG.md) |
| Steering/source comparison | [Test matrix](documentation/testing/STEERING_TEST_MATRIX.md) |
| Motor noise/load investigation | [Investigation](documentation/testing/MOTOR_NOISE_AND_LOAD.md) |
| Reasoning and engineering reflection | [Reflection](documentation/testing/STEERING_REFLECTION.md) |
| Firmware and commands | [Testing guide](firmware/testing/README.md) |
| Serial logs | [Serial guide](documentation/testing/SERIAL_CONTROL.md) |
| MATLAB/Octave assumptions and examples | [Simulations](simulations/README.md) |
| Remaining work | [Progress](progress/PROGRESS.md) |
| Constraints and unverified behavior | [Limitations](limitations/LIMITATIONS.md) |
| Current documentation revision | [V2.8.0](versions/V2.8.0.md) |

## Modeling and evidence

Simulation outputs depend on assumed parameters and do not prove motor behavior. For example, the initial assumed 65 mm wheel and 120 wheel RPM model approached 0.41 m/s; this was not measured car speed. Preserve the test log's distinction between observations, hypotheses, host code tests, and measured hardware results.
