# Dual Motor V2 — Final Controller Pinout

## Status

**Controller wiring soldered and documented.**

| Function | Device | C3 GPIO | Board Label |
|---|---|---:|---|
| Navigation X | Nav joystick VRX | GPIO0 | A0 |
| Navigation Y | Nav joystick VRY | GPIO1 | A1 |
| Driving X | Drive joystick VRX | GPIO3 | A3 |
| Driving Y | Drive joystick VRY | GPIO4 | A4 |
| Navigation SW | Nav joystick SW | GPIO6 | GPIO6 |
| Driving SW | Drive joystick SW | GPIO7 | GPIO7 |
| UART TX | ES3C28P RX | GPIO20 | TX |
| UART RX | ES3C28P TX | GPIO21 | RX |
| Joystick VCC | Both | 3V3 | 3V3 |
| Joystick GND | Both | GND | GND |

## ES3C28P UART

| Function | S3 GPIO | Connected to |
|---|---:|---|
| UART RX | GPIO43 | C3 GPIO20 TX |
| UART TX | GPIO44 | C3 GPIO21 RX |
| GND | GND | C3 GND |

UART signaling is 3.3 V logic. **Legacy note:** the current controller firmware no longer uses this UART link; C3/S3 communication now occurs over Wi-Fi TCP. The physical UART wiring may remain soldered but is inactive in firmware.

## Switch behavior

- Navigation short press: Select
- Navigation hold: Back
- Driving short press: Brake
- Driving hold: Emergency Stop

## Physical mounting

Both joysticks are mounted sideways. Firmware includes configurable axis swap/inversion settings so forward/backward and left/right can be calibrated from the actual soldered hardware.

## Soldering record

The controller input and UART wiring has been soldered according to this final pinout.

Vehicle wiring remains deferred until controller testing is complete.
