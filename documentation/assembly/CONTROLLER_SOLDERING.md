# Controller Soldering Record

## Status

**Soldering completed for the controller wiring.**

Completed connections:
- Navigation VRX -> C3 GPIO0 / A0
- Navigation VRY -> C3 GPIO1 / A1
- Navigation SW -> C3 GPIO6
- Driving VRX -> C3 GPIO3 / A3
- Driving VRY -> C3 GPIO4 / A4
- Driving SW -> C3 GPIO7
- Both joystick VCC -> C3 3V3
- Both joystick GND -> C3 GND
- C3 GPIO20 TX -> S3 GPIO43 RX
- C3 GPIO21 RX <- S3 GPIO44 TX
- C3 GND <-> S3 GND

## Firmware

The C3 firmware reads four analog joystick axes, applies a deadzone, debounces the two switches, detects short/hold actions, and sends telemetry over UART.

The S3 firmware renders the 320x240 landscape cyberpunk engineering-console UI and receives the C3 telemetry.

## Next test

1. Power controller.
2. Confirm S3 display boots.
3. Confirm UART connection.
4. Move both joysticks and verify X/Y.
5. Test Select, Back, Brake, and Emergency Stop events.
6. Adjust axis swap/inversion flags if needed.

Vehicle motor-control firmware is intentionally not included in this controller stage.
