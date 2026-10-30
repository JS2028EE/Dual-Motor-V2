# Portable battery and updated control simulation (V3)

**Script:** [portable_battery_control_v3.m](portable_battery_control_v3.m)  
**Purpose:** Estimate the motor and electronics power paths with the project's newer two-battery arrangement and documented BLE drive/steering behavior.

## How to run

Open GNU Octave, set the current working directory to the repository's \`simulations\` folder, then run:

\`\`\`octave
portable_battery_control_v3
\`\`\`

The script requires no additional packages and generates four figures.

## Four tests

1. **Steering only:** left command at 1.00 s and right command at 4.00 s. Each has a 50 ms direction-change delay followed by a 200 ms steering pulse.
2. **Driving only:** forward command from 0 to 8 seconds, followed by stop.
3. **Combined control:** forward remains active from 0 to 8 seconds. Left steering is commanded at 2 seconds; right steering at 5 seconds. The 200 ms steering pulses do not end the forward command.
4. **Power breakdown:** estimated battery/wiring loss, boost-converter loss, boost-output droop loss, L293D motor-path loss, motor-terminal power, and path efficiency.

The combined test is an illustrative command sequence matching the described behavior; it is not a recorded firmware trace. The exact firmware pulse timing should be checked against the currently installed sketch before treating the timeline as an exact reproduction.

## Updated battery arrangement

| Branch | Modeled arrangement |
|---|---|
| Motors | 3.7 V, 1,000 mAh cell -> dedicated boost converter -> 6.0 V target motor rail -> L293D -> motors |
| ESP32 and logic | Separate 3.7 V, 500 mAh cell -> dedicated boost converter -> 5.0 V electronics rail |
| Charging | TP4056 is not treated as a running 5 V regulator; charging operation is not simulated |

The intended motor and electronics positive rails are separate. Their grounds must be connected as required by the circuit. The model assumes correct wiring and does not simulate USB backfeed or unintended current paths.

## Inputs and evidence status

| Input | Value | Status |
|---|---:|---|
| Motor cell nominal voltage | 3.70 V | User-reported nominal cell |
| Motor cell capacity | 1,000 mAh | User-reported |
| Electronics cell nominal voltage | 3.70 V | User-reported nominal cell |
| Electronics cell capacity | 500 mAh | User-reported |
| Motor boost target | 6.00 V | Intended target, not a logged loaded measurement |
| Electronics boost target | 5.00 V | Intended target |
| Boost efficiency, both branches | 85% | Assumption; not measured |
| Motor boost output resistance | 0.25 ohm | Placeholder to illustrate output droop; not measured |
| Battery/protection/switch/wiring series resistance | 0.15 ohm per branch | Placeholder; not measured |
| L293D motor-path conducting drop | 2.00 V | Simplified assumption; varies with current and temperature |
| Drive motor-rail current | 250 mA | Retained estimate from earlier simulation; not measured |
| Steering motor-rail current | 100 mA | Retained estimate, treated as average during pulse; not measured |
| ESP32/BLE 5 V current | 180 mA | Assumption |
| L293D logic 5 V current | 35 mA | Assumption |
| Wheel diameter | 65 mm | Simplified model input |
| Wheel RPM at 4 V | 120 RPM | Assumption, not measured |

The 0.25 ohm boost-output value is deliberately exposed as an assumption so its effect can be reviewed later. It must not be interpreted as a measured converter output resistance or a manufacturer specification. The 85% efficiencies are also placeholders.

## What is and is not represented

The battery model estimates voltage sag from a constant equivalent series resistance and solves the battery input current needed to supply converter input power. For the motor branch, converter input power is calculated from the nominal 6 V output and load current; the assumed boost-output resistance then accounts separately for the rail voltage drop and its associated loss. This avoids counting the same output droop as both reduced converter output power and a separate loss. The boost converter is otherwise represented by a fixed efficiency. The L293D is represented by a fixed total conducting drop in the motor-current path.

The speed graph uses a simplified linear relationship between motor-terminal voltage and wheel RPM, followed by a first-order response. It does not model torque, motor winding resistance/inductance, stall current, gearing, vehicle mass, friction, tire slip, or braking. The steering graph shows command pulses and electrical demand; it does not predict steering angle or turning radius.

The script includes the two battery branches, boost losses, approximate L293D loss, and aggregate assumed battery/protection/switch/wiring resistance. It does not assign continuous power consumption to the pull-down resistor, motor ceramic capacitors, or bulk capacitor. Capacitors can affect transients/noise but cannot supply sustained motor power. Their exact installed values, ESR, and placement are not established in this model. The TP4056 charging process is outside the running-power simulation.

GPS and ultrasonic sensor loads are excluded because their installed configuration and measured current are not yet part of this test.

## Runtime estimates

The script prints charge used during each 10-second test and continuous-load runtime scenarios. Runtime values are mathematical extrapolations from assumed steady current and an 80% usable-capacity planning factor. They are not expected driving-session runtimes and do not account for battery discharge limits, voltage cutoff, temperature, aging, or changing load.

## Validation plan

1. Run the script in GNU Octave and preserve screenshots of all four figures and the Command Window output.
2. Compare steering-only, driving-only, and combined predictions with measured battery current and rail voltage.
3. Measure motor-rail voltage and motor-battery current under each condition; use suitable meter/current-measurement methods and avoid shorting the cell.
4. Replace assumptions only when there is a measurement or a datasheet value to support the change.
5. After sensors/GPS are installed, add their measured electronics-rail currents and repeat the power budget.

**Status:** Code and model documentation are committed as a prediction baseline. The script has not yet been run by the user for this V3 revision. No graph or simulation output is being represented as a measured vehicle result.
