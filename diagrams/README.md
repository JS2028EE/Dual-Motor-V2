# V2 block diagrams

Current planning revision: October 16, 2026. These are architecture diagrams, not a pin-level wiring schematic.

## Signals and control

```mermaid
flowchart TD
    Client["Phone or computer"] <-->|Wi-Fi or Bluetooth: selection pending| ESP["ESP32-WROOM-32D"]
    Ultra["Front ultrasonic sensor"] -->|"Distance; protect Echo input if 5 V"| ESP
    GPS["GPS module"] -->|UART location data| ESP
    Feedback["Steering feedback: to be identified"] -.-> ESP
    ESP -->|"Four direction signals and two PWM enables"| Driver["L293D"]
    Driver --> Drive["Drive motor: forward and reverse"]
    Driver --> Steer["Steering motor: left and right"]
    ESP --> LightSwitch["Headlight transistor switch"]
    LightSwitch --> Lights["Two white LEDs; one resistor each"]
    ESP --> BuzzerDriver["Suitable buzzer driver"]
    BuzzerDriver --> Buzzer["Buzzer"]
```

Feedback is conditional. A two-wire steering motor does not report steering angle. Inspect centering springs, end stops, and any position sensor before choosing a control strategy.

## Power

```mermaid
flowchart TD
    Cells["Three 1.5 V cells: 4.5 V nominal"] --> SwitchM["Motor power switch"]
    SwitchM --> Boost["Boost converter: candidate 6 V output"]
    Boost --> MotorRail["Motor rail: measure under load"]
    MotorRail --> L293["L293D motor supply"]
    MotorRail --> Cap["100 uF electrolytic across supply and ground"]
    L293 --> Motors["Drive and steering motors"]

    Charge["Compatible charger and protection: selection pending"] -.-> Battery["Separate rechargeable electronics battery"]
    Battery --> SwitchE["Electronics power switch"]
    SwitchE --> Reg["Regulated 5 V supply: verify board compatibility"]
    Reg --> ESP["ESP32 development-board 5 V input"]
    Reg --> Logic["L293D logic supply"]
    Reg --> Loads["Compatible sensor and accessory rails"]

    Ground["Common ground; positive rails remain separate"] --- Cells
    Ground --- Battery
    Ground --- L293
    Ground --- ESP
    Ground --- Loads
```

The candidate boost path replaces the unresolved direct 4.5 V motor path; it is not yet a validated supply. GPS and other module rails must match their exact breakout requirements. The ESP32 module itself uses 3.3 V; the 5 V input shown is for a suitable development board regulator.

The charger is a future design item. A boost or buck converter is not a battery charger. Do not charge ordinary non-rechargeable cells. Charger load sharing and operation while charging remain unresolved.
