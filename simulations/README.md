# GNU Octave estimates

Run `movement_power_v2.m` in GNU Octave. No packages are required.

## What is modeled

The movement section prescribes a speed curve approaching the speed implied by 65 mm wheels at 120 wheel RPM, with an assumed 1.5 s response time. Distance is the numerical integral of that speed.

The electrical section compares direct 4.5 V and candidate boosted 6 V motor supplies using an assumed 2 V L293D loss. It also estimates separate electronics-battery runtime.

**Changing the boost setting does not change the movement graph.** Voltage, torque, gearing, mass, tire slip, and friction are not mechanically coupled in this initial model. Steering is not simulated.

## Example outputs

| Quantity | Approximate result |
|---|---:|
| Direct motor terminal voltage | 2.50 V |
| Boosted motor terminal voltage | 4.00 V |
| Total assumed motor load while both are active | 350 mA |
| Motor battery current with 6 V boost at 85% efficiency | 549 mA |
| Driver heat from motor paths only | 0.70 W |
| Electronics current at 5 V | 275 mA |
| Electronics battery current at 3.7 V | 437 mA |
| 1,000 mAh electronics runtime, 80% usable | 1.83 h |
| 500 mAh electronics runtime, 80% usable | 0.91 h |
| Prescribed steady speed | 0.41 m/s |
| Distance after 10 s | 3.47 m |

Motor-pack runtime is not estimated because cell capacity and actual current are unknown. Starting/stall surges, radio peaks, battery sag, converter dynamics, charging, and capacitor transients are not simulated.

## Verification record

On October 16, 2026, the user ran the initial script in Octave and supplied a screenshot showing the expected rising speed curve and distance near 3.47 m at 10 s. Its upper axis was labeled distance; this saved script labels it speed.

The numerical estimates were independently checked using arithmetic. This updated script adds a boost comparison and has not yet been executed in Octave in the assistant environment. The screenshot is evidence of the initial example running, not of real vehicle movement or validation of this updated file.

## Next measurements

Replace wheel diameter, driven-wheel RPM, motor current, driver loss, battery voltage, and electronics currents with measured values. Then add motor torque/gearing, mass, resistance, and braking data for a coupled movement model.
