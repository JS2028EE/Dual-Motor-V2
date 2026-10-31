# Firmware status

Target: original ESP32-WROOM-32D vehicle.

- [Current BLE + USB Serial + HC-SR04 test sketch](testing/DualMotorV2_BLE_Pulse/DualMotorV2_BLE_Pulse.ino): 200 ms steering pulses, latched drive, front distance sensing, and a 20 cm forward obstacle interlock.
- [Command/wiring guide](testing/README.md).
- [HC-SR04 wiring and bring-up guide](testing/HC_SR04_INTEGRATION.md).
- [Serial guide](../documentation/testing/SERIAL_CONTROL.md).
- [Vehicle integration](vehicle-esp32/README.md).
- [Future Wi-Fi/Bluetooth app](../documentation/ui/APP_PLAN.md).

Wi-Fi, GPS/navigation/waypoints, actual speed/distance telemetry, BLE sensor telemetry, and autonomous navigation are not implemented. HC-SR04 sensing is now present in the current test sketch, but physical validation is pending. Earlier motor-only sketches remain as test comparisons; they are not the current controller.

User-reported driving/steering success and host code tests are documented separately. Software stop is not physical power isolation.
