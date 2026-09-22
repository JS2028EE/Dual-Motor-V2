# Dual Motor V2

Controller-first redesign of the dual-motor robotic car.

## Phase 1 — Controller

- ESP32-C3 SuperMini
- ESP32-S3 ES3C28P 2.8-inch 240x320 display
- Two analog joysticks with push switches
- Dedicated Wi-Fi link between C3 and S3
- TCP controller protocol on port 4210
- Landscape cyberpunk UI
- Touch intentionally unused

The S3 creates the controller's private Wi-Fi access point. The C3 joins that network and exchanges controller data with the S3 over TCP. A separate router or Internet connection is not required.

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
| GPIO20 | TX — legacy UART, not used by Wi-Fi firmware |
| GPIO21 | RX — legacy UART, not used by Wi-Fi firmware |
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
- UART0 RX: GPIO43 — legacy controller UART, not used by Wi-Fi firmware
- UART0 TX: GPIO44 — legacy controller UART, not used by Wi-Fi firmware
- Expansion: GPIO2, GPIO3, GPIO14, GPIO21

## Wi-Fi controller link

| Setting | Value |
|---|---|
| Network owner | ESP32-S3 |
| Mode | S3 Access Point / C3 Station |
| SSID | DUAL-MOTOR-V2 |
| Password | DMV2-CTRL |
| S3 IP | 192.168.4.1 |
| Transport | TCP |
| Port | 4210 |
| Expected controller | 1 C3 |

The S3 starts the network automatically. The C3 automatically reconnects if the Wi-Fi or TCP connection drops.

## UI

Landscape orientation. Dark neon cyberpunk engineering-console aesthetic. Current UI displays navigation input, driving input, Wi-Fi controller status, vehicle connection status, and the latest controller event.

## Future vehicle

- Normal ESP32
- GY-GPS6MV2
- L293D
- 2 x 3–6V DC motors
- 3 x 1.5V batteries in series = 4.5V motor supply
- 3.7V Li-ion electronics supply
