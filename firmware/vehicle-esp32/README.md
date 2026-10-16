# ESP32 vehicle firmware

This is the active target for the redesigned Dual Motor V2.

Planned target: ESP32-WROOM-32D development board. Manual commands will arrive from a phone/computer over Wi-Fi or Bluetooth; transport is pending.

Required features include L293D drive/steering control, ultrasonic obstacle sensing, GPS telemetry, white headlights, buzzer, and defined stop behavior.

**No vehicle sketch is implemented here yet.** Final GPIO assignments and power design must be verified first. Existing C3/S3 sketches belong to the earlier handheld-controller design.

See [vehicle plan](../../documentation/architecture/VEHICLE_V2.md).
