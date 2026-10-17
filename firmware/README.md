# Firmware status

**Active target: ESP32-WROOM-32D vehicle only. No C3/S3 controller.**

- [BLE testing sketches and command guide](testing/README.md): latest latched-control behavior and earlier timed version.
- [Vehicle integration status](vehicle-esp32/README.md): GPS, ultrasonic, lights, buzzer, and automatic mode remain pending.
- [Motor-noise/load investigation](../documentation/testing/MOTOR_NOISE_AND_LOAD.md): current unresolved bench observations.

Test firmware exists, but no complete validated vehicle controller is released. BLE software stop is not physical power isolation. Earlier controller-c3/controller-s3 files are historical only.
