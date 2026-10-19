# Firmware status

Target: original ESP32-WROOM-32D vehicle.

- [Current BLE + USB Serial motor sketch](testing/DualMotorV2_BLE_Pulse/DualMotorV2_BLE_Pulse.ino): 200 ms steering pulses, latched drive, test commands and logs.
- [Command/wiring guide](testing/README.md).
- [Serial guide](../documentation/testing/SERIAL_CONTROL.md).
- [Vehicle integration](vehicle-esp32/README.md).
- [Future Wi-Fi/Bluetooth app](../documentation/ui/APP_PLAN.md).

Wi-Fi, sensor integration, navigation/waypoints, actual speed/distance telemetry and automatic behavior are not implemented in the current motor test. Earlier motor-only sketches remain as test comparisons; they are not the current controller.

User-reported driving/steering success and host code tests are documented separately. Software stop is not physical power isolation.
