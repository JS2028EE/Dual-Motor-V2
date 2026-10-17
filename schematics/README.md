# Schematics

Current references:
- [Power terminal map](../documentation/pin-configuration/POWER_PINOUT.md)
- [Active block diagrams](../diagrams/README.md)
- [Current decisions](../documentation/DECISIONS.md)

No complete verified vehicle wiring schematic or final GPIO map exists yet. The architecture uses one ESP32-WROOM-32D, L293D, separate 5 V/6 V boost stages, TP4056-style electronics charging, and two switched supply branches. Earlier C3/S3 controller wiring is historical.

The bench motor-running-with-switch-off observation remains unresolved; the diagrams describe intended final connections rather than a verified copy of the present breadboard.
