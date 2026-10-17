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

Detailed terminal assignments: [power pinout plan](../documentation/pin-configuration/POWER_PINOUT.md).

```mermaid
flowchart TD
    USB["External USB 5 V"] --> Charger["TP4056-style protected charging module"]
    Cell["One Li-ion cell: battery leads to B+ and B-"] --- Charger
    Charger -->|"OUT+"| SWE["SW-E: SPST electronics switch"]
    SWE --> BoostE["Separate electronics boost: 5.0 V"]
    BoostE --> Rail5["5 V rail"]
    Rail5 --> ESP["ESP32 board verified 5 V input"]
    ESP --> Rail3["3.3 V rail: compatible low-current loads"]
    Rail5 --> Logic["L293D pin 16: logic supply"]
    Rail5 --> Loads["Compatible 5 V loads"]

    AA["Three 1.5 V cells in series: 4.5 V nominal"] --> SWM["SW-M: SPST motor switch"]
    SWM --> BoostM["Motor boost: candidate 6.0 V"]
    BoostM --> VM["L293D pin 8: motor supply"]
    BoostM --> Cap["100 uF across motor rail and ground"]
    VM --> Motors["Two motor bridges: drive and steering"]

    GND["SYSTEM_GND: charger OUT-, not B-"] --- AA
    GND --- ESP
    GND --- VM
    GND --- BoostE
    GND --- BoostM
```

Ground lines indicate shared load returns: AA negative, both converter returns, ESP32 GND, L293D ground pins, and accessory grounds join SYSTEM_GND. Motor return wiring goes directly to the motor battery negative rather than through the ESP32 board. Raw rechargeable-cell B- stays only on the battery side of the protection circuit.

Two boost stages are required; only one module has been shown. A buck-only converter cannot supply either voltage increase. Positive rails remain separate. GPS rail and exact ESP32 development-board input must be confirmed.

Both SPST switches are OFF for storage/load shutdown and charging. The charger stays connected to the cell. No charge-and-run load-sharing circuit is included. The ordinary three-cell motor pack is not connected to the charger.

GPIOs and analog/digital signal wiring will be assigned separately. This power architecture is a draft, pending actual module, current, charge-rate, and mixed-power-state checks.
