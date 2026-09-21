# Dual Motor V2

Controller-first redesign of the dual-motor robotic car.

## Phase 1 — Controller only

- ESP32-C3 SuperMini
- ESP32-S3 ES3C28P 2.8-inch 240x320 display
- Two analog joysticks with push switches
- UART between C3 and S3
- Landscape cyberpunk UI
- Touch intentionally unused

Vehicle ESP32, GY-GPS6MV2, L293D, motors, and vehicle power are deferred until the controller is validated.

## Controller roles

**Navigation joystick:** X/Y menu navigation; press = Select; hold = Back.

**Driving joystick:** X/Y driving input; press = Brake; hold = Emergency Stop.

Forward is defined as movement toward the joystick module's pin header. Axis inversion/swapping will be calibrated in firmware.

## C3 reference pinout

| Pin | Assignment |
|---|---|
| GPIO0 | A0 |
| GPIO1 | A1 |
| GPIO2 | A2 |
| GPIO3 | A3 |
| GPIO4 | A4 / SCK |
| GPIO5 | MISO / A5 |
| GPIO6 | MOSI |
| GPIO7 | SS |
| GPIO8 | SDA / built-in LED |
| GPIO9 | SCL |
| GPIO10 | GPIO10 |
| GPIO20 | TX |
| GPIO21 | RX |
| 3V3 | 3.3V |
| GND | Ground |
| 5V | 5V |

## S3 ES3C28P reference

ILI9341V, 2.8-inch IPS TFT, 240x320, 4-line SPI.

- LCD CS: GPIO10
- LCD D/C: GPIO46
- LCD SCK: GPIO12
- LCD MOSI: GPIO11
- LCD MISO: GPIO13
- LCD backlight: GPIO45
- Touch SDA: GPIO16
- Touch SCL: GPIO15
- Touch reset: GPIO18
- Touch interrupt: GPIO17
- RGB LED: GPIO42
- Battery ADC: GPIO9
- UART0 RX: GPIO43
- UART0 TX: GPIO44
- Expansion: GPIO2, GPIO3, GPIO14, GPIO21

## UART

C3 GPIO20 TX -> S3 GPIO43 RX

C3 GPIO21 RX <- S3 GPIO44 TX

GND -> GND

UART is 3.3V logic. The board's 5V power input is separate from UART signaling.

## UI

Landscape orientation. Dark neon cyberpunk engineering-console aesthetic. Planned areas include Home, Connection, Vehicle Status, Location, Manual, Assistance, Automation, Beep, Lights, Diagnostics, and Settings.

## Future vehicle

- Normal ESP32
- GY-GPS6MV2
- L293D
- 2 x 3–6V DC motors
- 3 x 1.5V batteries in series = 4.5V motor supply
- 3.7V Li-ion electronics supply
