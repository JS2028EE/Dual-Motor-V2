# Vehicle control and telemetry app plan

Updated October 19, 2026. Requirements/roadmap only; no app is implemented by this document.

## Connection and control

The phone/computer app connects directly to the ESP32-WROOM-32D car using Wi-Fi or Bluetooth. The initial Bluetooth implementation is BLE. Wi-Fi firmware, transport selection, permissions, pairing/security, simultaneous-radio policy and app platform remain to be chosen and tested. USB Serial remains a bench diagnostic interface.

Provide independent drive and steering controls so forward/back and left/right can be used together. Show connection and command acknowledgement state, a prominent stop, and the active manual/automatic mode. Speed policy (fixed/selectable/adjustable) is undecided; preserve the current test commands until a new protocol is implemented and verified.

Current control semantics matter: drive latches; steering is a bounded 200 ms pulse, with same-side repeat suppression until opposite or stop. A held UI control must not accidentally retrigger steering continuously or leave drive latched after a lost session. Plan session ownership, drive watchdog, app backgrounding/focus-loss and reconnect behavior before live app tests.

## Feature requirements

| Area | Intended app function | Vehicle/data dependency | Status |
|---|---|---|---|
| Manual driving | Simultaneous drive and steering; stop | Motor command/acknowledgement protocol | UI pending; BLE test commands exist |
| Sensing | Display distance, obstacle and sensor health | Identified sensors, correct levels, calibrated data | Pending |
| Navigation/map | Show actual position, heading where valid, route context | GPS/location validity, map source and coordinate conventions | Pending |
| Waypoints | Add, inspect, reorder and cancel targets | Reliable localization, path/control logic and local stopping | Pending |
| Automation | Explicit start/pause/cancel, visible mode/goal | Local vehicle state machine and obstacle behavior | Pending |
| Speed | Show actual velocity separately from commanded PWM | Valid GPS-derived speed or another calibrated sensor | Pending |
| Distance traveled | Trip distance, reset and validity/quality indicators | Validated accumulation from position or wheel measurements | Pending |
| Diagnostics | Link state, command age, logs and fault state | Telemetry protocol and timestamps | Serial command logs exist; full telemetry pending |
| Accessories | Buzzer/headlight state and controls where useful | Accessory drivers/firmware | Pending |

Requested velocity and distance are measurement features, not values inferred directly from speed_100 or PWM duty. Decide the measurement method and evaluate its limitations on the actual car. Define handling of invalid location, stale readings, resets and drift before reporting trip totals. No wheel encoders or steering-position sensor are currently confirmed.

## Data and protocol plan

Use a common command/state model for both wireless transports. Version it; define command identifiers/acknowledgements, session ownership, timestamps and units (m, m/s, degrees, ms). Mark telemetry valid/stale/unavailable. Do not invent location, current, voltage or traveled distance when no corresponding measurement exists.

Separate user navigation goals from low-level motor control. Manual override/stop must cancel automatic movement, and the vehicle must handle link loss locally. Remote app requests are not a substitute for local obstacle checks, feedback, accessible power-off, or motor end-stop limits.

Current BLE Nordic UART UUIDs and lowercase commands are documented in the [testing guide](../../firmware/testing/README.md). New waypoint/automation messages and live parameter edits are not implemented. Current serial status is commanded GPIO/PWM/timing only.

## Incremental implementation

1. Validate portable power, stop behavior and session/watchdog requirements.
2. Build a simple app control screen using the existing BLE protocol; add independent controls and acknowledgements.
3. Implement/test the Wi-Fi vehicle link using the same command/state definitions.
4. Integrate sensors, GPS and actual telemetry with freshness/validity.
5. Validate speed/distance measurements and map display.
6. Add supervised waypoint and automatic behavior only after localization, sensing and stopping are demonstrated.

This sequence is a plan, not completed work or a release schedule.
