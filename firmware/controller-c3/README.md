# Controller C3 Firmware

ESP32-C3 SuperMini firmware for the handheld controller input board.

## Responsibilities

- Read navigation joystick X/Y
- Read driving joystick X/Y
- Read both joystick push switches
- Apply deadzone and axis normalization
- Generate Select, Back, Brake, and Emergency Stop events
- Connect to the S3 over Wi-Fi
- Send controller telemetry/events over TCP
- Reconnect automatically after link loss

## Network

- Wi-Fi mode: Station
- SSID: `DUAL-MOTOR-V2`
- S3 IP: `192.168.4.1`
- TCP port: `4210`
- Telemetry interval: approximately 50 ms

The previous UART implementation is no longer active.
