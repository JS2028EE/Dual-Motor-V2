# Steering investigation and engineering reflection

October 17, 2026. This record summarizes Jay's observations, explanation, proposed fixes, and learning. The first-person section is an edited paraphrase of his explanation, not a verbatim quotation. The admissions discussion is a hypothetical interpretation, not an admissions evaluation or predicted outcome.

## Latest hardware update

A later trial on October 17 reported that continuous steering noise stopped with the 150 ms cutoff, driving improved by observation, but steering remains weak and brief noise persists. See [Test 9](TEST_LOG.md#test-9--user-reported-hardware-results-after-150-ms-steering-cutoff). Earlier pending-validation statements below describe the state when this reflection was first written; measured current, voltage, speed, angle, and energy results are still pending. No pulse-duration increase has been implemented.

## My observation and explanation

I noticed that the driving motor slowed down when I also pressed left or right. The steering motor made noise, and the driving motor also made noise at a low speed setting around 30. Adding a capacitor did not eliminate the noise.

I realized that our steering motor is different from the driving motor. The driving motor is supposed to keep turning while the car moves. The steering motor only needs to move the steering mechanism left or right. Our original software kept the steering motor powered after the command, even when the mechanism might already be at its limit.

My explanation is that the steering motor could be trying to turn against the mechanism when it cannot move farther. That could make it draw extra current and contribute to the noise and the driving slowdown. I want to compare the original continuous command with a short pulse instead of leaving steering powered.

My proposed fix is to power steering for 150 milliseconds and then turn its bridge off. Forward and back should stay running until I change direction or send stop. I also asked to reverse the forward/back electrical mapping because my motor connections were reversed.

## Evidence and corrections

| Item | What we can say |
|---|---|
| Noise and driving slowdown | User-reported observations; no measured sound level, RPM, or simultaneous voltage/current trace. |
| Continuous steering command | Confirmed in the earlier firmware. |
| Steering stalls against its limit | Plausible hypothesis; physical stall and its current have not been measured. |
| Extra steering demand causes driving slowdown | Plausible shared-supply explanation; supply sag, current limiting, driver losses, and mechanics remain possible contributors. |
| Steering uses “50% of the power” | Correction: approximately 50% PWM duty does not mean half the shared power. Actual power depends on voltage, current, load, and the driver. |
| Wheels stay turned after cutoff | Desired behavior, not established. Gears, friction, springs, and road forces determine what happens without power. |
| Capacitor experiment | Latest reported capacitor was 1,000 uF, rated 50 V; noise persisted. Placement and electrical effect were not validated. Motors reportedly already have ceramic capacitors. |

The L293D has a shared motor supply, not a fixed division of voltage between motors. A steering load can affect driving if the source/wiring voltage sags or the source reaches a current limit. Driver losses and thermal limits also matter. Software cutoff does not diagnose the previously reported motor motion with the motor-power switch off.

## Implemented software changes

Current test sketch: [DualMotorV2_BLE_Pulse.ino](../../firmware/testing/DualMotorV2_BLE_Pulse/DualMotorV2_BLE_Pulse.ino).

- Left/right energize steering for 150 ms, then disable the steering bridge.
- Repeated same-side messages do not restart or extend the pulse. An opposite command or stop re-arms steering.
- Forward/back remain latched, with their electrical polarity reversed.
- Startup speed remains 100; speed_N accepts integer settings from 0 to 100.
- Lowercase stop stops both motors; BLE disconnect also stops them.
- The architecture remains ESP32-WROOM-32D plus L293D.

Host logic tests passed for timing, repeated commands, reversed drive, continued driving, stop behavior, speed zero, and timer rollover. Actual ESP32 compilation and hardware verification remain pending. This is test firmware, not a validated complete vehicle controller.

## Visual comparison requested

The proposed MATLAB/Octave comparison has six panels:

| Quantity over time | Before | Revised |
|---|---|---|
| Average current | Continuous steering | 150 ms steering pulse |
| Voltage | Continuous steering | 150 ms steering pulse |
| Accumulated energy | Continuous steering | 150 ms steering pulse |

The example script was supplied in conversation. It assumes a 6 V source, effective source/wiring resistance, approximate bridge voltage drop, illustrative motor resistances and back EMF, and a steering limit reached 100 ms after the command. Those are assumptions, not identified properties of this car.

The model predicts less steering demand after cutoff and rail recovery when source resistance is nonzero. Setting source resistance to zero removes modeled rail sag while retaining the reduction in steering energy. This distinguishes two effects instead of assuming all slowdown has one cause.

Equivalent numerical calculations were checked in Python. The Octave script itself has not been executed here, and no user-run results have been reviewed. It does not calculate noise, temperature, vehicle speed, actual steering travel, PWM current ripple, or position retention. A simulation based on an assumed stall cannot independently prove a real stall.

## Hardware validation plan

Compare the same car, supply setting, drive command, starting steering position, and surface/load using the earlier and revised firmware. Keep the motor supply at the planned 6 V rather than raising voltage to compensate for an unmeasured problem.

Record:

1. Supply mode (CV or CC), actual voltage, and current during drive only and drive plus steering.
2. Shared motor rail voltage at L293D pin 8 relative to common ground.
3. Whether steering reaches a limit, how far a 150 ms pulse moves it, and whether it stays turned after cutoff.
4. Driving speed and noise before, during, and after steering, using a consistent observation method.
5. Any heating and whether noise persists after the steering bridge disables.

Avoid deliberately holding the mechanism stalled. Supply-display current is total supply current, not automatically one motor's current; a slow display may miss a 150 ms transient. Do not put an ammeter directly across a supply. Accurate energy savings require suitable time-resolved voltage/current data or a clearly identified estimate.

Successful hardware results would support the hypothesis. Persistent noise or slowdown would mean checking other causes rather than labeling the proposed fix proven.

## Engineering skills demonstrated

This episode shows a specific engineering process:

- Observation: notice noise and the interaction between steering and driving.
- System reasoning: connect mechanical resistance, software commands, and electrical demand.
- Firmware inspection: compare the intended behavior with what the program actually does.
- Design revision: make a small, explicit timing change while preserving drive behavior.
- Experimental planning: compare before/after conditions and identify measurements.
- Intellectual honesty: separate reported behavior, hypotheses, model assumptions, and verified results.

It also identifies what remains to learn: motor current and back EMF, PWM, supply regulation, driver losses, and feedback versus timed control.

## Hypothetical admissions / portfolio interpretation

This is relevant project evidence for an interest in electronics, embedded systems, and robotics. A reader could see initiative, debugging, iteration, and an effort to understand how software and physical hardware interact.

The observation is valuable because it leads to a testable explanation and a concrete change. It is not presented as a novel discovery or as evidence that the entire car is finished. Its credibility will depend on Jay understanding the code and model, documenting his own work accurately, and reporting actual results, including unsuccessful ones.

An admissions reader's response cannot be predicted, and one project episode cannot establish overall fit for a school or program. This project can contribute evidence of technical curiosity and developing engineering judgment. A clear before/after record is stronger evidence than unsupported claims of a solved problem or guaranteed energy savings.

Attribution: Jay identified and explained the steering hypothesis and requested the control change and comparison. An AI assistant helped edit documentation, prepare firmware changes and host tests, and write an illustrative simulation. Independent implementation details and results should be described according to what Jay actually performs and understands.

## Related project records

- [Test log](TEST_LOG.md)
- [Motor noise and load investigation](MOTOR_NOISE_AND_LOAD.md)
- [Firmware testing guide](../../firmware/testing/README.md)
- [Project limitations](../../limitations/LIMITATIONS.md)
