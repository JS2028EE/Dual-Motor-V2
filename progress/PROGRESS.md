# Current V2 progress

## ESP32-only vehicle redesign — October 16, 2026

- [x] Replace damaged-C3 dependency with ESP32-WROOM-32D vehicle plan.
- [x] Select phone/computer wireless-control direction; protocol remains pending.
- [x] Document drive motor plus steering motor arrangement.
- [x] Document L293D, GPS, ultrasonic, buzzer, and white headlights.
- [x] Draft signal and power block diagrams.
- [x] Record battery/converter/capacitor inventory.
- [x] Run initial hypothetical movement script in Octave (user screenshot).
- [x] Add simulation assumptions, electrical estimates, and corrected speed-axis label.
- [ ] Complete fit inspection and weigh the complete vehicle.
- [ ] Measure wheel diameter/RPM and identify motor gearing.
- [ ] Inspect steering centering and feedback.
- [ ] Verify motor starting/running current against driver and converter limits.
- [ ] Verify battery chemistry, capacity, discharge rating, and loaded voltage.
- [ ] Test candidate 6 V boost rail and driver heat.
- [ ] Finalize rechargeable electronics supply, charging/protection, and switches.
- [ ] Confirm sensor/buzzer models and capacitor values.
- [ ] Assign ESP32 pins and draw a verified wiring schematic.
- [ ] Run the updated saved Octave script.
- [ ] Build a movement model that responds to voltage, torque, and mass.
- [ ] Implement ESP32 vehicle firmware and manual-control app/protocol.
- [ ] Test stop behavior, obstacle sensing, and automatic mode.

## Historical controller progress

The following checklist records the earlier C3/S3 design. It is retained as history; its pending tasks are outside the active vehicle plan.

# Progress

## Phase 1 — Controller

- [x] Project created
- [x] Controller-first scope
- [x] C3 pin reference
- [x] S3 specifications
- [x] Landscape orientation
- [x] Cyberpunk UI direction
- [x] Navigation joystick role
- [x] Driving joystick role
- [x] Switch functions
- [x] C3/S3 UART pins identified as legacy wiring
- [x] Display bring-up
- [x] Adafruit_GFX + Adafruit_ILI9341 display firmware
- [x] Landscape orientation corrected
- [x] Full-screen redraw flicker removed
- [x] Live joystick display path (30 ms display refresh)
- [x] 20 ms joystick telemetry
- [x] Non-blocking TCP receive parser
- [x] TCP no-delay mode and Wi-Fi auto-reconnect
- [x] Navigation/action feedback added
- [x] Wi-Fi AP on S3
- [x] Wi-Fi station on C3
- [x] TCP controller protocol
- [x] Automatic Wi-Fi/TCP reconnect logic
- [ ] Physical Wi-Fi link test
- [ ] Verify long-duration Wi-Fi stability after V2.2.0
- [ ] Navigation joystick physical calibration
- [ ] Driving joystick physical calibration
- [ ] Full controller integration test
- [ ] Wi-Fi range/reliability test

## Phase 2 — Vehicle

Deferred.

