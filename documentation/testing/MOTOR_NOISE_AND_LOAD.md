# Motor noise and drive slowdown during steering

October 17, 2026. Status: investigation; no validated noise/power fix.

## Observations

- Steering motor produces audible noise.
- Drive motor makes noise around `speed_30`; user reports operation above that setting appears fine.
- Activating left/right while driving makes the drive motor slow down.
- User reports both motors already have ceramic capacitors.
- An electrolytic supply-capacitor experiment did not eliminate the noise. Earlier conversation described 100 uF; the latest report specifies **1,000 uF, 50 V**. Record this as a later reported test/value clarification, not proof both values were separately installed.
- Actual capacitor placement and polarity were not verified. The instructed location is positive to L293D pin 8 / motor boost OUT+, striped negative to common GND. Never put a polarized electrolytic across reversing motor outputs.

## What the test code explains

Both enable channels use 1,000 Hz PWM. The drive slider selects 0–100% duty; the steering uses fixed 128/255 (about50%) duty. Partial-duty motor drive can produce audible vibration. Low-duty buzzing may also accompany insufficient startup torque or a mechanically stalled motor.

No microphone, scope, RPM, or current data confirms a single cause. Existing ceramic capacitors suppress electrical brush noise; they do not guarantee acoustic silence. Bulk electrolytics can buffer brief supply disturbances but cannot provide continuous extra power or remove all PWM/mechanical sound.

## Why steering can slow drive

The two bridges share the motor battery, boost converter, supply wires, and L293D package. When steering activates, total demanded current increases. If it reaches an end stop, stall current can be much higher than freely moving current.

The motors are not in series and do not automatically split the 6 V rail in half. In a sufficiently capable supply, both bridges can access the same rail voltage. Extra current can instead cause:
- Battery voltage sag.
- Boost current limiting or output droop.
- More voltage drop across wires, breadboard contacts, or the driver.
- Increased driver heating and possible thermal limitation.

These are hypotheses. No reliable loaded voltage/current measurement identifies which is dominant. `speed_30` is not an RPM measurement.

## Proposed changes, ordered for diagnosis

1. Send `stop` and establish whether noise ceases. Do not keep steering energized against an end stop.
2. Check steering motion/centering/limits. Plan short pulses or actual limit/position feedback so power is removed when motion is complete. No feedback sensor is confirmed.
3. At the SAME drive-duty setting, compare drive-only with drive-plus-steering. Record motor-pack voltage, motor boost OUT voltage, voltage across the drive motor leads, driver temperature behavior, and current when a reliable meter/current-limited setup is available. Measure voltage in parallel; never use a current-range meter directly across a source.
4. If pack voltage sags, address battery/current delivery. If the pack holds but boost output sags, assess converter capacity. If boost holds but drive-motor voltage falls, assess driver/connection losses and heat.
5. Once current delivery and steering stall are understood, evaluate PWM frequency changes within the actual driver's switching/thermal capability. A higher frequency may reduce audible PWM tone but is not automatically suitable for L293D and does not remove a mechanical stall or lack of available torque.
6. If the motor starts reliably only above a certain duty, characterize its minimum usable drive setting; a brief startup boost or duty mapping is a possible later feature. It must be tested under load and is not implemented here.
7. A lower-loss driver or improved supply is a possible later upgrade after motor startup/stall current is known. The current agreed architecture still uses L293D; no part substitution or rail-voltage increase has been adopted.

Do not raise the motor boost above the chosen 6 V target as a substitute for these checks.

## Capacitor limits

A correctly connected 1,000 uF capacitor supplying an extra 0.3 A with an allowed 0.5 V drop provides ideally t=C*deltaV/I = about 1.67 ms. Actual behavior also depends on ESR, wiring, and converter dynamics. It cannot sustain a continuously stalled steering motor. The 50 V marking is a voltage rating, not generated voltage.

## Sources

- [TI L293x datasheet](https://www.ti.com/lit/ds/symlink/l293.pdf): output drop, current and heat limits.
- [Pololu motor noise guidance](https://www.pololu.com/docs/0J15/9): motor ceramics, bulk supply filtering, and wiring.
- [ESP32 LEDC documentation](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/ledc.html): PWM configuration.
- [Recorded BLE test source](../../firmware/testing/DualMotorV2_BLE_Latched/DualMotorV2_BLE_Latched.ino): actual selected frequency and duty behavior.

These sources support the circuit principles; actual causes remain unmeasured.
