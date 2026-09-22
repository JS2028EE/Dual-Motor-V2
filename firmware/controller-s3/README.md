# Controller S3 Firmware

ESP32-S3 ES3C28P firmware for the controller display/network board.

## Responsibilities

- Create the controller's dedicated Wi-Fi access point
- Accept the C3 TCP connection
- Receive joystick telemetry/events
- Send PING/IDENTIFY commands
- Display controller link status
- Render the 320x240 landscape cyberpunk UI
- Keep vehicle status explicitly NOT CONNECTED until vehicle integration exists

## Display libraries

- Adafruit_GFX
- Adafruit_ILI9341

TFT_eSPI and User_Setup.h are not required by the current firmware.

## Network

- Wi-Fi mode: Access Point
- SSID: `DUAL-MOTOR-V2`
- Password: `DMV2-CTRL`
- AP IP: `192.168.4.1`
- TCP port: `4210`

The previous UART controller link is no longer active.
