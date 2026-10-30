# Dual Motor V2 — BLE Controller

A small browser-based remote for the current ESP32-WROOM-32D BLE test firmware. It sends the existing text commands directly over Bluetooth Low Energy; it does not use Wi-Fi, a server, a cloud service, or a new firmware protocol.

## Open it

1. Open this folder with VS Code.
2. Serve it from a secure context: GitHub Pages HTTPS, or http://localhost.
3. Open the page in Chrome or Edge on a supported desktop or Android device.
4. Power the car, select Connect BLE, and choose DualMotorV2.
5. Use the direction buttons. Use STOP to clear both motor outputs.

For a quick local test, from this folder run: python -m http.server 8000, then open http://localhost:8000. For phone use, publish it over HTTPS (for example, GitHub Pages).

## Firmware compatibility

Uses the BLE Nordic UART service in firmware/testing/DualMotorV2_BLE_Pulse/DualMotorV2_BLE_Pulse.ino.

- Service: 6e400001-b5a3-f393-e0a9-e50e24dcca9e
- RX write: 6e400002-b5a3-f393-e0a9-e50e24dcca9e
- TX notifications: 6e400003-b5a3-f393-e0a9-e50e24dcca9e

Commands sent exactly as the firmware expects:
- forward, back: latched drive direction.
- left, right: one 200 ms steering pulse. Same-side repeats are suppressed until opposite direction or STOP.
- speed_0 through speed_100: integer PWM duty setting.
- stop: disables both motor bridges and clears pending movement.

The UI sends one command per click. It does not repeatedly transmit while a button is held. Drive remains latched until another drive command or STOP is sent.

## Important limits

- This is a browser UI for current test firmware, not a validated vehicle controller.
- Web Bluetooth works in supported Chromium browsers (Chrome/Edge) on supported desktop and Android devices. It is not supported by iOS Safari or Firefox. The page must run on HTTPS or localhost.
- BLE disconnect should trigger the firmware stop behavior. A browser tab closing or phone losing focus is not a substitute for an accessible physical power switch.
- The firmware currently has no drive watchdog. A lost control session can leave the last drive command latched until the firmware detects a BLE disconnect or receives STOP. Keep the car supervised, test with the wheels raised first, and keep the physical power switch accessible.
- On-screen response is firmware command/status text, not measured speed, distance, voltage, current, steering angle, GPS, or sensor telemetry.
- No Wi-Fi, map, GPS, ultrasonic, or autonomous-navigation feature is implemented here.

## Files

- index.html — control layout
- style.css — responsive styling
- app.js — Web Bluetooth connection and command handling
