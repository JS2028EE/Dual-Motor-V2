# ESP32-C3 Arduino IDE Troubleshooting

## Error

If Arduino IDE reports undefined reference to `setup()` and undefined reference to `loop()`, the ESP32-C3 core is linking successfully, but the selected Arduino sketch does not contain the required Arduino entry points.

## Important

The repository firmware file `firmware/controller-c3/controller_c3.ino` already contains both `void setup()` and `void loop()`.

Therefore this linker error is normally caused by opening or compiling the wrong sketch, or by the sketch file not being loaded into the Arduino IDE editor.

## Correct Arduino IDE setup

1. Open File -> Open in Arduino IDE.
2. Select `firmware/controller-c3/controller_c3.ino`.
3. Arduino IDE should open the sketch named `controller_c3`.
4. Confirm the editor visibly contains `void setup()` and `void loop()`.
5. Select the ESP32-C3 Super Mini under Tools -> Board.
6. Select the correct COM port.
7. Compile again.

## Sketch-folder rule

Arduino expects the main `.ino` sketch to have the same name as its sketch folder.

Correct structure:

```text
controller_c3/
└── controller_c3.ino
```

If the file is renamed, keep the folder and `.ino` filename consistent.

## If the error still appears

Check the beginning of the Arduino IDE output. If there is no compiler error pointing to `controller_c3.ino`, Arduino may be compiling a different sketch.

Temporary test:

```cpp
void setup() {
  Serial.begin(115200);
}

void loop() {
}
```

If that compiles, the board/core installation is working and the problem is the selected sketch or file structure.

## Current C3 pinout

- NAV X -> GPIO0 / A0
- NAV Y -> GPIO1 / A1
- DRIVE X -> GPIO3 / A3
- DRIVE Y -> GPIO4 / A4
- NAV SW -> GPIO6
- DRIVE SW -> GPIO7
- UART RX -> GPIO21
- UART TX -> GPIO20
- Joystick VCC -> 3V3
- Joystick GND -> GND