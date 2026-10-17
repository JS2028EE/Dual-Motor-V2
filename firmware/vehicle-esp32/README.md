# ESP32 vehicle integration

Current board: original ESP32-WROOM-32D. Follow [block diagrams](../../diagrams/README.md).

The motor-only BLE [test sketches](../testing/README.md) now record the selected GPIO mapping, commands, drive PWM, steering PWM, and stop behavior. They are testing code, not complete vehicle firmware.

GPS, ultrasonic sensing, white headlights, buzzer, automatic mode, GPS/map/API integration, validated shutdown, and steering limit/position behavior remain pending. No C3/S3 controller is used.

Before integration, investigate [audible noise and shared-load slowdown](../../documentation/testing/MOTOR_NOISE_AND_LOAD.md), earlier unintended motor power, and power/thermal limits. Do not assume the latched steering test can be held against an end stop.
