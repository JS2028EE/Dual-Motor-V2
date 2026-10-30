# Current design decisions

Updated October 19, 2026. See [block diagrams](../diagrams/README.md), [app plan](ui/APP_PLAN.md) and [wireless plan](network/WIRELESS_LINK.md).

| Area | Decision/status |
|---|---|
| Vehicle controller | One original ESP32-WROOM-32D development board |
| Client | Custom browser-based BLE controller website created and user-tested; future sensor/map interface still to be developed |
| Communication | Wi-Fi and Bluetooth selected; BLE and USB Serial test control exist, Wi-Fi pending |
| App scope | Website currently provides manual BLE movement controls; sensors, navigation/map, automation, waypoints, distance and telemetry remain future work |
| Driver/motors | L293D; one drive and one steering motor, existing 3–6 V brushed DC |
| Current firmware | BLE + USB Serial; latched driving, reversed drive polarity, 200 ms steering pulses, fixed steering duty 128/255 at 1 kHz |
| Drive speed | Startup setting 100 with motors stopped; integer speed_N supported; future app speed policy undecided |
| Stop | stop disables both bridges; BLE link changes and input errors stop; drive watchdog and USB-unplug detection absent |
| Bench result | User reports successful simultaneous driving/steering at 6 V, 0.60 A limit; supply display observed up to ~0.55 A |
| Motor battery | Separate 3.7 V, 1,000 mAh rechargeable cell with boost converter; user reports successful driving/steering; electrical validation pending |
| Electronics battery | Separate 3.7 V, 500 mAh rechargeable cell with dedicated boost to 5 V ESP32 input; user reports functional power-up, electrical validation pending |
| Power | Separate 6 V motor and 5 V electronics boost rails; common protected load returns; positive rails separate |
| Charging | Compatible single-cell charger/protection per cell arrangement; motor-cell details pending; no combined-cell or charge-and-run design |
| Accessories | GPS, front ultrasonic sensor, buzzer, two white headlights; other sensors only with a defined purpose |
| Motor GPIOs | Drive 25/26/27, steering 14/32/33; two separate 10k enable pull-downs to GND |
| Simulation | MATLAB/Octave examples are assumption-based; not hardware proof |

App and Wi-Fi work are requirements, not implemented features. Future speed/distance need actual measurements; PWM is not measured velocity. Waypoints require validated localization, control, sensing and stopping behavior.

The portable battery arrangement and custom BLE website have now been user-tested for driving and steering; see [Test 15](testing/TEST_LOG.md) and [battery allocation](power/BATTERY_REALLOCATION.md). Loaded voltage/current, protection, charging, thermal margins, runtime and the earlier motor-switch-off power path remain to be verified. The previous ~550 mA bench display observation is not a confirmed 600 mA continuous motor requirement. The next phase is sensor integration and testing, GPS/geolocation mapping, automation, and a final integrated driving test.

The earlier motor-running-with-switch-off path, charger heat, mixed USB/external power isolation and thermal/load margins remain unresolved. A working movement test does not resolve them. See [limitations](../limitations/LIMITATIONS.md).
