# ESP32 vehicle plan

Updated October 17, 2026. This plan supersedes the handheld-controller-first scope.

## Power terminal plan

See the [power pinout plan](../pin-configuration/POWER_PINOUT.md) for TP4056 B/OUT/IN terminals, two switched positive branches (likely SPDT switches used for SPST on/off), separate boost stages, supply rails, and protected common ground. Use charger OUT- as load ground; do not externally bridge B- to OUT-. The six motor-test GPIO assignments and BLE commands are recorded in the [testing guide](../../firmware/testing/README.md); sensor/accessory signal assignments remain deferred. Two boost stages are required by the proposed rails; module identities and complete integration remain unverified.

See [current decisions](../DECISIONS.md) and [bench test log](../testing/TEST_LOG.md). No C3 or S3 controller is part of the active build. Bench USB power is temporary; it does not change the battery/charger/two-boost architecture.

## Goal and control

Use one ESP32-WROOM-32D development board, one drive motor, and one steering motor. A phone or computer will provide manual commands through Wi-Fi or Bluetooth. The app, protocol, command format, and timeout are still to be selected.

Automatic mode will initially use a front ultrasonic sensor to stop before obstacles. Avoidance behavior requires steering tests and stopping-distance measurements. GPS is for outdoor location/telemetry, not precise obstacle detection.

Planned stop behavior: disable drive on startup, manual stop, stale manual-control commands, or invalid/stale obstacle readings in automatic mode. A lost manual link should stop the car before any deliberate switch to another mode. Hardware power-off must remain accessible. These behaviors are requirements, not implemented firmware.

## Parts and decisions

| Item | Current selection | Status |
|---|---|---|
| MCU | ESP32-WROOM-32D development board | Selected; exact header layout pending |
| Motor driver | L293D | Retained; current and heat checks pending |
| Drive | Existing 3–6 V brushed DC motor | RPM, gearing, torque, and current unknown |
| Steering | Existing steering motor/mechanism | Centering and feedback unknown |
| Motor pack | Three 1.5 V cells in series, 4.5 V nominal | Cell type, capacity, and load sag pending |
| Electronics battery | Separate 1,000 mAh cell selected for modeling; 500 mAh alternative | 3.65 V user report; confirm chemistry, charge/discharge ratings |
| Step-up | Two independent boosts: motor 6 V and electronics 5 V; pictured type resembles MT3608 | Motor direct test reported; identities and full load capability unconfirmed |
| Step-down | Available converter | Exact model and need pending |
| Ultrasonic | Working front sensor; HC-SR04 candidate | Confirm model; protect 5 V Echo if applicable |
| GPS | Existing GPS breakout | Exact model, rail voltage, and current pending |
| Lights | Two white LEDs | One resistor per LED; shared transistor control |
| Buzzer | Existing buzzer | Active/passive type, voltage, current pending |
| Bulk capacitor | 100 uF, 50 V electrolytic | Candidate near driver motor supply |
| Extra capacitor | 10 uF, 50 V electrolytic | Available; placement to be decided |
| Ceramics | User reports 22 pF and 104 pF | Record as reported; not selected for motor suppression |
| Charging and switches | TP4056-style protected charger and two on/off functions; three-terminal switch likely SPDT | Verify common/outer contacts, charge current, protection, heating, and ratings |
| Magnetic sensor | Available | Deferred until a purpose is chosen |

Battery capacities are mAh, not current ratings. Do not combine the 500 mAh and 1,000 mAh cells in this plan; they are alternative electronics-battery candidates.

The reported ceramic values are retained as inventory. If a capacitor is marked only "104", the standard three-digit code normally denotes 100 nF; do not silently substitute that value for the user's reported 104 pF. Confirm by measurement or reliable component documentation before selecting noise filtering. A verified 100 nF ceramic across each brushed motor is a common suppression choice.

## Power calculations and limits

Three series cells give 4.5 V nominal; their mAh capacity does not triple. Actual voltage depends on cell chemistry, condition, and current.

The L293D has separate logic and motor supplies. TI specifies logic VCC1 at 4.5–7 V and motor VCC2 at or above VCC1. A 5 V logic rail with a direct 4.5 V motor rail is outside recommended conditions. Nominal 4.5 V logic also leaves no discharge/sag margin.

Evaluate 4.5 V pack -> boost set to 6 V -> L293D motor supply, with a separate regulated 5 V logic rail. Adjust and measure boost output before attaching the driver. Verify output voltage and converter temperature under actual motor starting and running loads.

Example only:
- Assumed total running motor current: 250 mA drive + 100 mA steering = 350 mA while both are active.
- Assumed L293D path drop: 2 V, giving 2.5 V from a direct 4.5 V supply or 4 V from a boosted 6 V supply.
- Assumed boost efficiency: 85%; 6 V at 350 mA requires about 549 mA from a 4.5 V pack.
- Assumed motor-path driver loss: 2 V x 0.35 A = 0.70 W. This excludes logic-supply losses.

Driver drop is load dependent. TI's typical combined drop at 600 mA is about 2.6 V; the 2 V model is an illustrative input, not a guaranteed specification. The L293D is rated up to 600 mA continuous per channel, subject to thermal limits. Check starting/stall currents and simultaneous motor heating; the boost converter does not increase this limit.

For the capacitor, t = C * deltaV / I. An ideal 100 uF capacitor supplying an extra 0.3 A with a 0.5 V dip supports it for only about 0.17 ms. It helps brief supply disturbances, not sustained motor demand. Put polarized electrolytics across supply and ground with correct polarity, not across a reversing motor.

## Electronics estimates

These are provisional average currents at the 5 V rail, not measured component specifications.

| Load | Assumed current |
|---|---:|
| ESP32 development board | 180 mA |
| GPS breakout | 30 mA |
| Ultrasonic sensor | 15 mA |
| Two headlights combined | 10 mA |
| Buzzer average | 5 mA |
| L293D logic | 35 mA |
| Total | 275 mA |

At 5 V this is 1.375 W. With an assumed 3.7 V battery, 85% boost efficiency, and 80% usable capacity, estimated input is 437 mA: approximately 1.83 h on 1,000 mAh or 0.91 h on 500 mAh. These estimates exclude motors and do not size the supply for radio or buzzer peaks. If module rails differ, recompute power for each rail.

## GPIO budget

| Function | GPIO count |
|---|---:|
| Four motor direction inputs and two PWM enables | 6 |
| Ultrasonic Trigger and Echo | 2 |
| GPS receive only / bidirectional UART | 1–2 |
| Shared headlight switch | 1 |
| Buzzer control | 1 |
| Optional mode button | 1 |
| Total before steering feedback | 12–13 |

Board pins include power/ground and reserved functions. GPIO34–39 are input-only; avoid flash pins and check boot-strapping constraints. UART and I2C can use suitable remapped GPIOs. Final assignments require the exact board pinout. Motors and accessories needing more current use suitable drivers rather than GPIO power.

## Physical inspection and measurements

Record:
1. Tire outside diameter and driven-wheel RPM.
2. Complete vehicle mass, including batteries.
3. Wheelbase, track width, steering range, and available mounting space.
4. Motor labels, gearing, running current, and starting/stall-current data obtained with a suitably current-limited method.
5. Steering centering spring, end stops, limit switches, and position feedback.
6. Battery chemistry/capacity and open-circuit versus loaded voltage.
7. Converter input/output voltage and current under load.
8. GPS placement, front ultrasonic clearance, wire routing, and switch access.

A two-wire DC steering motor cannot command an absolute angle without a reliable reference/feedback strategy. Do not keep it powered against an end stop.

## Next physics models

- Wheel speed: v = pi * D * wheel_RPM / 60.
- Required force: F = m*a + rolling resistance + m*g*sin(slope).
- Required driven-wheel torque: torque = F * D/2.
- Stopping distance: d approximately v*response_delay + v^2/(2*braking_deceleration).

Motor torque/gearing, friction, and measured acceleration are needed to connect weight and voltage to movement. The first Octave script is a hypothetical speed curve, not that full model. The newer power/control example adds an assumed linear voltage-to-RPM relation, but still has no torque/mass model. See [both scripts](../../simulations/README.md).

## Sources

- [TI L293x datasheet](https://www.ti.com/lit/ds/symlink/l293.pdf): supply conditions, logic levels, output drop, and current/thermal limits.
- [Espressif ESP32-WROOM-32D/32U datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32d_esp32-wroom-32u_datasheet_en.pdf): module pins and electrical requirements.
- [TI Working with Boost Converters](https://www.ti.com/lit/an/snva731/snva731.pdf): input/output power balance.
- [Pololu motor noise guidance](https://www.pololu.com/docs/0J15/9): ceramic and supply filtering.
- [Aerosemi MT3608 datasheet, distributor-hosted](https://www.mouser.com/datasheet/2/306/MT3608-3223743.pdf): candidate converter IC only; does not identify or rate the pictured assembled board.

## Motor-test update — October 17

The active motor-only bench test uses BLE on the original ESP32, the agreed L293D pin map, drive speed_N integer duty0–100 (initial 100), latched forward/back/left/right, and lowercase stop for both motors. Steering remains fixed near 50% PWM; both channels are 1 kHz. This is test code, not a complete/validated car controller.

The latest reported 1,000 uF/50 V capacitor experiment did not remove audible noise. Existing ceramics were reported on both motors. Drive noise around speed 30 and drive slowdown during steering are recorded in the [test log](../testing/TEST_LOG.md). Full current, voltage, torque, temperature, and steering-limit validation remains pending. No driver/rail change has been adopted.
