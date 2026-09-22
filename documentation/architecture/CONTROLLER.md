# Controller Architecture

Phase 1 is intentionally limited to the handheld controller.

## ESP32-C3 SuperMini

The C3 reads both joysticks and their switches, performs local input processing, and sends controller telemetry/events to the S3 over Wi-Fi TCP.

### Navigation joystick

- X/Y = menu navigation
- Short press = Select
- Hold = Back

### Driving joystick

- X/Y = driving input
- Short press = Brake
- Hold = Emergency Stop

## ESP32-S3 ES3C28P

The S3 creates the dedicated controller Wi-Fi network, accepts the C3 TCP connection, and renders the 2.8-inch ILI9341V landscape cyberpunk UI.

Capacitive touch exists on the hardware but is intentionally not used.

## Controller network

The S3 operates as a Wi-Fi access point:

- SSID: DUAL-MOTOR-V2
- Password: DMV2-CTRL
- IP: 192.168.4.1
- TCP port: 4210

The C3 operates as a Wi-Fi station and connects to the S3 AP.

This creates a direct controller network without requiring a home/router Wi-Fi network or Internet access.

## Transport

TCP is used because controller telemetry and button events need reliable, ordered delivery.

### C3 -> S3

- `READY,C3`
- `DEVICE,DUAL_MOTOR_V2_C3`
- `INPUT,navX,navY,driveX,driveY`
- `EVENT,NAV_SELECT`
- `EVENT,NAV_BACK`
- `EVENT,DRIVE_BRAKE`
- `EVENT,DRIVE_ESTOP`
- `PONG,C3`

### S3 -> C3

- `PING`
- `IDENTIFY`

Telemetry is sent approximately every 50 ms while the TCP link is available.

## Legacy UART

The previously documented C3 GPIO20/GPIO21 to S3 GPIO43/GPIO44 UART wiring remains physically documented, but the current firmware does not use it. The active controller communication path is Wi-Fi.

## Link recovery

The C3 periodically retries Wi-Fi and TCP connection establishment.

The S3 detects stale controller traffic and changes the UI to a Wi-Fi waiting/offline state when the C3 stops sending packets.

## Vehicle boundary

The vehicle is not connected yet. The S3 continues to display **VEHICLE: NOT CONNECTED** rather than fabricating vehicle telemetry.
