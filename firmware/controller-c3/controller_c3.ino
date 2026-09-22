/*
 * Dual Motor V2 — Controller Firmware
 * Target: ESP32-C3 Super Mini
 *
 * Navigation joystick: short press = Select, hold = Back
 * Driving joystick: short press = Brake, hold = Emergency Stop
 *
 * NETWORK:
 *   C3 connects to the ES3C28P controller Wi-Fi AP.
 *   Transport: TCP
 *   S3 AP IP: 192.168.4.1
 *   TCP port: 4210
 *
 * The old GPIO20/GPIO21 UART wiring is retained physically but is
 * no longer used by this firmware. Controller communication is Wi-Fi.
 */

#include <Arduino.h>
#include <WiFi.h>

// ------------------------------------------------------------
// Wi-Fi link
// ------------------------------------------------------------
constexpr char WIFI_SSID[] = "DUAL-MOTOR-V2";
constexpr char WIFI_PASSWORD[] = "DMV2-CTRL";

const IPAddress S3_IP(192, 168, 4, 1);
constexpr uint16_t TCP_PORT = 4210;

WiFiClient ControllerClient;

uint32_t lastWiFiAttemptMs = 0;
uint32_t lastTelemetryMs = 0;
uint32_t lastLinkMessageMs = 0;

// ------------------------------------------------------------
// Joystick pins
// ------------------------------------------------------------
constexpr uint8_t NAV_X_PIN   = 0;  // A0 / GPIO0
constexpr uint8_t NAV_Y_PIN   = 1;  // A1 / GPIO1
constexpr uint8_t DRIVE_X_PIN = 3;  // A3 / GPIO3
constexpr uint8_t DRIVE_Y_PIN = 4;  // A4 / GPIO4
constexpr uint8_t NAV_SW_PIN = 6;
constexpr uint8_t DRIVE_SW_PIN = 7;

// ------------------------------------------------------------
// Joystick calibration
// ------------------------------------------------------------
constexpr int CENTER = 2048;
constexpr int DEADZONE = 180;
constexpr uint32_t DEBOUNCE_MS = 35;
constexpr uint32_t HOLD_MS = 700;

// Sideways joystick mounting.
// Change these only after physical direction testing.
constexpr bool NAV_SWAP_XY = false;
constexpr bool NAV_INVERT_X = false;
constexpr bool NAV_INVERT_Y = false;
constexpr bool DRIVE_SWAP_XY = false;
constexpr bool DRIVE_INVERT_X = false;
constexpr bool DRIVE_INVERT_Y = false;

struct Stick {
  int x;
  int y;
};

struct ButtonState {
  bool stablePressed = false;
  bool lastRawPressed = false;
  uint32_t lastChangeMs = 0;
  uint32_t pressStartMs = 0;
  bool holdSent = false;
};

ButtonState navButton;
ButtonState driveButton;

// ------------------------------------------------------------
// Input helpers
// ------------------------------------------------------------
int normalizeAxis(int raw) {
  int delta = raw - CENTER;

  if (abs(delta) <= DEADZONE) {
    return 0;
  }

  if (delta > 0) {
    return constrain(
      map(delta, DEADZONE, 2047, 0, 1000),
      0,
      1000
    );
  }

  return constrain(
    map(delta, -DEADZONE, -2048, 0, -1000),
    -1000,
    0
  );
}

Stick readStick(
  uint8_t xPin,
  uint8_t yPin,
  bool swapXY,
  bool invertX,
  bool invertY
) {
  int x = normalizeAxis(analogRead(xPin));
  int y = normalizeAxis(analogRead(yPin));

  if (swapXY) {
    int temp = x;
    x = y;
    y = temp;
  }

  if (invertX) x = -x;
  if (invertY) y = -y;

  return {x, y};
}

// ------------------------------------------------------------
// Wi-Fi connection
// ------------------------------------------------------------
void connectToS3() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  if (millis() - lastWiFiAttemptMs < 5000) {
    return;
  }

  lastWiFiAttemptMs = millis();

  Serial.print("[WiFi] Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void sendPacket(const String &packet) {
  if (!ControllerClient || !ControllerClient.connected()) {
    return;
  }

  ControllerClient.println(packet);
}

void handleNetworkCommand() {
  if (!ControllerClient || !ControllerClient.connected()) {
    return;
  }

  while (ControllerClient.available()) {
    String command = ControllerClient.readStringUntil('\n');
    command.trim();

    if (command == "PING") {
      sendPacket("PONG,C3");
    }
    else if (command == "IDENTIFY") {
      sendPacket("DEVICE,DUAL_MOTOR_V2_C3");
    }
  }
}

void connectTCP() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (ControllerClient.connected()) {
    return;
  }

  if (millis() - lastLinkMessageMs < 1000) {
    return;
  }

  lastLinkMessageMs = millis();

  Serial.println("[WiFi] Connecting TCP to S3...");

  ControllerClient.stop();

  if (ControllerClient.connect(S3_IP, TCP_PORT)) {
    Serial.println("[WiFi] Controller link connected");
    sendPacket("READY,C3");
    sendPacket("DEVICE,DUAL_MOTOR_V2_C3");
  }
  else {
    Serial.println("[WiFi] TCP connection failed");
  }
}

// ------------------------------------------------------------
// Button events
// ------------------------------------------------------------
void updateButton(
  ButtonState &b,
  uint8_t pin,
  bool navigation
) {
  bool rawPressed = digitalRead(pin) == LOW;
  uint32_t now = millis();

  if (rawPressed != b.lastRawPressed) {
    b.lastRawPressed = rawPressed;
    b.lastChangeMs = now;
  }

  if (
    now - b.lastChangeMs >= DEBOUNCE_MS &&
    rawPressed != b.stablePressed
  ) {
    b.stablePressed = rawPressed;

    if (b.stablePressed) {
      b.pressStartMs = now;
      b.holdSent = false;
    }
    else if (!b.holdSent) {
      sendPacket(
        navigation
          ? "EVENT,NAV_SELECT"
          : "EVENT,DRIVE_BRAKE"
      );
    }
  }

  if (
    b.stablePressed &&
    !b.holdSent &&
    now - b.pressStartMs >= HOLD_MS
  ) {
    b.holdSent = true;

    sendPacket(
      navigation
        ? "EVENT,NAV_BACK"
        : "EVENT,DRIVE_ESTOP"
    );
  }
}

// ------------------------------------------------------------
// Telemetry
// ------------------------------------------------------------
void sendState(const Stick &nav, const Stick &drive) {
  if (!ControllerClient || !ControllerClient.connected()) {
    return;
  }

  ControllerClient.print("INPUT,");
  ControllerClient.print(nav.x);
  ControllerClient.print(',');
  ControllerClient.print(nav.y);
  ControllerClient.print(',');
  ControllerClient.print(drive.x);
  ControllerClient.print(',');
  ControllerClient.println(drive.y);
}

// ------------------------------------------------------------
// Setup
// ------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  analogReadResolution(12);

  pinMode(NAV_SW_PIN, INPUT_PULLUP);
  pinMode(DRIVE_SW_PIN, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);

  // Keep the local controller link responsive.
  WiFi.setSleep(false);

  Serial.println();
  Serial.println("=== DUAL MOTOR V2 / C3 ===");
  Serial.println("Communication: Wi-Fi TCP");
  Serial.print("Target AP: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

// ------------------------------------------------------------
// Main loop
// ------------------------------------------------------------
void loop() {
  connectToS3();
  connectTCP();
  handleNetworkCommand();

  Stick nav = readStick(
    NAV_X_PIN,
    NAV_Y_PIN,
    NAV_SWAP_XY,
    NAV_INVERT_X,
    NAV_INVERT_Y
  );

  Stick drive = readStick(
    DRIVE_X_PIN,
    DRIVE_Y_PIN,
    DRIVE_SWAP_XY,
    DRIVE_INVERT_X,
    DRIVE_INVERT_Y
  );

  updateButton(navButton, NAV_SW_PIN, true);
  updateButton(driveButton, DRIVE_SW_PIN, false);

  if (millis() - lastTelemetryMs >= 50) {
    lastTelemetryMs = millis();
    sendState(nav, drive);
  }

  delay(2);
}
