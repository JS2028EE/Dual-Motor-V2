# V2 power pinout plan

Updated: October 17, 2026. Status: planning draft; verify actual boards before wiring.

This document assigns power terminals and supply rails only. GPIO, analog inputs, UART, PWM, and sensor signals will be planned separately. The motor driver here is the **L293D** from the current parts list. If the actual part is L298N, its power pinout must be redone.

## New battery allocation — supersedes older source assignments below

October 17: select the 1,000 mAh rechargeable cell for the motor branch and the smaller cell (previously 500 mAh, label to confirm) for electronics. The motor branch still targets 6 V and electronics 5 V. **Portable charging/protection and converter load validation are pending.** Use [battery reallocation](../power/BATTERY_REALLOCATION.md) as the current decision. The three-cell motor-pack wiring and larger-electronics-cell assignment below are retained as earlier setup history, not instructions for the new motor cell. Do not bypass protection or connect both cells to one single-cell charger.

## 1. Earlier supply rails

| Rail / net | Voltage | Source | Destination |
|---|---|---|---|
| MOTOR_RAW | 4.5 V nominal | Three 1.5 V cells in series | SW-M input |
| MOTOR_SW | Variable around pack voltage | SW-M output | Motor boost IN+ |
| MOTOR_6V | Candidate regulated 6.0 V | Motor boost OUT+ | L293D VCC2, pin 8 |
| CELL+ / CELL- | One compatible Li-ion/LiPo cell, 3.7 V nominal, 4.2 V full | 1,000 mAh selected for modeling; 500 mAh alternative | Charger B+ / B- |
| BAT_PROTECTED+ | Battery-following voltage, not regulated 5 V | Charger OUT+ | SW-E input |
| ELEC_SW | Battery-following voltage | SW-E output | Electronics boost IN+ |
| ELEC_5V | Regulated 5.0 V | Electronics boost OUT+ | Verified ESP32 board 5 V input, L293D logic, compatible 5 V loads |
| ELEC_3V3 | Regulated 3.3 V, not 3.0 V | ESP32 development-board regulator | ESP32 module and verified low-current 3.3 V peripherals |
| SYSTEM_GND | 0 V reference / load return | Charger OUT- joined to motor-pack negative | Converters, ESP32, driver, and loads |
| CHARGE_5V | External regulated 5 V | USB charger | TP4056 USB input OR IN+ / IN- |

Separate positive supply rails are not joined together. Select one rechargeable electronics cell; do not combine the two batteries.

## 2. Three-cell motor pack: series connection

Connect cell 1 positive to cell 2 negative, and cell 2 positive to cell 3 negative. The two remaining ends are pack negative and pack positive:

- Pack negative = cell 1 negative -> SYSTEM_GND.
- Pack positive = cell 3 positive -> SW-M.
- Nominal total = 1.5 + 1.5 + 1.5 = 4.5 V.
- Series connection adds voltage; it does not triple mAh.
- Ordinary non-rechargeable cells are not connected to the TP4056 charger.

## 3. Motor branch terminal map

| From | To | Purpose |
|---|---|---|
| Motor pack positive | SW-M verified connected outer terminal | Raw motor power |
| SW-M verified common terminal | Motor boost IN+ | Disconnect converter input when off |
| Motor pack negative / SYSTEM_GND | Motor boost IN- | Battery return |
| Motor boost OUT+ | MOTOR_6V -> L293D pin 8, VCC2 | Candidate 6 V motor rail |
| Motor boost OUT- | SYSTEM_GND | Motor supply return |
| L293D pins 4, 5, 12, 13 | SYSTEM_GND | All driver ground/heat-sink pins |
| L293D pins 3 and 6 | Drive motor's two leads | Reversible motor output; neither motor lead is fixed ground |
| L293D pins 11 and 14 | Steering motor's two leads | Reversible motor output; neither motor lead is fixed ground |
| 100 uF electrolytic positive | MOTOR_6V near driver | Bulk supply filtering |
| 100 uF electrolytic negative stripe | SYSTEM_GND near driver | Capacitor return |

The two motors are powered through separate H-bridges, not connected in series with each other. The capacitor is across the rail in parallel, not in series with the motor.

6 V is the selected motor boost target and was user-reported adjusted during the direct motor test. Loaded output remains unmeasured. Set it with a multimeter before attaching the driver. Confirm the motors remain within their 3–6 V terminal rating at light and heavy load; do not treat a fixed driver-drop estimate as guaranteed.

## 4. Charger terminal map

The pictured six-pad TP4056-style board has battery and load terminals. Confirm the actual board includes its protection IC/MOSFETs.

| Terminal | Connect to | Meaning |
|---|---|---|
| B+ | Rechargeable cell positive only | Battery connection |
| B- | Rechargeable cell negative only | Battery-side return |
| OUT+ | SW-E verified connected outer terminal | Protected load positive |
| OUT- | SYSTEM_GND | Protected load return |
| IN+ | External regulated 5 V, if using wired input | Alternative to USB charging input |
| IN- | External charging-source return, if using wired input | Alternative to USB charging input |
| USB socket | Compatible 5 V USB source | Alternative charging input |

Use USB OR the wired IN input; do not connect two independent charging supplies at once.

**Important correction to the earlier generic common-ground diagram:** common load ground connects to **OUT-**, not directly to raw cell **B-**. Do not bridge B- to OUT-, ESP32 ground, or motor-pack negative externally: this can bypass the protected board's low-side cutoff. Keep B- on the battery side. Verify the exact board topology.

The charger remains connected to B+/B- while SW-E is off. Its output follows the battery voltage; it is not a regulated 5 V output. Charging current is set by the module's programming resistor and must be checked against the chosen cell's specified charge limit; do not assume a default 1 A is suitable for either battery.

For this version, turn both load switches OFF while charging. No simultaneous charge-and-run or extra USB/power return paths are designed here. The charger still draws a small standby current; the switches disconnect loads, not the battery from its charger.

## 5. Electronics branch terminal map

| From | To | Purpose |
|---|---|---|
| Charger OUT+ | SW-E terminal 1 | Protected battery power |
| SW-E verified common terminal | Electronics boost IN+ | Load switch before converter |
| Charger OUT- / SYSTEM_GND | Electronics boost IN- | Protected battery return |
| Electronics boost OUT+ | ELEC_5V distribution | Regulated 5.0 V |
| Electronics boost OUT- | SYSTEM_GND | Electronics return |
| ELEC_5V | ESP32 board's verified 5 V input | Through onboard 3.3 V regulator |
| ESP32 GND | SYSTEM_GND | Shared signal reference |
| ESP32 3V3 | ELEC_3V3 | Output for compatible low-current loads only |
| ELEC_5V | L293D pin 16, VCC1 | Driver logic power |

The ESP32-WROOM-32D module itself is a 3.3 V device. Board labels such as VIN or 5V must be checked against the exact development-board schematic. Never feed 5 V into its 3V3 pin or GPIOs.

Do not connect external 5 V and the ESP32 USB power together until that board's power isolation/backfeed behavior is verified. Disconnect external load power for USB programming during initial bring-up.

Motor current must return directly through motor wiring to motor-pack negative, not through ESP32 PCB ground traces.

## 6. Accessory power allocation

These branches are in parallel across the appropriate rail and SYSTEM_GND.

| Component | Proposed supply | Decision still needed |
|---|---|---|
| HC-SR04, if confirmed | ELEC_5V and SYSTEM_GND | Echo level protection belongs in the later signal plan |
| GPS breakout | ELEC_5V or ELEC_3V3, per actual breakout | Identify model and regulator before wiring |
| Two white headlights | ELEC_5V through separate LED resistors and a transistor switch | LED current, resistor values, transistor details |
| Buzzer | Appropriate verified rail through suitable driver | Identify active/passive type and rated voltage/current |
| L293D logic | ELEC_5V | Chip identity and power-state tests |
| Additional low-current 3.3 V sensors | ELEC_3V3 if regulator budget permits | Exact modules and current budget |

3V3 is an output of the board's regulator in this plan; sensors are not placed in series with the ESP32. All load currents must fit the regulator, battery, charger-protection, converter, switch, and wiring ratings.

## 7. Converter availability

This architecture requires **two independent boost stages**:
1. Protected rechargeable cell -> 5 V electronics.
2. Three-cell motor pack -> candidate 6 V motor rail.

Two independent boost converters are the accepted design. The user now reports electronics-branch assembly, working switch, and ESP32 LED power-up. Actual converter identities, connection inspection, loaded voltage/current, and thermal behavior remain unverified. A step-down-only converter cannot raise a 3.7 V cell to 5 V or a 4.5 V pack to 6 V. Do not connect the two batteries to a single converter input. Confirm two suitable boost modules or revise the supply design before assembly.

The pictured module resembles an MT3608 board; confirm the chip marking and actual output-current/thermal capability. Datasheet switch-current limits are not guaranteed module output-current ratings.

## 8. Switches and off state

SW-M and SW-E provide independent **SPST on/off functions in the positive supply paths**. A physical switch was reported to have three terminals, so it is likely SPDT: use its verified common and one outer terminal, with the other outer terminal disconnected and insulated. The center is often common but must be checked with power disconnected. This changes the hardware description, not the two-branch power architecture. Size each switch for its branch's DC current, including startup/inrush.

| Operating state | SW-M | SW-E | Notes |
|---|---|---|---|
| Stored / loads off | OFF | OFF | Both converters disconnected; charger remains on cell |
| Charging | OFF | OFF | External 5 V feeds charger; no operation while charging |
| Driving | ON | ON | Allowed only after power/current tests |
| One switch on | Mixed | Mixed | Not a validated operating mode; driver supply and signal backfeed must be checked |

Turning the ESP32 off does not open its GND connection. Motor-driver logic inputs and enables need defined inactive states; enable pull-downs and unpowered-interface behavior will be specified in the later signal plan. Do not rely solely on absence of software commands for shutdown.

TI specifies L293D VCC1 >= 4.5 V and VCC2 >= VCC1. A direct nominal 4.5 V motor rail with 5 V logic is outside these conditions. The candidate 6 V motor rail addresses this while both supplies are on; independent switching still needs validation.

## Bench-test deviation and unresolved power behavior

During the reported test, USB powered the ESP32 board rather than the final electronics battery/boost branch. Its 5 V rail fed L293D pin 16 plus fixed HIGH control jumpers at pins 1 and 2. The motor ran with its motor switch off and sped up when it was turned on. This is an observed limitation, not intended power switching.

The source path remains unconfirmed. Remove fixed 5 V control jumpers before any ESP32 GPIO connection. Pin 16 remains a 5 V logic supply; pin 1 will become software-controlled enable, with a defined default-disabled state. GPIO control does not prove physical motor-power isolation. See [test log](../testing/TEST_LOG.md).

The charger was also reported very hot; the proposed B+/OUT+ bridge explanation is not a confirmed diagnosis. Charging current, polarity, protection topology, and thermal behavior need verification.

## 9. Capacitors and remaining checks

- 100 uF / 50 V electrolytic: selected as a candidate bulk capacitor across MOTOR_6V and SYSTEM_GND.
- 10 uF / 50 V electrolytic: available; additional placement depends on converter requirements.
- User-reported 22 pF and 104 pF ceramics: not selected for supply bypass or motor suppression.
- Obtain/confirm appropriate local ceramic bypass capacitors; TI recommends at least 0.1 uF at each L293D supply.
- Do not put a polarized electrolytic across a reversing motor.
- Verify exact ESP32 header and driver package, charger protection/charge current, converter count, loaded rail voltages, motor startup current, and component heating before final wiring.

## References

- [TI L293x datasheet](https://www.ti.com/lit/ds/symlink/l293.pdf): power pin numbers, supply ordering, current/thermal limits, and decoupling.
- [TP4056 manufacturer datasheet, distributor-hosted](https://www.sunrom.com/download/TP4056.pdf): one-cell charging and charge-current programming.
- [Protected TP4056 module vendor manual](https://shillehtek.com/blogs/shillehtek-product-manuals/tp4056-1a-lipo-battery-charging-board-type-c-with-current-protection-manual): B/OUT terminal roles and load-sharing limitations; verify actual module.
- [Espressif module datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32d_esp32-wroom-32u_datasheet_en.pdf): 3.3 V module requirements; development-board power input remains board-specific.

## Electronics power-up update — October 17

The user reported correcting an overlapping battery/wire connection, particularly underneath, and then reported a working switch and ESP32 power LED. The precise fault was not identified. This does not change the agreed wiring or establish numerical rail/current values. See [test 6](../testing/TEST_LOG.md#test-6--electronics-power-branch-assembled-october-17) and [MATLAB/Octave model](../../simulations/electronics_power_chain.m).
