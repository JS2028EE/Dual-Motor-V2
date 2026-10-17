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

## Test 8 / design revision — Steering stall hypothesis and pulse control, October 17

**Additional bench context:** The user received a bench supply and reported the noise/drive slowdown still occurred during combined drive and steering tests. Actual loaded voltage, current, and CV/CC readings were not supplied, so a supply limit versus driver/mechanical cause remains undetermined.

**User's observation and reasoning:** The existing steering command stayed active after pressing left/right. Once the mechanism could not turn further, the motor could continue pushing against the linkage/end stop. The user identified this as a likely source of noise and extra power demand.

**Firmware inspection:** The previous latched sketch confirms steering enable remained active until stop, a direction change, disconnect, or another stop condition. That supports the stall hypothesis but does not prove that the motor was physically stalled or quantify its current.

**Requested and implemented revision:**
- Drive forward/back remain latched.
- Reverse only drive electrical polarity to compensate for reported swapped motor wires.
- Left/right become a single brief steering movement followed by steering output disable.
- Preserve integer speed_N messages, startup setting100, BLE identity, GPIO wiring, 1 kHz PWM, and approximately 50% steering duty.
- Initial pulse is150 ms of actual energization, not a calibrated steering angle. Same-side repeat packets do not extend/retrigger it; opposite direction or stop re-arms steering.
- Stop/disconnect still clears both drive and steering. Stop cancels a pending pulse.

**Energy interpretation:** Removing continuous steering power should reduce steering energized time compared with holding it on, especially if it was stalled. Actual energy savings, current reduction, and noise/speed improvement have not been measured.50% PWM is not necessarily50% total supply power.

**Mechanical limitation:** Once steering power is removed, this driver does not lock wheel position. Position retention depends on gears, friction, centering springs, and external load; the wheels may move or return. No feedback/limit sensing exists. A timed pulse can still stall briefly if an end stop is reached before it ends.

**Artifacts and status:** [Current pulse-steering test code](../../firmware/testing/DualMotorV2_BLE_Pulse/DualMotorV2_BLE_Pulse.ino) added; prior code preserved. Host parser/control tests passed. Actual board compilation and hardware retest remain pending. Keep6 V motor rail and compare steering movement, noise, drive speed, supply voltage/current, and CV/CC behavior.

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


## Test 9 — User-reported hardware results after 150 ms steering cutoff

October 17, 2026. First reported physical trial of the pulse-steering revision. The report indicates the revised behavior was tried on hardware; the build log, exact flashed source, loaded rail voltage, and current readings were not supplied.

### Reported results

- Continuous steering noise stopped. Previously one left/right command caused ongoing noise; now noise is brief during steering activation.
- Steering remains very weak and does not turn adequately during the 150 ms pulse.
- Driving is reported faster/more usable during steering than with continuous steering. A brief slowdown may remain while steering is powered.
- User estimated the brief disturbance at approximately 175–200 ms by observation. This is not instrumented timing and does not change the configured 150 ms energization time; the firmware also includes a direction-transition guard.
- Driving reportedly takes roughly three seconds to approach a steady speed; no measured RPM, distance/time trace, or acceleration data.
- The car veers sideways. Alignment, starting wheel position, mechanical play, friction, and steering position retention remain to be inspected.
- Low-speed driving noise around speed_30 remains. User proposes insufficient torque against the mechanism/load; PWM excitation, friction, supply limits and driver losses remain alternatives or contributing causes.
- Both motors reportedly have ceramic capacitors marked 104. Under the usual three-digit code this means 100,000 pF = 100 nF = 0.1 uF, not 104 pF. Actual parts were not inspected.
- No electrolytic capacitor was installed during this particular trial. Earlier capacitor experiments are separate tests and are not contradicted by this report.
- Motor pack is three 1.5 V cells in series, now reported around 4.3 V. Loaded voltage, boost output under load, and current remain unknown.

### Interpretation

The disappearance of continuous noise after a bounded command supports the claim that sustained steering energization contributed to the persistent noise. It does not independently prove a stall, quantify energy savings, or establish the cause of every remaining noise. Improved driving is user-reported, not a measured speed or power result. The mechanism does not allocate a fixed fraction of supply power to each motor.

A longer pulse increases energized duration, not automatically instantaneous current, voltage, or torque. User proposes 200–230 ms; these are about 1.33–1.53 times 150 ms, not double. Double would be 300 ms. A longer pulse may allow more travel if it is still moving, but can extend a stall if the mechanism is blocked.

### Next work — proposed, not implemented

Firmware remains at 150 ms; no parts, supply setting, or capacitor were changed by this documentation update. Inspect steering friction, linkage, removed gears, alignment, and load on the front wheels. Compare lifted wheels with wheels on the surface. For the same front load, tire friction can resist turning; mechanical leverage/gearing also affects available steering force. More pulse time cannot repair binding or replace missing gear reduction.

Measure source current and voltage before selecting another battery or changing a supply current limit. A source's current capability/limit is not current forced into the motor. A 500 mAh rating is charge capacity, not a 500 mA maximum output rating.

Proposed current measurement: with power off, black lead COM, red lead 10 A, dial DC A / 10 A. Break the positive lead feeding the motor boost, connect source-side positive to red probe and black probe to boost IN+, leaving IN- on battery negative. This inserts the meter in series and measures boost INPUT current, not individual motor current. Never place a meter in current mode across battery terminals. Follow the meter's time limits; return red lead to V-ohm-mA before measuring voltage. A normal meter may miss the 150 ms steering peak. Alternatively, a bench supply display can show total current and CV/CC mode, but may also miss a short pulse.

For voltage: red lead V-ohm-mA, black COM, DC V 20 V range; measure across the battery or across motor-rail pin 8 and common ground with the load operating. Power off before moving wiring or meter jacks.

Replacement 3.7 V / 500 mAh cell is only a proposal. Cell discharge rating, protection, charger compatibility and boost capability must be established. Boost conversion approximately obeys Iout = efficiency * Vin * Iin / Vout: at 3.7 V input, 5 V output and assumed 85% efficiency, Iout is about 0.63 * Iin. A voltage boost reduces available output current for a given input current; it does not create power. “5 volts” is the voltage target, not “5 milliamps.” No battery swap or new motor rail voltage has been adopted.

### Outcome

Partial success: continuous steering noise resolved in the reported trial; driving behavior improved by observation. Remaining work: weak steering, brief activation noise, low-duty drive noise, sideways travel, and supply/load measurements. Actual power savings, steering angle, stall current and cause of remaining weakness are not yet verified.


## Test 10 — Three-test steering plan and 200 ms candidate

October 17, 2026. User requested a duration trial, a bench-source trial, and a combined trial, all with the electrolytic capacitor installed. See [full matrix](STEERING_TEST_MATRIX.md). Baseline with the same capacitor is added to distinguish capacitor effects. Current firmware now uses 200 ms; use 150 ms for baseline and source-only comparison. 230 ms is a later candidate, not implemented. Source remains 6 V, not an overvoltage trial. Host logic tests passed; hardware results pending. Prior across-terminal current readings are invalid and are not used to size the supply.


## Test 11 — Test 1 duration trial: unloaded steering improves, loaded steering remains weak

October 17, 2026. User identifies this as Test 1 of the planned matrix, following the 200 ms candidate update. Exact flashed pulse value and capacitor installation/value were not independently confirmed in this report. No voltage/current traces, CV/CC readings, or measured steering angles were supplied.

### Observed conditions

| Condition | User-reported result |
|---|---|
| Car held with wheels clear of ground; driving motor OFF | Steering works well and responds in time. |
| Wheels lifted; driving motor ON | Steering barely moves compared with drive-off operation. |
| Car on ground, stationary; driving motor OFF | Steering attempts to move but barely turns. |
| Car on ground and moving | Steering is reported even worse. |
| Driving while issuing a steering command | A noticeable brief driving slowdown, described as under a second. |

These conditions must remain separate: "works in the air" applies specifically to drive-OFF operation, not all lifted-wheel tests. Increased pulse time has not solved steering under actual vehicle load. Earlier continuous-noise improvement is retained as a separate successful change.

### User explanation and proposed fixes

Jay attributes the drive-on weakness to inadequate shared power and the ground-only weakness to tire friction/load. He proposes inspecting the steering mechanism, correcting binding or gearing, suitable lubrication, weight distribution, a different battery, separate motor supplies, or a later software mode that reduces driving to about 50% duty during a stronger steering command. Reducing total weight is constrained by the existing components and planned buzzer/sensor.

These are hypotheses and proposed changes, not confirmed causes or implemented fixes. Tire scrub and front-wheel load can increase steering torque demand, while changes with driving energized suggest electrical interaction; driving vibration or linkage effects could also contribute. A moving car is not universally harder to steer than a stationary car: this is the user's observation on this mechanism and surface, not a general friction rule.

### Interpretation and useful next steps

- Timing alone is insufficient in this trial. More time can increase travel only while the mechanism can move.
- The firmware still uses approximately 50% steering PWM (128/255). Pulse duration and PWM duty are separate variables. Increased steering duty is a possible later test, not implemented or established safe for this motor/load/driver.
- Reducing drive duty during a steering pulse may reduce competing supply demand. It does not allocate a fixed percentage of total power or guarantee increased steering torque. Actual current and rail behavior depend on the loads/source.
- Inspect linkage, removed gear pieces, leverage and mechanical alignment with power disconnected. Restoring gear reduction may increase steering torque at lower speed; removing gears to gain speed can reduce torque. Use only a small amount of plastic-compatible lubricant on appropriate pivots/gears, not tires, motor brushes or electronics.
- Compare the same steering action with the front wheels lifted versus on the ground, keeping initial angle and source settings consistent. Front axle load, tire contact/surface, and any interference deserve attention.
- A different battery could help only if its current capability and the boost/driver path meet the load. Lithium chemistry or mAh capacity alone does not prove this.
- A single L293D has one motor-supply pin (8) shared by both bridges. Separate motor power supplies would require separate suitable drivers or a redesigned driver arrangement; do not connect two supplies to pin 8 or put a motor across an external source while still connected to driver outputs.
- Driver voltage drop and thermal/current limits remain possible constraints even with an adequate source. Source: https://www.ti.com/lit/ds/symlink/l293.pdf .

### Next test and comparison controls

Continue the planned source-only Test 2 at 6 V bench supply and 150 ms, then the combined Test 3 at 6 V and 200 ms. Keep capacitor, steering duty/frequency, starting angle, surface/load, and drive command the same. The current download defaults to 200 ms, so explicitly edit STEER_PULSE_MS to 150 and upload for Test 2. If the user instead retains 200 ms on the bench, record it as Test 3, not a source-only comparison.

Use the previously documented source-disconnection wiring and current-limit procedure. Record CV/CC and actual displayed voltage before deciding the supply solved the problem. If steering remains weak on the ground while the bench holds 6 V in CV, investigate mechanics, available motor/driver torque and steering duty rather than assuming higher voltage is needed.

### Status

Test 1: partial improvement when unloaded; fails the practical requirement to steer under ground load/while driving. Test 2 and Test 3 results pending. No firmware, PWM duty, voltage, capacitor wiring, separate supply, battery chemistry, or mechanical change was implemented by this documentation update.


## Mechanical setup update before Test 2

October 17, 2026. Jay reports replacing the steering gears with lighter gears; the mechanism still turns. This is a user-performed hardware modification, not a firmware change. Gear tooth counts, gear ratio, material, mass, geometry, friction, torque effect, and exact timing relative to Test 1 were not provided. The statement about whether these gears make steering harder was incomplete; no torque improvement or reduction is claimed.

Record this modification as part of the installation/setup history before interpreting Test 2. Lower gear mass alone does not establish improved steering torque. Do not assume the reduction ratio stayed the same.

Keep this gear setup fixed for the battery baseline and bench-source comparison. If gears changed since the battery trial, repeat the battery baseline with the replacement gears and the same 150 ms pulse/capacitor/load before comparing it with the source-only bench test. Otherwise differences could arise from mechanics as well as source. Test 2 remains pending; no new movement result, firmware change, or supply setting is recorded by this note.



## Test 12 — Bench-source observation and USB Serial instrumentation

October 17, 2026. User reports CV with wheels lifted, including drive/steer activity. On the ground, driving alone reaches CC and driving plus steering also stays in CC; steering remains weak. Actual set current limit, loaded voltage/current, gear configuration details and exact flashed pulse were not supplied. This identifies source current limiting under the reported ground load; it does not prove the original battery was the sole cause. Increasing the bench limit gradually toward the previously agreed 0.60 A total ceiling was suggested, but no new setting/result is confirmed here.

At user request, current test code adds USB Serial commands and command-state logs. Pulse remains 150 ms. No claimed voltage/current sensing or automatic power allocation was added. See [Serial guide](SERIAL_CONTROL.md). Host tests pass; actual ESP32-core compilation and serial hardware verification remain pending.


## Test 13 — Bench-source improvement reported; start combined 200 ms trial

October 17, 2026. User reports the Serial firmware compilation issue is resolved. With the bench source at 6 V and a reported 0.60 A setting, driving is described as working well, but the user considers the 150 ms steering time insufficient. User identifies source capability and pulse duration as the likely combined causes and requests the longer pulse.

The user also reports an operating-current figure of 0.55 A. Unit correction: 0.55 A = 550 mA, not 55 mA. It is not yet established whether 0.55 A was an actual live output-current measurement, a current-limit setting, or a peak/steady value, nor whether it referred to driving alone or simultaneous steering. It is not documented as a motor rating or universal minimum. Actual loaded voltage, CV/CC at the improved setting, quantitative steering travel and source current trace were not supplied.

The reported improvement supports investigating source capability, but does not establish all power issues solved, eliminate driver/mechanical constraints, or specify a suitable portable battery. Bench capability does not directly translate to battery/boost capability.

At user request, current STEER_PULSE_MS is restored to 200 ms for Test 3. USB Serial and BLE controls, command-state logs, drive latch, reversed drive mapping, steering duty 128/255 (~50%), PWM frequency, stop behavior and startup speed remain unchanged. No automatic steering priority or drive duty reduction was added. Supply remains the previously agreed 6 V with a maximum 0.60 A total bench limit for this supervised test; no additional current/voltage increase was requested or implemented.

Both host pulse/drive and USB/BLE tests passed at 200 ms. The serial test now derives its timing from the actual sketch constant. Full ESP32-core compilation of this new revision and Test 3 hardware results remain pending.

Next: use the same bench wiring, gear setup, starting angle, capacitor arrangement and surface/load as the 150 ms bench trial. Compare steering travel with driving off/on, lifted/on ground, noise duration and brief driving slowdown. Record actual supply voltage/current and CV/CC where available. A longer pulse helps only while movement can continue; stop if binding or a mechanical end stop prevents movement. Portable supply selection remains future work.
