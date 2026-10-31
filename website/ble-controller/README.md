# Dual Motor V2 — BLE Controller

A browser-based remote for the ESP32-WROOM-32D BLE test firmware, including motor controls, HC-SR04 distance telemetry, and active-buzzer status/control.

## Open it

1. Open this folder with VS Code.
2. Serve it from a secure context: GitHub Pages HTTPS, or http://localhost.
3. Open the page in Chrome or Edge on a supported desktop or Android device.
4. Power the car, select Connect BLE, and choose DualMotorV2.
5. Use the direction buttons. Use STOP to clear both motor outputs.

For a quick local test, from this folder run: `python -m http.server 8000`, then open http://localhost:8000. For phone use, publish it over HTTPS (for example, GitHub Pages).

## BLE protocol

Uses the Nordic UART service in `firmware/testing/DualMotorV2_BLE_Pulse/DualMotorV2_BLE_Pulse.ino`.

- Service: `6e400001-b5a3-f393-e0a9-e50e24dcca9e`
- RX write: `6e400002-b5a3-f393-e0a9-e50e24dcca9e`
- TX notifications: `6e400003-b5a3-f393-e0a9-e50e24dcca9e`

Commands:
- `forward`, `back`: latched drive direction.
- `left`, `right`: one 200 ms steering pulse.
- `speed_0` through `speed_100`: integer PWM duty setting.
- `stop`: disables both motor bridges.
- `sensor_on` / `sensor_off`: enable or disable the HC-SR04.
- `buzzer_on` / `buzzer_off`: enable or disable audible distance alerts.

## Startup defaults and safety

- The HC-SR04 sensor and buzzer are **enabled by default at every boot**. The motors remain stopped at startup.
- The firmware performs a live sensor check. Until a fresh valid echo is received, forward drive is blocked.
- Forward drive is blocked when the sensor is disabled, the reading is invalid/stale, or an obstacle is at or inside 20 cm. Reverse and steering remain available.
- Obstacle stopping remains active even if the user turns the buzzer off.
- The active buzzer is silent beyond 100 cm, beeps progressively faster as an obstacle approaches, and sounds continuously at or inside 20 cm. “Enabled” means the warning system is armed, not that it should beep constantly.
- Telemetry is sent as short independent notifications to work with the default BLE notification payload size: `D:34.5`, `S:1`, `B:1`, `O:NEAR`. The browser also accepts the earlier verbose key/value format.
- `O:CLEAR`, `O:NEAR`, `O:STOP`, `O:WAIT`, and `O:OFF` describe sensor/obstacle state.

## Required hardware wiring

### HC-SR04
- TRIG → ESP32 GPIO18.
- ECHO → ESP32 GPIO19 **through a 1 kΩ / 2 kΩ divider** (the Echo pin can output 5 V; do not connect it directly to the ESP32).
- VCC → regulated 5 V; GND shared with ESP32.

### Active buzzer (5 V rated)
Use an NPN low-side transistor driver; do not power an unknown or high-current buzzer directly from an ESP32 GPIO:
- GPIO23 → 1 kΩ resistor → NPN base (2N2222 or BC337).
- 10 kΩ resistor from base to GND.
- Emitter → GND.
- Collector → buzzer negative.
- Buzzer positive → regulated 5 V.
- Share ground between the 5 V supply and ESP32.
- If the buzzer is magnetic/inductive, add a flyback diode across it (cathode to +5 V, anode to collector). Check the buzzer's voltage/current rating before connecting it.

## Important limits

- This remains test firmware, not a validated vehicle controller. Test with the wheels raised, supervise the car, and keep the physical power switch accessible.
- Software stop is not a hardware emergency stop or power-isolation device.
- There is no drive watchdog or steering position feedback.
- Web Bluetooth requires a supported Chromium browser and HTTPS or localhost; it is not supported by iOS Safari or Firefox.
- No GPS, Wi-Fi, map, or autonomous-navigation feature is implemented.

## Files

- `index.html` — control layout
- `style.css` — responsive styling
- `app.js` — Web Bluetooth connection, commands, and telemetry display
