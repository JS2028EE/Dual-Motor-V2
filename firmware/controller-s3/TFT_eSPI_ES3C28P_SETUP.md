> **Historical controller document:** this file describes the earlier C3/S3 design. The October 16, 2026 plan uses an ESP32-WROOM-32D car with phone/computer control. See the [current project overview](https://github.com/JS2028EE/Dual-Motor-V2#readme). Earlier firmware and wiring here are not the current vehicle implementation.

# ES3C28P Display Firmware Setup

Target: LCDWIKI ES3C28P, ESP32-S3, ILI9341V, 240x320.

Firmware orientation is landscape (320x240).

## LCD pins

- CS = GPIO10
- DC = GPIO46
- SCK = GPIO12
- MOSI = GPIO11
- MISO = GPIO13
- Reset = shared board reset / EN
- Backlight = GPIO45

## UART

- RX = GPIO43
- TX = GPIO44
- 115200 baud, 8N1

## Touch

Touch is intentionally unused in this controller version.

## TFT_eSPI

Configure TFT_eSPI for an ILI9341 display using the ES3C28P LCD pins above before compiling controller_s3.ino.
