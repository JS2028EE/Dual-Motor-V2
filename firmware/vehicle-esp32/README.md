# Vehicle integration

The original ESP32-WROOM-32D is the car controller. Use the current [motor test sketch](../testing/DualMotorV2_BLE_Pulse/DualMotorV2_BLE_Pulse.ino) and [guide](../testing/README.md).

BLE and USB Serial control are implemented for testing; 200 ms steering and bench-powered driving/steering success are user-reported. Wi-Fi firmware and the dedicated app remain planned.

Next integration covers simultaneous app controls, sensing, GPS/map/navigation, automation, waypoints, actual speed and distance traveled, buzzer and white headlights. See [app roadmap](../../documentation/ui/APP_PLAN.md) and [wireless plan](../../documentation/network/WIRELESS_LINK.md).

Portable power, command watchdog, local stopping, steering feedback/limits, electrical isolation and sensor measurements need validation before treating this as complete vehicle firmware.
