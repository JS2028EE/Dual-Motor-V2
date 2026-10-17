# Test notes and observations

Recorded October 16–17, 2026, America/New_York. These records distinguish user-reported observations, assumed simulation inputs, and unverified explanations. No instrument readings are inferred from visual speed changes.

## Test 1 — Direct drive motor, October 16

**Purpose:** Compare the boosted motor supply with the earlier single-cell setup.

**Setup:** Three 1.5 V cells in series -> boost adjusted to 6.0 V -> drive motor directly across OUT+ and OUT-. No L293D or ESP32 control was included. The pack was reported at 4.68 V before testing.

**Observation:** The drive motor spun noticeably faster than with the earlier 3.7 V, 500 mAh battery.

**Conclusion:** The boosted supply ran that motor under the tested conditions. The comparison does not establish wheel RPM, available torque, efficiency, loaded voltage, or ability to carry the final vehicle mass. Test load/wheel contact was not fully specified.

**Status:** User-reported functional baseline; no recorded speed/current measurement.

## Test 2 — Drive motor through L293D, October 16

**Reported setup:** Motor on pins 3 and 6; motor boost OUT+ to pin 8; all driver grounds intended to share supply returns. USB powered the ESP32 board, whose 5 V rail supplied pin 16 and fixed HIGH test jumpers to pins 1 and 2. Pin 7 was instructed to be grounded; exact physical wiring still needs verification. No motor-control software was running. Steering was not included in the initial test.

**Observation:** The motor appeared roughly 50% slower than during the direct 6 V test.

**Interpretation:** This is a visual estimate. Driver voltage loss and/or supply sag are possible explanations; no numerical loss was measured.

**Next comparison:** Under matching load, measure pack voltage, motor boost output, voltage across motor terminals, and current. A request to raise the supply to 8 V was not adopted. The motor boost target remains 6.0 V.

## Test 3 — Motor runs with motor switch off, October 16

**Reported sequence:**
1. With ESP32 USB power disconnected, switching the motor supply did not start the motor.
2. Connecting USB power made the drive motor run while the motor-power switch was believed to be off.
3. Turning the motor-power switch on made the motor run faster.
4. The user visually checked the breadboard and reported no direct connection from pin 16's rail to pin 8.
5. The user clarified the supply pin was 16, not 15.

**Known control condition:** Pin 1 was tied HIGH, keeping the first motor bridge enabled; pin 2 was also tied HIGH. A fixed voltage is a valid static logic state even without software. Pin 16 is logic supply, pin 1 is enable, and pins 2/7 are direction inputs.

**User hypothesis:** Power from USB/ESP32 supply connections was reaching the motor; later GPIO control could disable it.

**Engineering status:** The behavior suggests another supply path remains available with the motor switch off. Its exact path is not confirmed. Continuous enable explains permission to drive, not the source of the unexpected motor power. No current measurement supports calling that power small. Possible causes remain hypotheses, not a diagnosed wiring error or proven chip failure.

**Limitation:** The motor switch alone did not reliably stop the motor in this wiring. GPIO enable control is planned but not implemented or validated. Software stopping does not establish physical power isolation.

**Next investigation:** With all supplies disconnected, inspect switch/common terminals, chip orientation, breadboard rails, pin 8 wiring, and USB/logic connections. Establish reliable voltage readings before further powered diagnosis, higher voltage, or floor testing.

## Test 4 — Multimeter troubleshooting, October 16

**Instrument identification:** User initially named AstroAI AM33D, but described APO/DC/mV displays and battery-test options; exact model and socket labels remain unverified.

**Observations:** DC 20 V reportedly showed 0.00 V on both a portable-charger/battery test and a reportedly charged 9 V battery. A later mV-range test showed fluctuating readings.

**Status:** No reliable L293D loaded voltage/current results were obtained. Changing readings on an unsuitable range are not validated battery-voltage readings. Earlier 4.68 V and 3.65 V reports are retained separately and are not confirmed by this troubleshooting sequence.

**Next check:** Identify actual meter, lead sockets, range, lead continuity, contact quality, and a known source. Voltage measurements use COM and the voltage socket, never the current socket across a battery.

## Test 5 — Charger heating report, October 16

**Observation:** The TP4056-style charging module was reported very hot. The user later removed a bridge described as between B+ and OUT+ and reported the issue fixed.

**Unconfirmed explanation:** On common protected TP4056 boards B+ and OUT+ are already connected internally. Removing that bridge alone does not prove the source of the heating. Actual topology, solder condition, battery polarity, charge-current setting, and battery temperature were not independently verified.

**Status:** User-reported correction; cause and subsequent thermal behavior unconfirmed. Do not list this as a diagnosed or validated repair.

**Plan:** One compatible rechargeable cell on B+/B-, protected load through OUT+/OUT-, correct charge-current setting for the selected cell, and both load switches off while charging. The three ordinary 1.5 V cells are separate from the charger.

## Test 6 — Electronics power branch assembled, October 17

**User-reported setup:** Rechargeable electronics battery -> TP4056-style protected OUT terminals -> electronics on/off switch -> boost converter -> ESP32 development board. This follows the planned electronics branch; it does not change the separate motor supply.

**Reported issue and correction:** While connecting the battery/wires, the user found an overlapping connection, especially on the underside, and inspected the connections to ensure they were separated. The exact pads, type of overlap, and electrical fault were not identified clearly enough to establish a root cause. The user reported correcting it.

**Observation after correction:** The switch reportedly works. The ESP32 board's power LED illuminated and looked similar to its appearance under USB power. The exact LED was not independently identified.

**Conclusion:** User-reported power-up and switch operation are positive functional observations. An illuminated LED does not establish an accurate 5.0 V rail, correct 3.3 V regulation, adequate current capacity, correct charging, or safe thermal behavior.

**Still to verify:** Actual loaded boost output, board 3.3 V rail, battery/current measurements, switch isolation with USB absent, connection inspection, and heating. This report does not independently resolve the earlier very-hot charger or the motor-running-with-motor-switch-off behavior.

**Simulation added:** [Electronics power-chain model](../../simulations/electronics_power_chain.m), with assumed source/path resistance, 5 V conversion efficiency, switch timing, and changing load. Its separate charging figure is an illustrative constant-current loss estimate, not a charge-cycle or thermal-safety validation.

## Test 7 — BLE control, low-duty noise, and steering load, October 17

**Test code:** [Latched BLE test sketch](../../firmware/testing/DualMotorV2_BLE_Latched/DualMotorV2_BLE_Latched.ino), recorded by behavior from the revised code shared in chat. Startup speed setting100; forward/back/left/right remain active after release; integer `speed_N`; lowercase `stop` disables both bridges. Drive and steering are independently latched.

**User-reported observations:**
- Steering makes audible noise.
- Drive motor makes noise around speed setting 30; above that the user reports it runs fine.
- With drive active, left/right steering commands reduce drive-motor speed.
- Both motors reportedly already have ceramic capacitors.
- Latest capacitor experiment: 1,000 uF electrolytic, 50 V rating; noise persisted. Earlier reports named 100 uF. Exact installed value/location/polarity and test conditions have not been independently checked.

**Interpretation:** Code uses 1 kHz PWM on both enable channels; steering is fixed near 50% duty, even at drive speed 100. PWM vibration or steering stall are possibilities. The steering adds current demand to the common motor supply; supply sag/driver loss are possible causes of drive slowdown. Power is not simply divided into equal voltages across the motors.

**Stop command:** `stop` is the intended emergency software command during testing, clearing both motor requests and disabling both bridges. No validated physical emergency-stop or instantaneous mechanical stop is established.

**Status:** Noise remains unresolved. No reliable loaded rail/current/RPM measurements or complete stall/temperature checks were provided. See [investigation and proposed solutions](MOTOR_NOISE_AND_LOAD.md).

## Steering strength and mechanical inspection plan — October 17

**New observation:** The user reports weak steering: the wheels turn only a small amount and the mechanism does not feel sufficiently stable or firmly connected.

**Relevant history:** The user removed some mechanical pieces in an attempt to make it faster. Exact parts and any change to the steering gearing/support have not been identified.

**Possible explanations, not confirmed:** Removed gear reduction or structural support, loose mounting/linkage, gear misalignment, binding/friction, supply/driver losses, or the current fixed approximately 50% steering PWM. No single cause has been established.

**Plan for the next work session (user called it "tomorrow"):**
- Disconnect USB and both batteries before opening or adjusting the steering assembly.
- Inspect the inside, gears, linkage, mounting, shafts, centering arrangement, and end stops.
- Identify the removed pieces and whether restoring them improves support or torque.
- Check free motion and alignment before applying power.
- If lubrication is appropriate for the materials, use only a tiny amount of plastic-compatible lubricant on the relevant mechanical joints/gears, away from electrical parts. No lubricant has been reported applied yet.
- Reassemble securely, then plan brief controlled comparisons with reliable voltage/current readings.

**Electrical/test decision:** Keep motor boost target at 6 V. Raising it to 8 V was proposed by the user but not adopted. A brief steering-only test at 100% PWM, drive disabled, was discussed but has NOT been performed or added to the code. The recorded test sketch still uses approximately 50% steering duty.

**Concept clarification:** The bridges share a supply; they do not split its voltage equally. Motor torque is approximately proportional to winding current, while attainable speed generally increases with applied voltage under comparable load. Available wheel force also depends on gearing and wheel radius. These relationships do not establish the cause of weak steering.

**Status:** Inspection, lubrication, repair, and the proposed duty comparison are pending. No motor/driver replacement, voltage increase, or confirmed fix is recorded.

## Simulation observations

### Initial movement example — October 16
The user shared an Octave screenshot with a curve approaching 0.41 m/s and approximately 3.47 m after 10 s. Wheel diameter 65 mm, wheel RPM 120, and response time 1.5 s were assumed. The upper graph was mislabeled distance; the saved baseline script labels speed.

### Power and GPIO-command example — October 17
The user ran the second script and shared three plots:
- Forward 60% at 2–8 s, disabled 8–10 s, reverse 50% at 10–16 s, disabled afterward.
- Hypothetical speed near +0.24 m/s forward and -0.20 m/s reverse.
- Estimated electronics input near 430 mA, drive-pack input near 380 mA, and about 530 mA while steering was also active.
- Electronics load current drops to zero at the modeled power-off time of 18 s.

These demonstrate script execution and its assumptions, not measured vehicle performance. There was no backfeed, charger, capacitor-transient, torque/mass, or startup-surge validation. See [simulation notes](../../simulations/README.md).

### Electronics power-chain example — October 17

The user shared both output figures from electronics_power_chain.m: regulated 5 V output during the modeled on interval, battery input current about 330/590 mA versus output 200/350 mA, and a separate charger-loss line from 0.45 W at 3.2 V to 0.225 W at 4.1 V. This confirms example execution, not measured battery/charger/boost performance or safe temperatures.

## Next records to collect

| Quantity | Current evidence |
|---|---|
| Motor pack voltage | 4.68 V user-reported; loaded measurement pending |
| Rechargeable cell voltage | 3.65 V user-reported; reliable recheck pending |
| Motor boost output | Adjusted to 6 V per user report; loaded measurement pending |
| Electronics boost output | 5 V target; switch and LED power-up user-reported Oct 17; loaded voltage/current unverified |
| Driver/motor current, wheel RPM | Unmeasured |
| Final mass, wheel size, steering feedback | Unmeasured/unconfirmed |
| Shutdown, disconnect and automatic-mode behavior | Unvalidated |
