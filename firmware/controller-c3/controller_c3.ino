/*
 * Dual Motor V2 — Controller Firmware
 * Target: ESP32-C3 Super Mini
 *
 * Navigation joystick: short press = Select, hold = Back
 * Driving joystick: short press = Brake, hold = Emergency Stop
 *
 * UART: C3 GPIO20 TX -> S3 GPIO43 RX
 *       C3 GPIO21 RX <- S3 GPIO44 TX
 */

#include <Arduino.h>

HardwareSerial DisplaySerial(1);

// Final controller pinout
constexpr uint8_t NAV_X_PIN   = 0;  // A0 / GPIO0
constexpr uint8_t NAV_Y_PIN   = 1;  // A1 / GPIO1
constexpr uint8_t DRIVE_X_PIN = 3;  // A3 / GPIO3
constexpr uint8_t DRIVE_Y_PIN = 4;  // A4 / GPIO4
constexpr uint8_t NAV_SW_PIN = 6;
constexpr uint8_t DRIVE_SW_PIN = 7;

constexpr uint8_t UART_RX_PIN = 21;
constexpr uint8_t UART_TX_PIN = 20;

constexpr int CENTER = 2048;
constexpr int DEADZONE = 180;
constexpr uint32_t DEBOUNCE_MS = 35;
constexpr uint32_t HOLD_MS = 700;

// Sideways joystick mounting: change these after physical direction testing.
constexpr bool NAV_SWAP_XY = false;
constexpr bool NAV_INVERT_X = false;
constexpr bool NAV_INVERT_Y = false;
constexpr bool DRIVE_SWAP_XY = false;
constexpr bool DRIVE_INVERT_X = false;
constexpr bool DRIVE_INVERT_Y = false;

struct Stick { int x; int y; };

struct ButtonState {
  bool stablePressed = false;
  bool lastRawPressed = false;
  uint32_t lastChangeMs = 0;
  uint32_t pressStartMs = 0;
  bool holdSent = false;
};

ButtonState navButton, driveButton;

int normalizeAxis(int raw) {
  int delta = raw - CENTER;
  if (abs(delta) <= DEADZONE) return 0;
  if (delta > 0) return constrain(map(delta, DEADZONE, 2047, 0, 1000), 0, 1000);
  return constrain(map(delta, -DEADZONE, -2048, 0, -1000), -1000, 0);
}

Stick readStick(uint8_t xPin, uint8_t yPin, bool swapXY, bool invertX, bool invertY) {
  int x = normalizeAxis(analogRead(xPin));
  int y = normalizeAxis(analogRead(yPin));
  if (swapXY) { int t = x; x = y; y = t; }
  if (invertX) x = -x;
  if (invertY) y = -y;
  return {x, y};
}

void updateButton(ButtonState &b, uint8_t pin, bool navigation) {
  bool rawPressed = digitalRead(pin) == LOW;
  uint32_t now = millis();

  if (rawPressed != b.lastRawPressed) {
    b.lastRawPressed = rawPressed;
    b.lastChangeMs = now;
  }

  if (now - b.lastChangeMs >= DEBOUNCE_MS && rawPressed != b.stablePressed) {
    b.stablePressed = rawPressed;

    if (b.stablePressed) {
      b.pressStartMs = now;
      b.holdSent = false;
    } else if (!b.holdSent) {
      DisplaySerial.println(navigation ? "EVENT,NAV_SELECT" : "EVENT,DRIVE_BRAKE");
    }
  }

  if (b.stablePressed && !b.holdSent && now - b.pressStartMs >= HOLD_MS) {
    b.holdSent = true;
    DisplaySerial.println(navigation ? "EVENT,NAV_BACK" : "EVENT,DRIVE_ESTOP");
  }
}

void sendState(const Stick &nav, const Stick &drive) {
  DisplaySerial.print("INPUT,");
  DisplaySerial.print(nav.x);
  DisplaySerial.print(',');
  DisplaySerial.print(nav.y);
  DisplaySerial.print(',');
  DisplaySerial.print(drive.x);
  DisplaySerial.print(',');
  DisplaySerial.println(drive.y);
}

void handleDisplayCommand() {
  if (!DisplaySerial.available()) return;
  String command = DisplaySerial.readStringUntil('\n');
  command.trim();

  if (command == "PING") DisplaySerial.println("PONG,C3");
  else if (command == "IDENTIFY") DisplaySerial.println("DEVICE,DUAL_MOTOR_V2_C3");
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  pinMode(NAV_SW_PIN, INPUT_PULLUP);
  pinMode(DRIVE_SW_PIN, INPUT_PULLUP);

  DisplaySerial.begin(115200, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  delay(300);
  DisplaySerial.println("READY,C3");
}

void loop() {
  Stick nav = readStick(NAV_X_PIN, NAV_Y_PIN, NAV_SWAP_XY, NAV_INVERT_X, NAV_INVERT_Y);
  Stick drive = readStick(DRIVE_X_PIN, DRIVE_Y_PIN, DRIVE_SWAP_XY, DRIVE_INVERT_X, DRIVE_INVERT_Y);

  updateButton(navButton, NAV_SW_PIN, true);
  updateButton(driveButton, DRIVE_SW_PIN, false);

  static uint32_t lastTelemetry = 0;
  if (millis() - lastTelemetry >= 50) {
    lastTelemetry = millis();
    sendState(nav, drive);
  }

  handleDisplayCommand();
  delay(2);
}
