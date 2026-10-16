# Dual Motor V2

An ESP32 robotic car with one motor for forward/reverse and one motor for steering.

**Current revision: V2.3.0 planning update — October 16, 2026.** The damaged C3 is excluded. The car will use the ESP32-WROOM-32D and receive manual commands from a phone or computer over Wi-Fi or Bluetooth. The earlier C3/S3 handheld-controller design is historical.

## Current plan

- ESP32-WROOM-32D development board as the vehicle controller.
- L293D to control two existing 3–6 V brushed DC motors: drive and steering.
- Manual wireless control plus planned automatic obstacle avoidance.
- Front ultrasonic sensor, GPS, buzzer, and two white headlights.
- Three 1.5 V cells in series for a nominal 4.5 V motor pack.
- Separate rechargeable electronics battery; 1,000 mAh and 500 mAh batteries are available.
- Evaluate an adjustable boost converter for a 6 V motor supply.
- Include an on/off switch and plan charging after confirming battery chemistry and charger requirements.
- Use GNU Octave for initial movement and electrical estimates, then replace assumptions with measurements.

**Status:** architecture and initial simulation only. Final wiring, pin assignments, firmware, charging circuit, motor-current limits, and actual driving performance are not validated.

## Documentation

| Topic | File |
|---|---|
| Signal and power block diagrams | [Block diagrams](diagrams/README.md) |
| Parts, power, control, and measurement plan | [Vehicle plan](documentation/architecture/VEHICLE_V2.md) |
| Octave script and assumptions | [Simulation notes](simulations/README.md) |
| Current checklist and earlier progress | [Progress](progress/PROGRESS.md) |
| Electrical and mechanical limits | [Limitations](limitations/LIMITATIONS.md) |
| Revision record | [V2.3.0](versions/V2.3.0.md) |
| Earlier controller overview | [Historical README](documentation/history/CONTROLLER_README.md) |

## First Octave result

With an assumed 65 mm wheel, 120 wheel RPM, and a 1.5 s response time, the example approaches **0.41 m/s** and covers **3.47 m in 10 s**. The user ran the initial script and shared the movement plots.

These are hypothetical results. The movement model does not yet couple battery voltage, current, driver loss, mass, or motor torque to speed. The boost comparison in the script changes electrical estimates only.

## Historical controller work

C3/S3 firmware and earlier controller notes are retained as development history. They do not implement the current ESP32-only vehicle plan. The S3 display and joystick controller are outside the active build scope.
