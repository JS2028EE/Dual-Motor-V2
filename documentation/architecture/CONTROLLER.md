# Controller Architecture

Phase 1 is intentionally limited to the handheld controller.

## ESP32-C3 SuperMini

Reads both joysticks and their switches, handles local controller state, and communicates with the S3 over UART.

## ESP32-S3 ES3C28P

Runs the 2.8-inch ILI9341V display and renders the landscape cyberpunk UI. Capacitive touch exists on the hardware but is intentionally not used.

## Input assignments

Navigation joystick: movement navigates; short press selects; hold returns/back.

Driving joystick: movement becomes driving input; short press brakes; hold triggers emergency stop.

## UART

C3 GPIO20 TX -> S3 GPIO43 RX

C3 GPIO21 RX <- S3 GPIO44 TX

GND -> GND

Power is separate from UART signaling and follows the S3 board's documented electrical requirements.
