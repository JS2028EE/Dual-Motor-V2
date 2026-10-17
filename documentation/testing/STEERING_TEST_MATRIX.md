# Steering test matrix — capacitor, pulse duration, and source

October 17, 2026. Proposed tests; no outcomes reported yet. Current default test firmware is now 200 ms. The earlier hardware result used 150 ms without an electrolytic capacitor: continuous noise stopped, but steering remained weak.

## Mechanical setup update before Test 2

October 17, 2026. Jay reports replacing the steering gears with lighter gears; the mechanism still turns. This is a user-performed hardware modification, not a firmware change. Gear tooth counts, gear ratio, material, mass, geometry, friction, torque effect, and exact timing relative to Test 1 were not provided. The statement about whether these gears make steering harder was incomplete; no torque improvement or reduction is claimed.

Record this modification as part of the installation/setup history before interpreting Test 2. Lower gear mass alone does not establish improved steering torque. Do not assume the reduction ratio stayed the same.

Keep this gear setup fixed for the battery baseline and bench-source comparison. If gears changed since the battery trial, repeat the battery baseline with the replacement gears and the same 150 ms pulse/capacitor/load before comparing it with the source-only bench test. Otherwise differences could arise from mechanics as well as source. Test 2 remains pending; no new movement result, firmware change, or supply setting is recorded by this note.

## Hypothesis and timing choice

Jay proposes that 150 ms gives insufficient steering travel, and that supply capability may also contribute to weak steering. Use 200 ms first: 33% more powered time than 150 ms. 230 ms would be 53% more, not double. A kinematic estimate at unchanged movement speed gives the same ratios for travel, only while moving freely. Unknown friction, gearing, motor speed and end-stop timing prevent predicting an adequate duration.

A longer pulse does not directly increase voltage, PWM duty, or instantaneous torque. If steering is already stalled, it increases stall time. The combined change is a candidate solution, not an established result.

## Capacitor connection for every test

With all sources OFF/disconnected, connect the reported 1,000 uF / 50 V electrolytic in parallel with the MOTOR SUPPLY, near the L293D:
- Capacitor positive -> L293D pin 8 / motor +6 V rail.
- Capacitor negative (stripe marking minus) -> common ground rail, shared with all L293D GND pins 4, 5, 12 and 13.
- On battery tests, those are also motor boost OUT+ and OUT-, respectively.
- On bench tests, those are bench positive and negative, respectively.

Check the part's actual value and polarity. Keep leads short, avoid shorts, and leave the existing motor ceramics. Do not connect a polarized electrolytic across reversing motor outputs (pins 11/14 or 3/6): their polarity reverses. Pin 16 remains a separate 5 V logic supply.

The bulk capacitor may help short rail transients; it does not repair binding or provide sustained motor power. TI recommends local bypass capacitors at both supply pins; motor ceramics do not replace those supply bypass parts. Source: https://www.ti.com/lit/ds/symlink/l293.pdf .

## Baseline plus three requested tests

| Run | Source at motor rail | Steering powered time | Capacitor |
|---|---|---|---|
| B: new baseline | Existing three-cell pack + motor boost set to 6 V | 150 ms | Installed |
| 1: more time only | Same battery and boost | 200 ms | Installed |
| 2: source comparison only | Bench supply set to 6 V directly to motor rail | 150 ms | Installed |
| 3: combined | Same bench supply setup | 200 ms | Installed |

Baseline B is needed because adding a capacitor is itself a change from the prior trial. Compare B with 1 to test pulse duration, B with 2 to test source, and 2 with 3 to test timing under the bench source. Do not label source effects isolated if the supply is current limiting or its current limit differs between bench comparisons. Battery and source ability remain unmeasured.

Current sketch: [DualMotorV2_BLE_Pulse.ino](../../firmware/testing/DualMotorV2_BLE_Pulse/DualMotorV2_BLE_Pulse.ino). Change only STEER_PULSE_MS between 150 and 200 and upload for each corresponding run. Keep steering duty 128/255 (~50%), PWM 1 kHz, speed setting, and all other code constant. If 200 ms is insufficient while the linkage is visibly moving freely, a later 230 ms trial can be considered separately; do not jump to it automatically.

## Bench source wiring and limits

Output OFF before changing connections. Disconnect BOTH motor boost output leads from the motor rail; do not parallel the boost/battery feed with the bench supply. Bench positive -> pin 8 motor rail; bench negative (not chassis/earth terminal) -> common circuit ground. Keep pin 16 at its separate 5 V logic supply and keep ESP32 power unchanged. Do not apply 6 V to an ESP32 GPIO or 3V3 pin.

Set 6.0 V and initially a 0.30 A current limit for a steering-only check. Observe CV/CC and actual voltage. If CC is active, the source is limiting current and may cause weakness; record that instead of inferring the motor is bad. For brief further checks, the source limit may be increased in small steps up to 0.60 A total while observing the mechanism and stopping if it binds, stops moving, or heats rapidly. Do not treat 0.60 A as a target to force into the motor or a guarantee of safe operation.

The L293D's 600 mA output rating is per channel, subject to thermal limits; package losses and PWM peak winding current are not established by a supply display's total average reading. Do not increase the current limit further to overcome an unresolved stall. Begin with steering alone, wheels lifted; then repeat with the same surface/front load for each run. Finish with driving plus steering only after steering-alone behavior is understood. If combined operation stays in CC, the test is inconclusive as a comparison against a stiff 6 V source.

A charged capacitor can source short pulses even when a bench supply is limited; output current limiting is not instantaneous winding-current protection. Discharge through a suitable resistor before handling or rewiring; do not short the capacitor leads.

## Record for every run

Record pulse setting, capacitor value, source setting, CV/CC, displayed loaded voltage/current if available, initial steering position, surface/load, achieved steering travel, noise duration, driving slowdown, and whether wheels retain position after cutoff. Use the same initial steering position for comparisons. Same-side commands are ignored until opposite or stop; send stop before a repeated same-side trial.

Stop rather than holding at an end stop. No multimeter current measurement is required for this test sequence. Earlier 12–21 mA readings obtained across source terminals in current mode are invalid load-current data and are excluded. Do not repeat those across-terminal current connections.

## Status

Only firmware timing and documentation have changed. No capacitor, supply setting, mechanics, or actual vehicle behavior was changed remotely. Host tests passed for 200 ms cutoff and preserved drive/stop behavior; Arduino-core compilation and 200 ms hardware results remain pending.


## Test 1 result update

The user reports steering works with wheels lifted and driving off, but barely moves with driving on even while lifted. Ground contact further limits steering, and driving briefly slows during commands. Timing alone has not met the loaded-steering requirement. See [Test 11](TEST_LOG.md#test-11--test-1-duration-trial-unloaded-steering-improves-loaded-steering-remains-weak) for the separated conditions, hypotheses and proposed fixes. Capacitor installation and exact flashed duration were not reconfirmed. Source-only Test 2 must use 150 ms; the current default is 200 ms and corresponds to Test 3 when used with the bench source.


## Test 2 report and Test 3 start

User reports improved driving with the 6 V bench source at a 0.60 A setting, while 150 ms steering remains insufficient. The reported 0.55 A (550 mA) figure has not been classified as a live reading versus limit setting. Current default firmware is restored to 200 ms for Test 3; retain the same source, gears, surface/load, initial angle and capacitor setup. Test 3 results are pending. See [Test 13](TEST_LOG.md#test-13--bench-source-improvement-reported-start-combined-200-ms-trial). Earlier default-timing instructions above describe earlier revisions.
