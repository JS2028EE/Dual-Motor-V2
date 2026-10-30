# Battery allocation update — installed and functionally tested; electrical validation pending

Updated October 30, 2026. This records the user's latest reported portable setup and supersedes the earlier three-cell motor-pack allocation. The user reports successful driving and steering using the new arrangement and the custom BLE website. This is a functional report, not instrumented validation of current, voltage, runtime or thermal margins.

| Branch | Current reported battery | Boosted rail | Latest status |
|---|---|---|---|
| Motors | 3.7 V rechargeable cell, 1,000 mAh | Dedicated boost converter; 6 V is the documented target for the 3–6 V motors / L293D motor supply | Installed and reported working for driving and steering; loaded voltage/current and protection ratings still need measurement |
| ESP32/electronics | Separate 3.7 V rechargeable cell, 500 mAh | Dedicated boost converter to 5 V input on ESP32 development board | Installed and reported working; loaded 5 V, current capability, charging and thermal checks remain |

The previous motor source was three 1.5 V cells in series (4.5 V nominal; an earlier pre-test reading was reported as 4.68 V). That pack is no longer used for the motor branch. The new arrangement uses one 1,000 mAh cell for the motors and a separate 500 mAh cell for the ESP32/electronics. Keep the two positive supply rails separate and use the common circuit ground required by the ESP32 and L293D.

## Why the source was changed

Earlier ground-load bench tests indicated that the previous motor source entered constant-current (CC) limiting, while a 6 V bench source set to a 0.60 A limit supported combined driving and steering with a reported display reading up to approximately 0.55 A. This supports the hypothesis that source capability contributed to the earlier weak steering and drive slowdown.

Jay's working estimate is that the old pack could not provide enough current, with approximately 600 mA discussed as the relevant demand. This is not yet a verified continuous motor-current requirement. The earlier 0.55 A was a bench display observation, not a minimum requirement, and the L293D's 600 mA per-channel specification is a driver rating, not a measured motor demand. The new cell's current at its 3.7 V side will be higher than current delivered at a 6 V boosted output, depending on converter efficiency and load.

## Functional test update — October 30, 2026

Jay reports that the new battery arrangement now drives the vehicle successfully and that steering works well. The custom browser-based BLE website was used as the controller instead of a generic BLE app. The ESP32 remained on its previously uploaded firmware for this website test; the website handles W/A/S/D key press/release behavior and sends the existing movement and stop commands. No new ESP32 firmware upload was needed for this interface-side behavior.

This confirms a user-reported functional milestone only. No loaded rail readings, cell-side current, converter temperature, battery runtime, steering angle or endurance measurements were provided.

## Electrical checks still required

- Confirm each cell's exact label, chemistry, protection circuit and continuous/pulse discharge ratings.
- Confirm the motor boost converter can supply the required sustained and startup power at its configured output; measure output voltage during drive and simultaneous steering.
- Confirm the ESP32 boost converter holds an appropriate 5 V input under Wi-Fi/BLE and peripheral loads; measure the board's 3.3 V rail as appropriate.
- Measure cell-side current and boosted-rail current separately under repeatable driving and steering loads.
- Check converter, driver, wiring and cells for abnormal heating; stop if there is odor, swelling or rapid heating.
- Verify compatible charging arrangements and charge-current settings for each cell independently. Do not assume a TP4056-style single-cell board is suitable for both cells together.
- Verify switching and USB/external-power isolation, and continue investigating the previously observed motor operation while its switch was believed off.
- Keep the motor and electronics positive rails separate; share ground only as required by the circuit topology. Do not bypass a cell protection board by joining raw battery negative to protected load negative.

mAh is charge capacity, not a discharge-current rating. The fact that the vehicle moves does not by itself establish safe current margin, converter efficiency, battery runtime, charging safety or thermal reliability.

See [driving/steering milestone](../testing/DRIVING_STEERING_MILESTONE.md) and [test log](../testing/TEST_LOG.md).
