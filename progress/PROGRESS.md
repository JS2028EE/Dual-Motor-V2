# Current V2 progress

Updated October 17, 2026. Follow the [block diagrams](../diagrams/README.md) and [current decisions](../documentation/DECISIONS.md). No C3/S3 controller is part of this build.

## Documented / user-reported work

- [x] Select ESP32-WROOM-32D with phone/computer control.
- [x] Document one drive motor and one steering motor with L293D.
- [x] Plan front ultrasonic, GPS, buzzer, and two white headlights.
- [x] Document two-boost architecture: 6 V motors and 5 V electronics.
- [x] Include TP4056-style protected charging and separate on/off branches.
- [x] Correct switch description: reported three-terminal device, likely SPDT used for on/off.
- [x] Document shared OUT- load ground and separate raw battery B-.
- [x] Record reported 4.68 V motor pack and 3.65 V electronics cell.
- [x] Record direct boosted drive-motor test.
- [x] Record drive-motor test through L293D and visual speed comparison.
- [x] Record switch-off motor operation, charger heating, and meter trouble as unresolved.
- [x] Record user execution/screenshots of both Octave examples.
- [x] Save latest power/control Octave script and explicit assumptions.
- [x] Record electronics switch/LED power-up and battery/wire-overlap correction report.
- [x] Add MATLAB/Octave electronics-chain voltage/current/loss model with separate charging example.
- [x] Distinguish final plan from temporary USB/manual-jumper bench wiring.

Checked items record documentation and user reports, not independent hardware verification.

- [x] Record revised BLE motor-only test code, GPIO mapping, latched commands, and software stop behavior.
- [x] Document drive noise near speed 30, steering noise, shared-load slowdown, and latest 1,000 uF/50 V capacitor experiment.
- [ ] Diagnose noise and shared-load voltage/current behavior; characterize steering limits.

## Next hardware checks

- [ ] Identify actual meter, correct its measurement setup, and establish reliable readings.
- [ ] Trace motor power path while motor switch is off.
- [ ] Verify switch common/outer contacts and DC current ratings.
- [ ] Verify charger topology, polarity, charge-current setting, and heating behavior.
- [ ] Confirm converter identities/count and sustained/startup load capability.
- [ ] Measure battery, boost, and motor voltages under matching loads.
- [ ] Measure motor running/starting current and driver temperature.
- [ ] Electrically validate reported rechargeable/5 V branch power-up: loaded voltage/current, switching, heating, and USB power isolation.
- [ ] Confirm capacitor selection and local supply bypassing.
- [ ] Inspect fit, weigh complete car, and measure wheel size/RPM/gearing.
- [ ] Inspect steering centering, feedback, and limits.
- [ ] Open steering mechanism with power disconnected; identify removed parts, gear reduction, mounting support, linkage alignment, and binding.
- [ ] Restore needed support/gearing and assess minimal material-compatible lubrication if appropriate.
- [ ] After mechanical inspection and electrical checks, consider a brief steering-only 100% duty test at the unchanged 6 V rail; not yet performed.
- [ ] Identify sensor/buzzer supply requirements and accessory drivers.

## Next control and software work

- [ ] Remove fixed 5 V control jumpers before attaching ESP32 GPIOs.
- [x] Select motor-only test GPIOs 25/26/27 and 14/32/33; specify separate 10k enable pull-downs.
- [ ] Electrically verify GPIO mapping, pull-downs, and power-off behavior.
- [x] Record BLE motor-control testing sketches.
- [ ] Integrate and validate complete ESP32 vehicle firmware.
- [ ] Verify enable LOW, startup, stop, disconnect, and power-switch behavior.
- [ ] Select phone/computer wireless app and protocol.
- [ ] Integrate ultrasonic, GPS, headlights, and buzzer.
- [ ] Develop GPS/map/API interface if pursued, with actual data rather than simulated telemetry.
- [ ] Test automatic obstacle behavior and stopping distance.
- [ ] Replace simulation assumptions with measurements and a torque/mass model.

[Earlier C3/S3 progress](../documentation/history/CONTROLLER_PROGRESS.md) is retained only as history.
