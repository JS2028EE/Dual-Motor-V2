# MATLAB / GNU Octave power and movement estimates

No packages are required. These scripts are hypothetical estimates, not vehicle firmware.

| Script | Purpose |
|---|---|
| [electronics_power_chain.m](electronics_power_chain.m) | New electronics battery, charger-output path, switch, boost, and separate charging-loss example |
| [movement_power_v2.m](movement_power_v2.m) | Earlier 10 s baseline with nominal 4.5 V pack and 3.7 V electronics cell |
| [power_control_v2.m](power_control_v2.m) | Latest 20 s power/control example using reported 4.68 V pack and 3.65 V cell |

The following baseline sections describe the first file; the latest model is documented below.

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

## Latest model: power_control_v2.m

Run `power_control_v2.m` to reproduce the second example shared in conversation. The user supplied the three-panel Octave result on October 17; it matches the intended command timeline. No Octave runtime is available in the assistant environment, so the saved script has not been independently executed there.

| Time | Intended action |
|---|---|
| 0–2 s | Electronics on; drive disabled |
| 2–8 s | Forward at 60% PWM; assumed steering current during 4–5 s |
| 8–10 s | Drive disabled; buzzer during 8–9 s |
| 10–16 s | Reverse at 50% PWM |
| 16–18 s | Drive disabled; electronics remain on |
| 18–20 s | Both supplies switched off; prescribed speed decays |

Estimated electronics battery current is about 427 mA normally and 435 mA with the buzzer. Motor-pack current is about 377 mA during drive and 528 mA with the assumed steering load. Target speed is +0.245 m/s forward and -0.204 m/s reverse; these are model targets, not measured top speeds. Estimated steady electronics runtime is about 1.87 h for 1,000 mAh with 80% usable capacity and the stated assumptions.

The latest model approximates average motor voltage as signed PWM duty times (6 V - assumed 2 V driver drop), then assumes wheel RPM scales linearly with that voltage. This is an illustrative electrical-to-speed relationship, not a validated motor model. It has no torque, mass, friction, motor inductance, or braking model. Speed decays even after enable drops; this approximates coasting rather than an immediate physical stop.

The drive-current assumption is fixed whenever enabled, so its current plot is the same for 60% forward and 50% reverse. Driver heat excludes logic losses. Current estimates exclude converter idle draw, protection losses, startup/stall surges, and radio peaks. Steering is only a current interval, not a steering-angle model.

Charge/current integration and the separate 5 V/6 V rails represent intended correct wiring. Charging, protection cutoff, capacitors, unintended USB power paths, and mixed-switch operation are not modeled. This script cannot diagnose the observed motor-running-with-switch-off condition.

The figures are evidence of example script behavior. See [bench test log](../documentation/testing/TEST_LOG.md) for actual reported observations.

## Electronics branch: electronics_power_chain.m

Copy the full file into a MATLAB script and Run. It uses standard script syntax, vector arithmetic, loops, and subplot/plot without extra toolboxes; it is also intended for GNU Octave. Neither MATLAB nor Octave execution is available in the assistant environment. Numerical power-balance checks were performed separately; compatibility is not a claim of execution.

### Actual power paths being represented

**Running, USB unplugged:** one rechargeable Li-ion/LiPo cell -> protected charger OUT pair -> positive-side switch -> boost IN pair -> regulated 5 V load.

During battery-powered operation, the TP4056 charging IC is not converting the battery into 5 V. The protection MOSFETs/wiring are represented by a small series resistance. The boost performs the voltage increase. B- remains battery-side; OUT- is system ground.

**Charging, modeled separately:** USB 5 V -> linear charger -> cell, with the load switch OFF. The final figure estimates charger loss as (5 V - cell voltage) times an assumed 250 mA charging current over a constant-current voltage range. It is not a full CC/CV charge model, current-setting recommendation, temperature prediction, or proof that the actual board has the modeled setting.

### Inputs and operation

- 3.65 V is assumed open-circuit cell voltage, held constant over the 20 s example. The earlier user report did not establish it as an open-circuit measurement.
- Capacity is 1.0 Ah (1,000 mAh); it is used to express charge consumed, not as a current limit.
- Internal cell resistance 0.12 ohm, protection/wiring resistance 0.10 ohm, and switch resistance 0.05 ohm are unmeasured assumptions.
- Boost efficiency is assumed 85%, target 5 V; input limit 1 A and minimum input 3.1 V are model guardrails, not actual module specifications.
- Switch is off at 0–2 s, on at 2–18 s, and off afterward. Output load is 200 mA, increasing to 350 mA at 8–12 s.
- Solve I*(Vcell-I*Rtotal) = 5*Iout/efficiency to account for upstream voltage sag. The same current passes through battery, protection path, switch, and boost input. Boost output current differs.
- While off, battery and protected-output voltage remain present, but the load-side switch/boost input and output are modeled at zero. Capacitor discharge and residual converter charge are omitted.

### Interpreting the figures

Figure 1 shows voltages after each stage, battery versus 5 V load current, and losses in battery internal resistance, protection path, switch, and boost. At the higher load, input current rises and voltage drops across resistances increase while output is held at 5 V. The model errors out if its guardrails are exceeded instead of pretending regulation continues.

Figure 2 is the separate charging-loss illustration. The electronics switch is OFF in that scenario; this is not simultaneous charge-and-run.

### Limits and verification

No switching waveform, inductor/capacitor dynamics, control-loop stability, battery state-of-charge/voltage curve, protection cutoff, actual converter current limit, USB backfeed, contact fault, motor, or temperature model is included. Converter idle current and charger standby current are omitted. This is a steady-state estimate at each time sample.

Power dissipated as heat is not temperature. Real temperature needs thermal resistance, environment, and measured behavior. The new power-LED observation and simulated 5 V output do not replace meter verification.

Sources: [TI Working with Boost Converters](https://www.ti.com/lit/an/snva731/snva731.pdf) for power balance, and [TP4056 protected-module vendor documentation](https://shillehtek.com/blogs/shillehtek-product-manuals/tp4056-1a-lipo-battery-charging-board-type-c-with-current-protection-manual) for battery/load terminals and charging separation. Verify actual board and battery ratings.
