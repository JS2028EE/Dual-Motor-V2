# Wi-Fi and Bluetooth vehicle link

Updated October 19, 2026. A phone/computer app communicates directly with the ESP32-WROOM-32D vehicle.

| Link | Current state | Intended role |
|---|---|---|
| Bluetooth BLE | Nordic UART text command interface in current test sketch | Wireless manual control; future telemetry |
| Wi-Fi | Selected; not implemented in current sketch | App control, telemetry and navigation goals |
| USB Serial | Implemented at 115200 baud | Bench commands and commanded-state logs |

Bluetooth UUIDs: service 6e400001-b5a3-f393-e0a9-e50e24dcca9e; RX 6e400002-b5a3-f393-e0a9-e50e24dcca9e; TX 6e400003-b5a3-f393-e0a9-e50e24dcca9e. Current lowercase commands: forward, back, left, right, speed_0..speed_100 and stop. See [firmware guide](../../firmware/testing/README.md).

Wi-Fi access-point versus local-network station mode, endpoint/protocol, credentials, discovery and authentication are not selected. There is no configured vehicle Wi-Fi SSID, IP, HTTP server, navigation API or app download to use yet. Do not substitute a previous unrelated network configuration.

Before app integration, define exclusive control/session ownership, explicit stop/acknowledgement, packet freshness, command timeout and reconnect behavior. BLE/USB currently share motor state; BLE link changes stop both motors, USB removal does not, and drive has no watchdog. These limits must be addressed for the future UI.

Choose/test radio coexistence and power budget instead of assuming both links can be used concurrently without effects. No wireless range or latency guarantee is established. Local obstacle/stop processing belongs on the car even if the app plans waypoints or shows maps.

See [app roadmap](../ui/APP_PLAN.md) for sensing, automation, waypoints, distance and speed requirements. All telemetry must distinguish actual readings from commanded state and unavailable data.
