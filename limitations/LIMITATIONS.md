# Current vehicle limitations

- The damaged C3 is excluded; the S3 handheld controller is outside the active scope.
- There is no implemented ESP32 vehicle firmware yet.
- Wireless transport/app, pin mapping, final wiring, and automatic behavior are pending.
- Motor RPM, torque, gearing, starting/stall current, and complete mass are unmeasured.
- Steering angle cannot be known from a two-wire motor alone; feedback/centering needs inspection.
- A direct 4.5 V motor supply with 5 V L293D logic is outside TI recommended supply conditions.
- L293D voltage drop and heating can substantially reduce motor performance.
- Candidate boost-module identity and motor-start capability are unconfirmed.
- Neither a boost converter nor a capacitor removes the L293D current/thermal limits.
- Capacitors reduce brief disturbances; they do not supply sustained extra voltage/current.
- The reported 22 pF and 104 pF ceramics are not selected as motor-noise suppression capacitors.
- Electronics battery voltage/chemistry, regulation, charge protection, and power switching need verification.
- Ultrasonic level protection and exact GPS/buzzer supply requirements need confirmation.
- GPS is not precise obstacle sensing.
- Simulation output uses assumed inputs; movement is independent of electrical estimates.
- The example battery runtimes exclude motors and do not guarantee supply peak-current capability.
- No real driving, obstacle-avoidance, stopping-distance, or fail-safe performance is validated.

See [vehicle plan](../documentation/architecture/VEHICLE_V2.md) and [simulation notes](../simulations/README.md).

## Historical controller limitations

# Current Limitations

- Joystick orientation/calibration still requires physical testing.
- Final power wiring must be verified on the actual S3 board.
- Wi-Fi controller communication is implemented and replaces UART in the active firmware.
- The old UART wires remain physically documented but are not used by the current controller firmware.
- The controller network uses prototype credentials compiled into both sketches.
- Vehicle communication is not implemented.
- Touch is intentionally unused.
- Real vehicle GPS data is unavailable until vehicle integration.
- Wi-Fi link behavior still needs physical range, reconnect, and interference testing.

