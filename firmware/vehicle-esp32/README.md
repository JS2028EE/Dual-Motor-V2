# Vehicle integration

The original ESP32-WROOM-32D is the car controller. Use the current [motor test sketch](../testing/DualMotorV2_BLE_Pulse/DualMotorV2_BLE_Pulse.ino) and [guide](../testing/README.md).

BLE and USB Serial control are implemented for testing; the current sketch now also reads a front-facing HC-SR04 and blocks forward commands when the reading is <=20 cm or unavailable/stale. The 200 ms steering and bench-powered driving/steering success are user-reported. Physical sensor validation is pending; Wi-Fi firmware and the dedicated app remain planned.

Next integration covers BLE/website sensor telemetry, GPS/map/navigation, automation, waypoints, actual speed and distance traveled, buzzer and white headlights. See [app roadmap](../../documentation/ui/APP_PLAN.md) and [wireless plan](../../documentation/network/WIRELESS_LINK.md).

Portable power, command watchdog, local stopping, steering feedback/limits, electrical isolation and sensor measurements need validation before treating this as complete vehicle firmware.
