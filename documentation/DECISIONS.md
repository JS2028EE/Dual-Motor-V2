# Current design decisions

Updated October 17, 2026. The [block diagrams](../diagrams/README.md) define the active architecture. This decision record clarifies them; bench-test wiring is not a replacement design.

| Area | Active decision |
|---|---|
| Controller | One original ESP32-WROOM-32D development board |
| C3 | Excluded; damaged board will not be used |
| S3 handheld/display controller | Outside this build; earlier firmware retained as historical reference |
| Commands | BLE Controller app selected for first motor test; Nordic UART text protocol recorded; full phone/computer integration unvalidated |
| Motors | One forward/reverse drive motor and one steering motor, both existing 3–6 V brushed DC |
| Driver | L293D, not L298N |
| Motor power | New selection: 1,000 mAh rechargeable cell -> protection/charging arrangement to verify -> switch -> dedicated 6 V boost -> L293D pin 8; portable load test pending |
| Electronics power | Smaller rechargeable cell (previously 500 mAh; label to confirm) -> compatible protected charger OUT -> switch -> separate 5 V boost |
| 5 V loads | Verified ESP32 development-board input, L293D pin 16, and compatible accessories |
| 3.3 V loads | Compatible low-current loads from the ESP32 board regulator within its current budget |
| Ground | Common load ground is charger OUT-, not an external bridge to raw cell B- |
| Charging | Single compatible Li-ion/LiPo cell only; both load switches off; charge-current/protection verification pending |
| Switching | Separate on/off function per positive supply; reported three-terminal switch is likely SPDT used as SPST |
| Sensors/accessories | Front ultrasonic sensor, GPS, buzzer, and two white headlights with separate LED resistors |
| Deferred parts | Magnetic sensor until a defined purpose exists |
| GPIOs | Motor test: GPIO25/26/27 for drive, GPIO14/32/33 for steering; complete car mapping/integration pending |
| Simulation | Preserve all three hypothetical MATLAB/Octave models; label assumed outputs clearly |

## Bench wiring versus final wiring

USB supplied the ESP32 during earlier reported motor-driver bench tests. Battery-powered electronics switch/LED power-up was subsequently reported. The ESP32 board's 5 V rail supplied L293D logic and fixed HIGH test inputs. This does not replace the final rechargeable/5 V boost electronics branch.

Fixed 5 V test jumpers must be removed from control inputs before GPIO connection. The final logic supply stays on L293D pin 16; motor supply stays on pin 8. The motor should not be powered through ESP32 GPIOs.

## Pending verification

The user accepted two boost converters as the selected architecture. Actual converter identities, quantity, load capability, and full battery-powered integration remain to be verified. No 8 V motor supply was adopted. GPIO control and software stop do not resolve the unidentified switch-off motor-power path by themselves.

A future GPS/map/API web interface may provide telemetry and control presentation, consistent with phone/computer access. No website, API integration, or working map is established by this repository update.

## History

V2.0.0–V2.2.0 and controller-c3/controller-s3 directories describe an abandoned handheld-controller approach. Their presence is not a dependency or instruction to build that design.

## Earlier latched-steering test

The user requested integer speed_N values, initial 100 drive duty, independently latched driving and steering, and lowercase stop for both motors. These behaviors are recorded as [testing code](../firmware/testing/README.md), not validated production control. No steering limit cutoff or command timeout is present in the latest latched version. Drive buzzing near30, steering noise, and drive slowdown during steering remain unresolved. The latest reported supply-capacitor experiment names 1,000 uF/50 V; location/polarity are unverified.

## Earlier pulse-steering revision — October 17

Use the [pulse-steering sketch](../firmware/testing/DualMotorV2_BLE_Pulse/DualMotorV2_BLE_Pulse.ino). Drive remains latched, with its electrical forward/back mapping reversed for the user's motor wiring. Left/right trigger one150 ms pulse at unchanged steering duty/frequency, then disable steering. Repeats in the same direction are ignored until opposite or stop. The timing is uncalibrated; no active position hold or limit feedback exists. No voltage increase, power-architecture change, or verified noise/energy improvement is recorded.


## Latest decision and successful combined test

User reports successful simultaneous driving/steering with 200 ms pulses and 6 V bench source at 0.60 A limit; approximately 0.55 A max observed on its display. See [milestone](testing/DRIVING_STEERING_MILESTONE.md). Current firmware supports USB Serial plus BLE, fixed ~50% steering duty and 200 ms cutoff; no automatic drive-priority or website/app was added. Select 1,000 mAh cell for motors and smaller cell for electronics; see [battery allocation](power/BATTERY_REALLOCATION.md). Do not infer discharge capability from mAh. Portable source, cell protection/charging integration, transient/thermal validation and earlier switch-off power path remain open. Better simultaneous control UI and GPS speed/map are deferred next work; speed-control policy remains undecided.
