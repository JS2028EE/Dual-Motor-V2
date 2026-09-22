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
