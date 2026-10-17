# V2 block diagrams

Current planning revision: October 17, 2026. This is the agreed active architecture: no C3 or S3 handheld controller. These are architecture diagrams, not a pin-level wiring schematic.

## Signals and control

```mermaid
flowchart TD
    Client["Phone or computer"] <-->|BLE motor test; full interface pending| ESP["ESP32-WROOM-32D"]
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
    Charger -->|"OUT+"| SWE["SW-E: electronics on/off"]
    SWE --> BoostE["Separate electronics boost: 5.0 V"]
    BoostE --> Rail5["5 V rail"]
    Rail5 --> ESP["ESP32 board verified 5 V input"]
    ESP --> Rail3["3.3 V rail: compatible low-current loads"]
    Rail5 -->|"5 V to pin 16"| DriverPower
    Rail5 --> Loads["Compatible 5 V loads"]

    AA["Three 1.5 V cells in series: 4.5 V nominal"] --> SWM["SW-M: motor on/off"]
    SWM --> BoostM["Motor boost: candidate 6.0 V"]
    BoostM -->|"6 V to pin 8"| DriverPower["L293D"]
    BoostM --> Cap["Bulk capacitor across motor rail and ground"]
    DriverPower --> Motors["Outputs 3/6 drive; 11/14 steering"]

    GND["SYSTEM_GND: charger OUT-, not B-"] --- AA
    GND --- ESP
    GND ---|"Pins 4, 5, 12, 13"| DriverPower
    GND --- BoostE
    GND --- BoostM
```

Ground lines indicate shared load returns: AA negative, both converter returns, ESP32 GND, L293D ground pins, and accessory grounds join SYSTEM_GND. Motor return wiring goes directly to the motor battery negative rather than through the ESP32 board. Raw rechargeable-cell B- stays only on the battery side of the protection circuit.

Two boost stages are selected; physical identities and complete battery-powered integration remain unverified. A buck-only converter cannot supply either voltage increase. Positive rails remain separate. GPS rail and exact ESP32 development-board input must be confirmed.

Both on/off switches are OFF for storage/load shutdown and charging. The charger stays connected to the cell. No charge-and-run load-sharing circuit is included. The ordinary three-cell motor pack is not connected to the charger.

The motor-only test GPIOs and commands are recorded in the [BLE testing guide](../firmware/testing/README.md); sensor/accessory signal wiring remains pending. This power architecture is a draft, pending actual module, current, charge-rate, and mixed-power-state checks.

## How to read this plan

The switches perform an SPST on/off function. The user reported a three-terminal switch, likely SPDT: verify its common and one connected outer terminal, leaving the other outer terminal unused. This is the same two-branch architecture.

This diagram shows intended final wiring. USB-fed ESP32 5 V and fixed HIGH driver-input jumpers were temporary bench-test arrangements. They are not an extra power route in the final plan. Remove fixed 5 V control jumpers before GPIO wiring; the final L293D logic supply stays at pin 16.

The motor ran with its supply switch off during the reported bench test. That is an unresolved observation, not intended behavior depicted by this diagram. See [test log](../documentation/testing/TEST_LOG.md) and [limitations](../limitations/LIMITATIONS.md). GPIO test code now exists, but power isolation and startup/disconnect/stop performance are not electrically validated.

Latest capacitor observation: the user reports1,000 uF/50 V and pre-existing motor ceramics; audible noise persisted. Earlier supply planning named 100 uF. The diagram shows recommended bulk-filter placement, not proof of the actual installation. See [noise/load investigation](../documentation/testing/MOTOR_NOISE_AND_LOAD.md). Architecture remains unchanged.
