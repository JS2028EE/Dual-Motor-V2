/*
 * Dual Motor V2 — ES3C28P Display Firmware
 * Target: ESP32-S3 ES3C28P, ILI9341V 240x320
 *
 * Adafruit_GFX + Adafruit_ILI9341 version.
 * Touch is intentionally unused.
 *
 * UART:
 *   S3 GPIO43 RX <- C3 GPIO20 TX
 *   S3 GPIO44 TX -> C3 GPIO21 RX
 *
 * ES3C28P LCD:
 *   CS   = GPIO10
 *   DC   = GPIO46
 *   SCK  = GPIO12
 *   MOSI = GPIO11
 *   MISO = GPIO13
 *   RST  = shared board reset / EN (not driven separately)
 *   BL   = GPIO45
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

HardwareSerial ControllerSerial(1);

// -----------------------------
// UART
// -----------------------------
constexpr uint8_t UART_RX_PIN = 43;
constexpr uint8_t UART_TX_PIN = 44;

// -----------------------------
// ES3C28P LCD
// -----------------------------
constexpr uint8_t TFT_CS   = 10;
constexpr uint8_t TFT_DC   = 46;
constexpr uint8_t TFT_MOSI = 11;
constexpr uint8_t TFT_SCLK = 12;
constexpr uint8_t TFT_MISO = 13;
constexpr int8_t  TFT_RST  = -1;
constexpr uint8_t TFT_BL   = 45;

// Explicit-pin constructor keeps the ES3C28P SPI pins independent
// of the ESP32-S3 board's default SPI mapping.
Adafruit_ILI9341 tft(
  TFT_CS,
  TFT_DC,
  TFT_MOSI,
  TFT_SCLK,
  TFT_RST,
  TFT_MISO
);

// -----------------------------
// Display
// -----------------------------
constexpr int SCREEN_W = 320;
constexpr int SCREEN_H = 240;

constexpr uint16_t BG      = 0x0000;
constexpr uint16_t PANEL   = 0x10A2;
constexpr uint16_t GRID    = 0x2945;
constexpr uint16_t CYAN    = 0x07FF;
constexpr uint16_t MAGENTA = 0xF81F;
constexpr uint16_t GREEN   = 0x07E0;
constexpr uint16_t YELLOW  = 0xFFE0;
constexpr uint16_t RED     = 0xF800;
constexpr uint16_t WHITE   = 0xFFFF;

int navX = 0;
int navY = 0;
int driveX = 0;
int driveY = 0;

String lastEvent = "SYSTEM READY";

uint32_t lastPacketMs = 0;
uint32_t lastDrawMs = 0;
uint32_t lastPingMs = 0;

bool c3Ready = false;

// -----------------------------
// Drawing helpers
// -----------------------------
void panel(int x, int y, int w, int h, uint16_t accent) {
  tft.fillRoundRect(x, y, w, h, 6, PANEL);
  tft.drawRoundRect(x, y, w, h, 6, accent);
}

void crosshair(int cx, int cy, int x, int y, uint16_t color) {
  tft.drawCircle(cx, cy, 27, GRID);
  tft.drawFastHLine(cx - 31, cy, 62, GRID);
  tft.drawFastVLine(cx, cy - 31, 62, GRID);

  int px = cx + map(x, -1000, 1000, -22, 22);
  int py = cy + map(y, -1000, 1000, 22, -22);

  tft.fillCircle(px, py, 5, color);
  tft.drawCircle(px, py, 8, color);
}

void drawHeader() {
  tft.fillRect(0, 0, SCREEN_W, 26, BG);
  tft.drawFastHLine(0, 25, SCREEN_W, CYAN);

  tft.setTextColor(CYAN, BG);
  tft.setTextSize(2);
  tft.setCursor(8, 5);
  tft.print("DUAL MOTOR // V2");

  tft.setTextSize(1);
  tft.setTextColor(c3Ready ? GREEN : YELLOW, BG);
  tft.setCursor(245, 9);
  tft.print(c3Ready ? "CTRL ONLINE" : "CTRL WAITING");
}

void drawNavigationPanel() {
  panel(6, 32, 150, 132, CYAN);

  tft.setTextColor(CYAN, PANEL);
  tft.setTextSize(1);
  tft.setCursor(14, 40);
  tft.print("NAVIGATION");

  crosshair(81, 99, navX, navY, MAGENTA);

  tft.setTextColor(WHITE, PANEL);
  tft.setCursor(15, 140);
  tft.print("X: ");
  tft.print(navX);

  tft.setCursor(84, 140);
  tft.print("Y: ");
  tft.print(navY);

  tft.setTextColor(GRID, PANEL);
  tft.setCursor(15, 153);
  tft.print("PRESS=SELECT");

  tft.setCursor(88, 153);
  tft.print("HOLD=BACK");
}

void drawDrivePanel() {
  panel(164, 32, 150, 132, MAGENTA);

  tft.setTextColor(MAGENTA, PANEL);
  tft.setTextSize(1);
  tft.setCursor(172, 40);
  tft.print("DRIVE INPUT");

  crosshair(239, 99, driveX, driveY, CYAN);

  tft.setTextColor(WHITE, PANEL);
  tft.setCursor(173, 140);
  tft.print("X: ");
  tft.print(driveX);

  tft.setCursor(242, 140);
  tft.print("Y: ");
  tft.print(driveY);

  tft.setTextColor(GRID, PANEL);
  tft.setCursor(173, 153);
  tft.print("PRESS=BRAKE");

  tft.setCursor(242, 153);
  tft.print("HOLD=E-STOP");
}

void drawStatusPanel() {
  panel(6, 172, 308, 60, GRID);

  tft.setTextSize(1);

  tft.setTextColor(YELLOW, PANEL);
  tft.setCursor(14, 180);
  tft.print("SYSTEM STATUS");

  tft.setTextColor(WHITE, PANEL);
  tft.setCursor(14, 195);
  tft.print("UART: ");
  tft.print((millis() - lastPacketMs < 500) ? "CONNECTED" : "WAITING");

  tft.setCursor(115, 195);
  tft.print("VEHICLE: ");

  // Vehicle is intentionally not fabricated.
  tft.setTextColor(RED, PANEL);
  tft.print("NOT CONNECTED");

  tft.setTextColor(WHITE, PANEL);
  tft.setCursor(14, 213);
  tft.print("EVENT: ");

  String eventText = lastEvent;
  if (eventText.length() > 34) {
    eventText = eventText.substring(0, 34);
  }
  tft.print(eventText);

  tft.setTextColor(CYAN, PANEL);
  tft.setCursor(278, 213);
  tft.print("V2");
}

void drawUI() {
  tft.fillScreen(BG);

  drawHeader();
  drawNavigationPanel();
  drawDrivePanel();
  drawStatusPanel();
}

// -----------------------------
// UART packet handling
// -----------------------------
void handlePacket(String packet) {
  packet.trim();

  if (packet.length() == 0) {
    return;
  }

  if (packet.startsWith("INPUT,")) {
    int p1 = packet.indexOf(',', 6);
    int p2 = packet.indexOf(',', p1 + 1);
    int p3 = packet.indexOf(',', p2 + 1);

    if (p1 > 0 && p2 > p1 && p3 > p2) {
      navX   = constrain(packet.substring(6, p1).toInt(), -1000, 1000);
      navY   = constrain(packet.substring(p1 + 1, p2).toInt(), -1000, 1000);
      driveX = constrain(packet.substring(p2 + 1, p3).toInt(), -1000, 1000);
      driveY = constrain(packet.substring(p3 + 1).toInt(), -1000, 1000);

      lastPacketMs = millis();
      c3Ready = true;
    }

    return;
  }

  if (packet.startsWith("EVENT,")) {
    lastEvent = packet.substring(6);
    lastPacketMs = millis();
    c3Ready = true;
    return;
  }

  if (packet == "READY,C3") {
    lastEvent = "C3 READY";
    lastPacketMs = millis();
    c3Ready = true;
    return;
  }

  if (packet == "PONG,C3") {
    lastEvent = "C3 PONG";
    lastPacketMs = millis();
    c3Ready = true;
    return;
  }

  if (packet.startsWith("DEVICE,")) {
    lastEvent = packet.substring(7);
    lastPacketMs = millis();
    c3Ready = true;
    return;
  }
}

// -----------------------------
// Setup
// -----------------------------
void setup() {
  Serial.begin(115200);

  ControllerSerial.begin(
    115200,
    SERIAL_8N1,
    UART_RX_PIN,
    UART_TX_PIN
  );

  // ES3C28P backlight.
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  delay(100);

  // Initialize ILI9341.
  tft.begin();

  // Landscape: 320 x 240.
  tft.setRotation(1);

  // IPS panel normally looks correct with inversion enabled.
  tft.invertDisplay(true);

  tft.fillScreen(BG);

  // Boot screen.
  tft.setTextColor(CYAN, BG);
  tft.setTextSize(3);
  tft.setCursor(44, 65);
  tft.print("DUAL MOTOR");

  tft.setCursor(118, 103);
  tft.print("V2");

  tft.setTextColor(MAGENTA, BG);
  tft.setTextSize(1);
  tft.setCursor(93, 145);
  tft.print("ENGINEERING CONSOLE");

  tft.setTextColor(GREEN, BG);
  tft.setCursor(115, 162);
  tft.print("S3 INITIALIZING");

  delay(900);

  ControllerSerial.println("PING");
  lastPingMs = millis();

  drawUI();
}

// -----------------------------
// Main loop
// -----------------------------
void loop() {
  while (ControllerSerial.available()) {
    String packet = ControllerSerial.readStringUntil('\n');
    handlePacket(packet);
  }

  // Keep the C3 handshake alive.
  if (millis() - lastPingMs >= 1000) {
    lastPingMs = millis();
    ControllerSerial.println("PING");
  }

  // Refresh UI at 10 FPS.
  if (millis() - lastDrawMs >= 100) {
    lastDrawMs = millis();
    drawUI();
  }
}
