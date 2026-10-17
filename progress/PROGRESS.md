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
- [x] Distinguish final plan from temporary USB/manual-jumper bench wiring.

Checked items record documentation and user reports, not independent hardware verification.

## Next hardware checks

- [ ] Identify actual meter, correct its measurement setup, and establish reliable readings.
- [ ] Trace motor power path while motor switch is off.
- [ ] Verify switch common/outer contacts and DC current ratings.
- [ ] Verify charger topology, polarity, charge-current setting, and heating behavior.
- [ ] Confirm converter identities/count and sustained/startup load capability.
- [ ] Measure battery, boost, and motor voltages under matching loads.
- [ ] Measure motor running/starting current and driver temperature.
- [ ] Verify complete rechargeable/5 V electronics branch with safe USB power isolation.
- [ ] Confirm capacitor selection and local supply bypassing.
- [ ] Inspect fit, weigh complete car, and measure wheel size/RPM/gearing.
- [ ] Inspect steering centering, feedback, and limits.
- [ ] Identify sensor/buzzer supply requirements and accessory drivers.

## Next control and software work

- [ ] Remove fixed 5 V control jumpers before attaching ESP32 GPIOs.
- [ ] Assign GPIOs for both enable and four direction signals; specify default-disabled pull-downs.
- [ ] Implement ESP32 vehicle firmware.
- [ ] Verify enable LOW, startup, stop, disconnect, and power-switch behavior.
- [ ] Select phone/computer wireless app and protocol.
- [ ] Integrate ultrasonic, GPS, headlights, and buzzer.
- [ ] Develop GPS/map/API interface if pursued, with actual data rather than simulated telemetry.
- [ ] Test automatic obstacle behavior and stopping distance.
- [ ] Replace simulation assumptions with measurements and a torque/mass model.

[Earlier C3/S3 progress](../documentation/history/CONTROLLER_PROGRESS.md) is retained only as history.
