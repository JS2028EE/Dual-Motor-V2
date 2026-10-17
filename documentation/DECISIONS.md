# Current design decisions

Updated October 17, 2026. The [block diagrams](../diagrams/README.md) define the active architecture. This decision record clarifies them; bench-test wiring is not a replacement design.

| Area | Active decision |
|---|---|
| Controller | One original ESP32-WROOM-32D development board |
| C3 | Excluded; damaged board will not be used |
| S3 handheld/display controller | Outside this build; earlier firmware retained as historical reference |
| Commands | Phone or computer over Wi-Fi or Bluetooth; transport/app not finalized |
| Motors | One forward/reverse drive motor and one steering motor, both existing 3–6 V brushed DC |
| Driver | L293D, not L298N |
| Motor power | Three 1.5 V cells in series -> on/off switch -> dedicated boost at 6 V -> L293D pin 8 |
| Electronics power | One compatible rechargeable cell -> protected TP4056 OUT -> on/off switch -> separate boost at 5 V |
| 5 V loads | Verified ESP32 development-board input, L293D pin 16, and compatible accessories |
| 3.3 V loads | Compatible low-current loads from the ESP32 board regulator within its current budget |
| Ground | Common load ground is charger OUT-, not an external bridge to raw cell B- |
| Charging | Single compatible Li-ion/LiPo cell only; both load switches off; charge-current/protection verification pending |
| Switching | Separate on/off function per positive supply; reported three-terminal switch is likely SPDT used as SPST |
| Sensors/accessories | Front ultrasonic sensor, GPS, buzzer, and two white headlights with separate LED resistors |
| Deferred parts | Magnetic sensor until a defined purpose exists |
| GPIOs | Enable/direction through ESP32 GPIOs eventually; no final assignments or firmware yet |
| Simulation | Keep baseline plus new power/control example; label hypothetical outputs clearly |

## Bench wiring versus final wiring

USB currently supplied the ESP32 during the reported motor-driver bench test. The ESP32 board's 5 V rail supplied L293D logic and fixed HIGH test inputs. This does not replace the final rechargeable/5 V boost electronics branch.

Fixed 5 V test jumpers must be removed from control inputs before GPIO connection. The final logic supply stays on L293D pin 16; motor supply stays on pin 8. The motor should not be powered through ESP32 GPIOs.

## Pending verification

The user accepted two boost converters as the selected architecture. Actual converter identities, quantity, load capability, and full battery-powered integration remain to be verified. No 8 V motor supply was adopted. GPIO control and software stop do not resolve the unidentified switch-off motor-power path by themselves.

A future GPS/map/API web interface may provide telemetry and control presentation, consistent with phone/computer access. No website, API integration, or working map is established by this repository update.

## History

V2.0.0–V2.2.0 and controller-c3/controller-s3 directories describe an abandoned handheld-controller approach. Their presence is not a dependency or instruction to build that design.
