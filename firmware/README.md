# Firmware status

**Active target: ESP32-WROOM-32D vehicle only.**

Use [vehicle-esp32](vehicle-esp32/README.md) for the current target status and [block diagrams](../diagrams/README.md) for architecture. No current vehicle sketch or final GPIO map has been committed.

The controller-c3 and controller-s3 directories contain historical sketches from the abandoned handheld design. Do not upload or treat them as dependencies for the current car. They are preserved to avoid losing earlier work.

The GPIO/PWM sequence in [the Octave example](../simulations/power_control_v2.m) is a simulation, not uploadable firmware or a tested pin assignment.
