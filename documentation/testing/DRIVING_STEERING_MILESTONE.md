# Driving and steering milestone

October 17, 2026. Jay reports a successful combined bench-powered driving/steering test after approximately a day and a half of debugging. This is user-reported hardware evidence, not independent instrumented validation or a completed autonomous vehicle.

## Result

With the bench supply set to 6 V and a 0.60 A current limit, and the current 200 ms steering pulse, Jay reports that the car drives and turns successfully, including simultaneous driving/steering and direction changes. Earlier reports separated unloaded steering, ground contact, and driving-on behavior. The combined test is now reported successful under the tested conditions.

The supply display reportedly did not exceed approximately 0.55 A during the observed driving/steering/reversing or higher-friction interactions. This is a live display observation in this report, not a motor nameplate rating, an established universal minimum, or a bound on sub-display-timescale current peaks. 0.55 A is 550 mA. The supply was set to allow 0.60 A; that does not force 0.60 A into the load. Jay described roughly 0.50–0.60 A as a useful tested source-limit range, not a finalized portable-supply specification. Exact CV/CC behavior and loaded voltage at each interaction were not supplied.

## What the investigation established

1. The original software kept steering energized after a left/right command.
2. Jay proposed that the steering motor might remain forced against its mechanism, creating noise and extra demand.
3. A 150 ms pulse eliminated the reported continuous steering noise, but steering remained weak under load.
4. Test 1's longer pulse helped unloaded travel but did not solve battery-powered ground steering.
5. Bench-source tests exposed CC operation when driving on the ground at the earlier setting, supporting a source-current-limit contribution.
6. A 6 V source with a 0.60 A limit plus 200 ms steering is now reported to deliver usable driving and steering together.

These results support the combined timing/source explanation for the tested setup. They do not independently prove an end-stop stall, quantify separate contributions, certify peak motor current, establish driver thermal margins, or show every earlier power issue resolved. Lighter replacement steering gears are part of the reported mechanical history; exact ratio and timing of that change were not measured. Capacitor use was planned throughout the matrix, but actual installation/value for the final run was not explicitly reconfirmed.

## Preserved firmware

Current sketch: [DualMotorV2_BLE_Pulse.ino](../../firmware/testing/DualMotorV2_BLE_Pulse/DualMotorV2_BLE_Pulse.ino).
- ESP32-WROOM-32D; L293D; no C3.
- USB Serial and BLE controls.
- 200 ms steering pulses, fixed 128/255 steering duty at 1 kHz.
- Latched driving, reversed drive polarity, startup speed 100.
- stop disables both motors; repeated same-side steering is suppressed until opposite or stop.
- Serial logs are commanded state/timing, not current/voltage/GPS measurements.

The prior Serial compilation issue was user-reported fixed. The user reports operating the revised behavior on hardware; build logs and exact binary were not retained. Host tests passed; these are separate from user hardware reports.

## New portable-power decision

Jay selects the 1,000 mAh cell for the motor branch and the smaller cell for ESP32/electronics, replacing the earlier three-cell motor-pack allocation. The smaller cell was previously identified as 500 mAh; the latest spoken “550 milliamps battery” is ambiguous. Confirm its label/capacity rather than treating 550 mA load current as capacity.

This is selected intent, not an installed/validated battery swap. The motor branch still needs 6 V regulation; electronics needs 5 V regulation into the verified development-board input. Two independent positive rails remain separate. Verify each cell's chemistry, condition, continuous/pulse discharge ratings, protection, converter input/output capability and a compatible charge-current setting. Any motor-cell charger/protection arrangement must be specified before wiring; do not put both cells on one TP4056 or parallel/series them as part of this decision.

Capacity alone does not establish sufficient discharge current. At an assumed 3.7 V cell voltage and 85% conversion efficiency, 6 V at 0.55 A output requires approximately 1.05 A cell input, with more at lower cell voltage or higher demand. This is a sizing example, not a tested cell rating or runtime guarantee.

See [battery reallocation](../power/BATTERY_REALLOCATION.md).

## Next interface and accessory work — deferred

Jay wants better simultaneous drive/steering control, such as a phone UI with independent drive and steering controls or computer WASD input, instead of the present one-click interaction. Speed control may be fixed, selectable or customizable; no final choice was made. A future interface may show GPS-derived speed, location/map and manual/automatic modes, once the GPS and other hardware are integrated and actual data is available.

GPS, ultrasonic/other optical sensing (exact sensor not selected in this report), buzzer and headlights remain planned. No UI/app/site, map, GPS velocity, autonomous mode or accessory integration was implemented in this update. The current session ends with cleanup, not new feature development.

## Shutdown and unresolved checks

Jay briefly mentioned a burnt smell, then withdrew the concern; there is no confirmed burning component or damage diagnosis. Turn outputs/switches off and disconnect power for cleanup. If an odor recurs or a component is unusually hot, keep power disconnected and inspect before retesting. Driver temperature, source/boost load margins, charger behavior and the previously unexplained motor-switch-off power path remain open; movement success does not validate isolation or charging.

## Engineering reflection

This is a concrete observe–hypothesize–change–compare–report cycle. Jay connected mechanics, code and source behavior, tested unsuccessful intermediate changes and reported a working combined configuration. The record distinguishes his observation/hypothesis from AI-assisted code/documentation and unmeasured electrical claims. That progression supports a credible engineering portfolio; no admissions outcome or evaluator response is claimed.
