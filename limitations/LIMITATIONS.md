# Current limitations

Updated October 17, 2026. This file covers the ESP32-only car in the [block diagrams](../diagrams/README.md). Earlier C3/S3 limitations are retained separately as [historical notes](../documentation/history/CONTROLLER_LIMITATIONS.md).

## Observed and unresolved

| Issue | Evidence | Status |
|---|---|---|
| Motor runs with motor switch off | User's USB-powered bench sequence; speed increases when motor switch turns on | Alternate power path unidentified; switch alone is not verified isolation |
| Motor slower through L293D | Visual estimate of roughly 50% less speed than direct 6 V | No RPM, motor voltage, or current measurement; cause unconfirmed |
| Electronics power-up | User reports working switch and ESP32 LED after correcting underside battery/wire overlap | Functional report only; precise fault and loaded voltages not verified |
| Charger very hot | User report; B+/OUT+ bridge removed afterward | No verified root cause, charge-current check, or thermal retest |
| Multimeter unreliable | Reported 0.00 V on known source and fluctuating mV readings | Exact meter/model, sockets, leads, range, and contacts need checking |
| Audible motor noise | Steering noise; drive noise around speed 30; latest 1,000 uF/50 V experiment did not remove it | PWM tone/stall hypotheses; no confirmed fix |
| Drive slows during steering | User report during left/right commands | Shared supply/driver loading suspected; voltage/current unmeasured |
| Driver held enabled in earlier test | Pin1 tied to fixed 5 V before GPIO control | Historical condition; GPIO test code now recorded, isolation still unresolved |

A fixed 5 V jumper is a valid logic HIGH. Setting a future enable GPIO LOW should disable the intended output stage, but does not diagnose the switch-off power path or prove hardware shutdown.

## Electrical design

- L293D is the selected driver; no L298N substitution is planned.
- L293D rated continuous current is up to 600 mA per channel, subject to thermal limits. Actual motor starting/stall current remains unknown.
- Driver voltage loss varies with load; the assumed 2 V loss is not a measured value or guarantee. Neither extra boost voltage nor a capacitor removes driver limits.
- Motor rail stays at the selected 6 V target. An 8 V increase was discussed but not adopted.
- Motor-pack 4.68 V and electronics-cell 3.65 V are user reports; reliable loaded readings are still needed.
- Two independent boost stages are selected. Actual devices, capacity, regulation, and electrical performance of the reported battery-powered electronics branch are unverified.
- USB powered the ESP32 in the reported bench tests. Do not assume USB and external 5 V may be connected together without checking the exact development board's isolation.
- The three-terminal switch is likely SPDT used as an SPST function. Its common/outer terminals and current rating need verification.
- Power-off does not disconnect ESP32 GND. Common load returns use charger OUT-, not an external B-/OUT- bridge.
- Charger protection, charge current, battery chemistry/rating, and wiring require verification. Charging occurs with both load switches off; no load-sharing design exists.
- A 3.7 V cell can reach 4.2 V; do not feed it directly into ESP32 3V3. A resistor divider is not an ESP32 power regulator.
- Capacitor transient filtering is not validated. Reported 22 pF and 104 pF parts are not selected for motor/supply suppression.
- Adequate local bypass capacitors, accessory drivers, rail current budgets, and wiring/switch ratings remain to be checked.

## Firmware and sensing

- No C3 or S3 controller is used; their retained files are historical only.
- BLE motor-control TEST sketches and selected six-GPIO test mapping are now recorded; no complete validated vehicle firmware, autonomous sensing, or production app integration exists.
- Fixed 5 V control jumpers must be removed before GPIO connection. GPIOs must not receive 5 V.
- Enable pull-downs, startup/disconnect/stop behavior and mixed-power states are not electrically validated. Current test code latches DRIVE with NO command timeout; steering now has a 150 ms pulse limit and same-side-repeat guard, but no physical end-limit feedback. BLE stop is software, not hardware isolation.
- Steering feedback, centering, limits, and safe actuation duration remain unknown. Do not hold a steering motor against an end stop.
- Exact ultrasonic/GPS/buzzer models, module supply voltages, and ultrasonic Echo level protection need confirmation.
- GPS provides location; it does not replace front obstacle sensing. GPS/map/API work is a future integration, not a verified feature here.

## Simulation and mechanics

- Both Octave examples use assumed wheel size/RPM, motor current, driver drop, and efficiency.
- The baseline movement graph is independent of voltage/current. The latest model adds only an assumed voltage-to-RPM relation; it has no mass, torque, gearing, friction, or validated braking model.
- Constant enabled motor current in the latest model is a simplification, not a PWM/current prediction.
- Backfeed, charger operation, battery sag, converter idle current, capacitor transients, and startup/stall surges are omitted.
- Runtime estimates are for the electronics cell only, with assumed efficiency/usable capacity. Motor-pack runtime is unknown.
- Wheel dimensions/RPM, complete mass, driving force, steering angle, stopping distance, range, obstacle avoidance, and floor-driving performance remain unmeasured/unvalidated.

See [test log](../documentation/testing/TEST_LOG.md), [power plan](../documentation/pin-configuration/POWER_PINOUT.md), and [simulation notes](../simulations/README.md).

## New electronics-chain model

The third script models assumed steady-state resistive drops and boost power balance. It omits standby current, battery discharge curve, contact faults, protection cutoff, converter transients, and temperature. Its charger plot is a separate constant-current loss illustration with load off, not a full charging simulation. The 250 mA example is not a verified setting for this battery or board. See [simulation notes](../simulations/README.md).

## Recorded motor test firmware

See [BLE test code](../firmware/testing/README.md). Both channels use 1 kHz PWM; steering remains near 50% duty regardless of drive slider. A 1,000 uF/50 V electrolytic was reported in the latest unsuccessful noise experiment; placement was not verified. Previous continuous steering may have stalled against a stop. The new timed pulse limits energized duration but can still reach a stop early; position retention after disable is unverified. See [motor-noise investigation](../documentation/testing/MOTOR_NOISE_AND_LOAD.md).

## Pulse-steering revision limits

The user's stall explanation is plausible, not a confirmed physical diagnosis. Current [pulse test code](../firmware/testing/DualMotorV2_BLE_Pulse/DualMotorV2_BLE_Pulse.ino) reverses drive polarity and keeps forward/back latched. Steering runs for150 ms at unchanged approximately 50% duty then disables; repeated same-side messages cannot extend it. That is open-loop timing, not position control. Wheels may spring back or move after disable. Same-side re-trigger requires opposite command or stop. Hardware validation, actual travel time, current savings, noise reduction, and shared-load performance remain pending. Host control tests passed; actual ESP32 build/radio tests were not performed.


## Latest combined test and battery transition — October 17

User reports successful driving/steering with 200 ms pulses on a 6 V bench supply limited to 0.60 A, with displayed current up to about 0.55 A. This supersedes earlier statements that practical combined steering has not yet worked; it does not establish transient current, thermal endurance, actual regulated voltage during every event, or all surfaces/loads. New portable selection is a 1,000 mAh motor cell and smaller electronics cell; cell discharge/protection/charge compatibility and converters remain unvalidated. The battery-side current can exceed the 6 V output current. Exact smaller-cell capacity and final capacitor installation remain unconfirmed. No app/site/map/GPS speed or autonomous behavior has been implemented. Earlier switch-off motion and charger heating remain unresolved. See [milestone](../documentation/testing/DRIVING_STEERING_MILESTONE.md).
