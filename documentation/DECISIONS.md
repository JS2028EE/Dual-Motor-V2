# Current design decisions

Updated October 19, 2026. See [block diagrams](../diagrams/README.md), [app plan](ui/APP_PLAN.md) and [wireless plan](network/WIRELESS_LINK.md).

| Area | Decision/status |
|---|---|
| Vehicle controller | One original ESP32-WROOM-32D development board |
| Client | Phone/computer app, to be developed |
| Communication | Wi-Fi and Bluetooth selected; BLE and USB Serial test control exist, Wi-Fi pending |
| App scope | Simultaneous drive/steering, navigation/map, sensing, automation, waypoints, distance traveled, speed and diagnostics |
| Driver/motors | L293D; one drive and one steering motor, existing 3–6 V brushed DC |
| Current firmware | BLE + USB Serial; latched driving, reversed drive polarity, 200 ms steering pulses, fixed steering duty 128/255 at 1 kHz |
| Drive speed | Startup setting 100 with motors stopped; integer speed_N supported; future app speed policy undecided |
| Stop | stop disables both bridges; BLE link changes and input errors stop; drive watchdog and USB-unplug detection absent |
| Bench result | User reports successful simultaneous driving/steering at 6 V, 0.60 A limit; supply display observed up to ~0.55 A |
| Motor battery | 1,000 mAh rechargeable cell selected; protection/charging/discharge and boost validation pending |
| Electronics battery | Smaller cell, previously 500 mAh; exact label to confirm |
| Power | Separate 6 V motor and 5 V electronics boost rails; common protected load returns; positive rails separate |
| Charging | Compatible single-cell charger/protection per cell arrangement; motor-cell details pending; no combined-cell or charge-and-run design |
| Accessories | GPS, front ultrasonic sensor, buzzer, two white headlights; other sensors only with a defined purpose |
| Motor GPIOs | Drive 25/26/27, steering 14/32/33; two separate 10k enable pull-downs to GND |
| Simulation | MATLAB/Octave examples are assumption-based; not hardware proof |

App and Wi-Fi work are requirements, not implemented features. Future speed/distance need actual measurements; PWM is not measured velocity. Waypoints require validated localization, control, sensing and stopping behavior.

USB motor testing and direct bench motor power are temporary test arrangements. The new battery allocation is a selected portable design, not a validated replacement for the bench supply. Keep prior tests and observations in the [test log](testing/TEST_LOG.md).

The earlier motor-running-with-switch-off path, charger heat, mixed USB/external power isolation and thermal/load margins remain unresolved. A working movement test does not resolve them. See [limitations](../limitations/LIMITATIONS.md).
